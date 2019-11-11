/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010-2015  npzacs
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmc_device.h"

#include "util/logging.h"
#include "util/macro.h"
#include "util/strutl.h"

#include <stdlib.h>
#include <string.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>

#include <DiskArbitration/DiskArbitration.h>

/* need to undefine VERSION as one of the members of struct
   SCSICmd_INQUIRY_StandardData is named VERSION (see
   IOKit/scsi/SCSICmds_INQUIRY_Definitions.h) */
#undef VERSION
#include <IOKit/scsi/SCSITaskLib.h>

#include <IOKit/storage/IOBDMediaBSDClient.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif


/*
 *
 */

struct mmcdev {
    MMCDeviceInterface **mmcInterface;
    SCSITaskDeviceInterface **taskInterface;

    /* device short name (ie disk1) */
    char bsd_name[128];

    /* for mounting/unmounting the disc */
    DADiskRef disk;
    DASessionRef session;
    bool is_mounted;
};

int device_send_cmd(MMCDEV *mmc, const uint8_t *cmd, uint8_t *buf, size_t tx, size_t rx)
{
    SCSITaskInterface **task = NULL;
    SCSI_Sense_Data sense;
    SCSITaskStatus status;
    SCSITaskSGElement iov;
    UInt8 direction;
    UInt64 sent;
    int rc;

    if (NULL == mmc->taskInterface) {
        return 0;
    }

    do {
        task = (*mmc->taskInterface)->CreateSCSITask (mmc->taskInterface);
        if (NULL == task) {
            BD_DEBUG(DBG_MMC, "Could not create SCSI Task\n");
            break;
        }

        iov.address = (uintptr_t) buf;
        iov.length  = tx ? tx : rx;

        if (buf) {
            direction = tx ? kSCSIDataTransfer_FromInitiatorToTarget :
                kSCSIDataTransfer_FromTargetToInitiator;
        } else {
            direction = kSCSIDataTransfer_NoDataTransfer;
        }

        SCSICommandDescriptorBlock cdb = {0};
        memcpy(cdb, cmd, sizeof(cdb));

        rc = (*task)->SetCommandDescriptorBlock (task, cdb, kSCSICDBSize_16Byte);
        if (kIOReturnSuccess != rc) {
            BD_DEBUG(DBG_MMC, "Error setting SCSI command\n");
            break;
        }

        rc = (*task)->SetScatterGatherEntries (task, &iov, 1, iov.length, direction);
        if (kIOReturnSuccess != rc) {
            BD_DEBUG(DBG_MMC, "Error setting SCSI scatter gather entries\n");
            break;
        }

        rc = (*task)->SetTimeoutDuration (task, 5000000);
        if (kIOReturnSuccess != rc) {
            BD_DEBUG(DBG_MMC, "Error setting SCSI command timeout\n");
            break;
        }

        memset (&sense, 0, sizeof (sense));

        rc = (*task)->ExecuteTaskSync (task, &sense, &status, &sent);

        char str[512];
        BD_DEBUG(DBG_MMC, "Send SCSI MMC cmd %s:\n", str_print_hex(str, cmd, 16));
        if (tx) {
            BD_DEBUG(DBG_MMC, "  Buffer: %s ->\n", str_print_hex(str, buf, tx>255?255:tx));
        } else {
            BD_DEBUG(DBG_MMC, "  Buffer: %s <-\n", str_print_hex(str, buf, rx>255?255:rx));
        }

        if (kIOReturnSuccess != rc || status != 0) {
            BD_DEBUG(DBG_MMC, "  Send failed!\n");
            break;
        } else {
            BD_DEBUG(DBG_MMC, "  Send succeeded! sent = %lld status = %u. response = %x\n",
                  (unsigned long long) sent, status, sense.VALID_RESPONSE_CODE);
        }

        (*task)->Release (task);

        return 1;
    } while (0);

    if (task) {
        (*task)->Release (task);
    }

    return 0;
}

static int get_mounted_device_from_path (MMCDEV *mmc, const char *path) {
  struct statfs stat_info;
  int rc;

  rc = statfs (path, &stat_info);
  if (0 != rc) {
    return rc;
  }

  strncpy (mmc->bsd_name, basename (stat_info.f_mntfromname), sizeof (mmc->bsd_name));

  return 0;
}

static void iokit_unmount_complete (DADiskRef disk, DADissenterRef dissenter,
                                    void *context) {
    (void)disk; /* suppress warning */

    if (dissenter) {
        BD_DEBUG(DBG_MMC, "Could not unmount the disc\n");
    } else {
        BD_DEBUG(DBG_MMC, "Disc unmounted\n");
        ((MMCDEV *)context)->is_mounted = 0;
    }
}

static void iokit_mount_complete (DADiskRef disk, DADissenterRef dissenter,
                                  void *context) {
    (void) disk; /* suppress warning */
    (void) dissenter; /* suppress warning */

    if (dissenter) {
        BD_DEBUG(DBG_MMC, "Could not mount the disc\n");
    } else {
        BD_DEBUG(DBG_MMC, "Disc mounted\n");
    }

    /* FIXME: The disc does not actually mount whether there is
     * a dissenter or not, the OS mounts the disc automatically
     * kind of racing against us mounting the disc.
     * It is pure luck if the disc is mounted or not, sometimes
     * we are lucky enough, especially because the runloop is
     * running for 10 seconds, which most of the time is long
     * enough for the OS to mount the disc again.
     */
    ((MMCDEV *)context)->is_mounted = 1;
}

static int iokit_unmount (MMCDEV *mmc) {
    if (0 == mmc->is_mounted) {
        return 0; /* nothing to do */
    }

    BD_DEBUG(DBG_MMC, "Unmounting disk\n");

    mmc->session = DASessionCreate (kCFAllocatorDefault);
    if (NULL == mmc->session) {
        BD_DEBUG(DBG_MMC, "Could not create a disc arbitration session\n");
        return -1;
    }

    mmc->disk = DADiskCreateFromBSDName (kCFAllocatorDefault, mmc->session, mmc->bsd_name);
    if (NULL == mmc->disk) {
        BD_DEBUG(DBG_MMC, "Could not create a disc arbitration disc for the device\n");
        CFRelease (mmc->session);
        mmc->session = NULL;
        return -1;
    }

    DAApprovalSessionScheduleWithRunLoop (mmc->session, CFRunLoopGetCurrent (),
                                          kCFRunLoopDefaultMode);

    DADiskUnmount (mmc->disk, kDADiskUnmountOptionForce, iokit_unmount_complete, mmc);

    CFRunLoopRunInMode (kCFRunLoopDefaultMode, 10, true);

    return mmc->is_mounted ? -1 : 0;
}

static int iokit_mount (MMCDEV *mmc) {
    if (0 == mmc->is_mounted) {
        if (mmc->disk && mmc->session) {
            DADiskMount (mmc->disk, NULL, kDADiskMountOptionDefault, iokit_mount_complete, mmc);

            CFRunLoopRunInMode (kCFRunLoopDefaultMode, 10, true);

            DAApprovalSessionUnscheduleFromRunLoop (mmc->session, CFRunLoopGetCurrent (),
                                                    kCFRunLoopDefaultMode);
        }

        if (mmc->disk) {
            CFRelease (mmc->disk);
            mmc->disk = NULL;
        }

        if (mmc->session) {
            CFRelease (mmc->session);
            mmc->session = NULL;
        }
    }

    return mmc->is_mounted ? 0 : -1;
}

static int iokit_find_service_matching (MMCDEV *mmc, io_service_t *servp) {
    CFMutableDictionaryRef matchingDict = IOServiceMatching("IOBDServices");
    io_iterator_t deviceIterator;
    io_service_t service;
    int rc;

    assert (NULL != servp);

    *servp = 0;

    if (!matchingDict) {
        BD_DEBUG(DBG_MMC, "Could not create a matching dictionary for IOBDServices\n");
        return -1;
    }

    /* this call consumes the reference to the matchingDict. we do not need to release it */
    rc = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &deviceIterator);
    if (kIOReturnSuccess != rc) {
        BD_DEBUG(DBG_MMC, "Could not create device iterator\n");
        return -1;
    }

    while (0 != (service = IOIteratorNext (deviceIterator))) {
        CFStringRef data;
        char name[128] = "";

        data = IORegistryEntrySearchCFProperty (service, kIOServicePlane, CFSTR("BSD Name"),
                                                kCFAllocatorDefault, kIORegistryIterateRecursively);

        if (NULL != data) {
            rc = CFStringGetCString (data, name, sizeof (name), kCFStringEncodingASCII);
            CFRelease (data);
            if (0 == strcmp (name, mmc->bsd_name)) {
                break;
            }
        }

        (void) IOObjectRelease (service);
    }

    IOObjectRelease (deviceIterator);

    *servp = service;

    return (service) ? 0 : -1;
}

static int iokit_find_interfaces (MMCDEV *mmc, io_service_t service) {
    IOCFPlugInInterface **plugInInterface = NULL;
    SInt32 score;
    int rc;

    rc = IOCreatePlugInInterfaceForService (service, kIOMMCDeviceUserClientTypeID,
                                            kIOCFPlugInInterfaceID, &plugInInterface,
                                            &score);
    if (kIOReturnSuccess != rc || NULL == plugInInterface) {
        return -1;
    }

    BD_DEBUG(DBG_MMC, "Getting MMC interface\n");

    rc = (*plugInInterface)->QueryInterface(plugInInterface,
                                            CFUUIDGetUUIDBytes(kIOMMCDeviceInterfaceID),
                                            (LPVOID)&mmc->mmcInterface);
    /* call release instead of IODestroyPlugInInterface to avoid stopping IOBDServices */
    (*plugInInterface)->Release(plugInInterface);
    if (kIOReturnSuccess != rc || NULL == mmc->mmcInterface) {
        BD_DEBUG(DBG_MMC, "Could not get multimedia commands (MMC) interface\n");
        return -1;
    }

    BD_DEBUG(DBG_MMC, "Have an MMC interface (%p). Getting a SCSI task interface...\n", (void*)mmc->mmcInterface);

    mmc->taskInterface = (*mmc->mmcInterface)->GetSCSITaskDeviceInterface (mmc->mmcInterface);
    if (NULL == mmc->taskInterface) {
        BD_DEBUG(DBG_MMC, "Could not get SCSI task device interface\n");
        return -1;
    }

    return 0;
}

static int mmc_open_iokit (const char *path, MMCDEV *mmc) {
    io_service_t service;
    int rc;

    mmc->mmcInterface = NULL;
    mmc->taskInterface = NULL;
    mmc->disk = NULL;
    mmc->session = NULL;
    mmc->is_mounted = true;

    /* get the bsd name associated with this mount */
    rc = get_mounted_device_from_path (mmc, path);
    if (0 != rc) {
        BD_DEBUG(DBG_MMC, "Could not locate mounted device associated with %s\n", path);
        return rc;
    }

    /* find a matching io service (IOBDServices) */
    rc = iokit_find_service_matching (mmc, &service);
    if (0 != rc) {
        BD_DEBUG(DBG_MMC, "Could not find matching IOBDServices mounted @ %s\n", path);
        return rc;
    }

    /* find mmc and scsi task interfaces */
    rc = iokit_find_interfaces (mmc, service);

    /* done with the ioservice. release it */
    (void) IOObjectRelease (service);

    if (0 != rc) {
        return rc;
    }

    /* unmount the disk so exclusive access can be obtained (this is required
       to use the scsi task interface) */
    rc = iokit_unmount (mmc);
    if (0 != rc) {
        return rc;
    }

    /* finally, obtain exclusive access */
    rc = (*mmc->taskInterface)->ObtainExclusiveAccess (mmc->taskInterface);
    if (kIOReturnSuccess != rc) {
        BD_DEBUG(DBG_MMC, "Failed to obtain exclusive access. rc = %x\n", rc);
        return -1;
    }

    BD_DEBUG(DBG_MMC, "MMC Open complete\n");

    return 0;
}

MMCDEV *device_open(const char *path)
{
    MMCDEV *dev;
    int     rc;

    dev = calloc(1, sizeof(MMCDEV));
    if (!dev) {
        BD_DEBUG(DBG_MKB | DBG_CRIT, "out of memory\n");
        return NULL;
    }

    rc = mmc_open_iokit (path, dev);
    if (0 != rc) {
        device_close (dev);
        return NULL;
    }

    return dev;
}

void device_close(MMCDEV **pp)
{
    if (pp && *pp) {
        MMCDEV *mmc = *pp;

        if (mmc->taskInterface) {
            (*mmc->taskInterface)->ReleaseExclusiveAccess (mmc->taskInterface);
            (*mmc->taskInterface)->Release (mmc->taskInterface);
            mmc->taskInterface = NULL;
        }

        if (mmc->mmcInterface) {
            (*mmc->mmcInterface)->Release (mmc->mmcInterface);
            mmc->mmcInterface = NULL;
        }

        (void) iokit_mount (mmc);

        X_FREE(*pp);
    }
}
