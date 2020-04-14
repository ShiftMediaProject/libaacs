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
#include <IOKit/IOBSD.h>
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
enum disk_state_e {
    disk_mounted,
    disk_unmounted,
    disk_appeared,
    disk_mounting
};

struct mmcdev {
    /* Interfaces required for low-level device communication */
    IOCFPlugInInterface **plugInInterface;
    MMCDeviceInterface **mmcInterface;
    SCSITaskDeviceInterface **taskInterface;

    /* device short name (ie disk1) */
    char bsd_name[MNAMELEN];

    /* for mounting/unmounting the disc */
    DADiskRef disk;
    DASessionRef session;
    enum disk_state_e disk_state;
    dispatch_semaphore_t sync_sem;
    dispatch_queue_t background_queue;
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
        task = (*mmc->taskInterface)->CreateSCSITask(mmc->taskInterface);
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

        rc = (*task)->SetCommandDescriptorBlock(task, cdb, kSCSICDBSize_16Byte);
        if (kIOReturnSuccess != rc) {
            BD_DEBUG(DBG_MMC, "Error setting SCSI command\n");
            break;
        }

        rc = (*task)->SetScatterGatherEntries(task, &iov, 1, iov.length, direction);
        if (kIOReturnSuccess != rc) {
            BD_DEBUG(DBG_MMC, "Error setting SCSI scatter gather entries\n");
            break;
        }

        rc = (*task)->SetTimeoutDuration(task, 5000000);
        if (kIOReturnSuccess != rc) {
            BD_DEBUG(DBG_MMC, "Error setting SCSI command timeout\n");
            break;
        }

        memset(&sense, 0, sizeof (sense));

        rc = (*task)->ExecuteTaskSync(task, &sense, &status, &sent);

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

        (*task)->Release(task);

        return 1;
    } while (0);

    if (task) {
        (*task)->Release(task);
    }

    return 0;
}

static int get_mounted_device_from_path(MMCDEV *mmc, const char *path) {
  struct statfs stat_info;
  int rc;

  rc = statfs(path, &stat_info);
  if (0 != rc) {
    return rc;
  }

  strncpy(mmc->bsd_name, basename (stat_info.f_mntfromname), sizeof (mmc->bsd_name));

  return 0;
}

static void iokit_unmount_complete(DADiskRef disk, DADissenterRef dissenter,
                                   void *context) {
    (void)disk; /* suppress warning */
    MMCDEV *mmc = context;

    if (dissenter) {
        BD_DEBUG(DBG_MMC, "Could not unmount the disc\n");
    } else {
        BD_DEBUG(DBG_MMC, "Disc unmounted\n");
        mmc->disk_state = disk_unmounted;
    }
    dispatch_semaphore_signal(mmc->sync_sem);
}

static void iokit_mount_complete(DADiskRef disk, DADissenterRef dissenter,
                                 void *context) {
    (void) disk; /* suppress warning */
    MMCDEV *mmc = context;

    if (dissenter) {
        DAReturn code = DADissenterGetStatus(dissenter);
        BD_DEBUG(DBG_MMC, "Could not mount the disc (%8X)\n", code);
        mmc->disk_state = disk_unmounted;
    } else {
        BD_DEBUG(DBG_MMC, "Disc mounted\n");
        mmc->disk_state = disk_mounted;
    }
    dispatch_semaphore_signal(mmc->sync_sem);
}

/* Unmount the disk at mmc->disk
 * Note: This MAY NOT be called on the background queue,
 *       as that would lead to a deadlock.
 */
static int iokit_unmount(MMCDEV *mmc) {
    if (disk_unmounted == mmc->disk_state) {
        return 0; /* nothing to do */
    }

    BD_DEBUG(DBG_MMC, "Unmounting disk\n");

    DADiskUnmount(mmc->disk, kDADiskUnmountOptionForce, iokit_unmount_complete, mmc);
    dispatch_semaphore_wait(mmc->sync_sem, DISPATCH_TIME_FOREVER);

    return (mmc->disk_state == disk_unmounted) ? 0 : -1;
}

/* Mount the disk at mmc->disk
 * Note: This MAY NOT be called on the background queue,
 *       as that would lead to a deadlock.
 */
static int iokit_mount(MMCDEV *mmc) {
    if (disk_mounted != mmc->disk_state) {
        if (mmc->disk && mmc->session) {
            mmc->disk_state = disk_mounting;
            DADiskMount(mmc->disk, NULL, kDADiskMountOptionDefault, iokit_mount_complete, mmc);
            dispatch_semaphore_wait(mmc->sync_sem, DISPATCH_TIME_FOREVER);
        }
    }
    return (mmc->disk_state == disk_unmounted) ? 0 : -1;
}

static int iokit_find_service_matching(MMCDEV *mmc, io_service_t *servp) {
    CFMutableDictionaryRef matchingDict = IOServiceMatching("IOBDServices");
    io_iterator_t deviceIterator;
    io_service_t service;
    int rc;

    assert(NULL != servp);

    *servp = IO_OBJECT_NULL;

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

    while (0 != (service = IOIteratorNext(deviceIterator))) {
        CFStringRef data;
        char name[MNAMELEN] = "";

        data = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR(kIOBSDNameKey),
                                               kCFAllocatorDefault, kIORegistryIterateRecursively);

        if (NULL != data) {
            rc = CFStringGetCString(data, name, sizeof (name), kCFStringEncodingASCII);
            CFRelease(data);
            if (0 == strcmp(name, mmc->bsd_name)) {
                break;
            }
        }

        (void) IOObjectRelease(service);
    }

    IOObjectRelease(deviceIterator);

    *servp = service;

    return (service != IO_OBJECT_NULL) ? 0 : -1;
}

static int iokit_find_interfaces(MMCDEV *mmc, io_service_t service) {
    SInt32 score;
    int rc;

    rc = IOCreatePlugInInterfaceForService(service, kIOMMCDeviceUserClientTypeID,
                                           kIOCFPlugInInterfaceID, &mmc->plugInInterface,
                                           &score);
    if (kIOReturnSuccess != rc || NULL == mmc->plugInInterface) {
        return -1;
    }

    BD_DEBUG(DBG_MMC, "Getting MMC interface\n");
    IOCFPlugInInterface **plugInInterface = mmc->plugInInterface;

    rc = (*plugInInterface)->QueryInterface(plugInInterface,
                                            CFUUIDGetUUIDBytes(kIOMMCDeviceInterfaceID),
                                            (LPVOID)&mmc->mmcInterface);

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

static DADissenterRef iokit_mount_approval_cb(DADiskRef disk, void *context)
{
    MMCDEV *mmc = context;

    /* If the disk state is mounted, there is nothing to do here. */
    if (disk_mounted == mmc->disk_state) {
        return NULL;
    }

    /* Check if the disk that is to be mounted matches ours,
     * if not, we do not need to reject mounting.
     */
    if (!CFEqual(disk, mmc->disk)) {
        return NULL;
    }

    BD_DEBUG(DBG_MMC, "Mount approval request for matching disc\n");

    /* When we are trying to mount, the mount approval callback is
     * called too, so we need to allow mounting for ourselves here.
     */
    if (disk_mounting == mmc->disk_state) {
        BD_DEBUG(DBG_MMC, "Allowing ourselves to mount\n");
        return NULL;
    }

    mmc->disk_state = disk_appeared;
    dispatch_semaphore_signal(mmc->sync_sem);

    CFStringRef reason = CFSTR("Disk is going to be mounted libaacs");
    return DADissenterCreate(kCFAllocatorDefault, kDAReturnBusy, reason);
}

static int iokit_da_init(MMCDEV *mmc) {
    mmc->session = DASessionCreate(kCFAllocatorDefault);
    if (NULL == mmc->session) {
        BD_DEBUG(DBG_MMC, "Could not create a disc arbitration session\n");
        return -1;
    }

    mmc->disk = DADiskCreateFromBSDName(kCFAllocatorDefault, mmc->session, mmc->bsd_name);
    if (NULL == mmc->disk) {
        BD_DEBUG(DBG_MMC, "Could not create a disc arbitration disc for the device\n");
        CFRelease(mmc->session);
        mmc->session = NULL;
        return -1;
    }

    mmc->background_queue = dispatch_queue_create("org.videolan.libaacs", DISPATCH_QUEUE_SERIAL);
    DASessionSetDispatchQueue(mmc->session, mmc->background_queue);

    // Register event callbacks
    DARegisterDiskMountApprovalCallback(mmc->session, NULL, iokit_mount_approval_cb, mmc);

    mmc->sync_sem = dispatch_semaphore_create(0);

    return 0;
}

static void iokit_da_destroy(MMCDEV *mmc) {
    if (mmc->session) {
        /* The approval callback must be unregistered here, doing it in the
         * mount approval callback instead after we got a matching disk would
         * cause the OS to immediately re-try to mount the disk faster than we
         * can mount it.
         */
        DAUnregisterApprovalCallback(mmc->session, iokit_mount_approval_cb, mmc);
        DASessionSetDispatchQueue(mmc->session, NULL);
        CFRelease(mmc->session);
        mmc->session = NULL;
    }

    if (mmc->disk) {
        CFRelease(mmc->disk);
        mmc->disk = NULL;
    }

    dispatch_release(mmc->sync_sem);
}

static int mmc_open_iokit(const char *path, MMCDEV *mmc) {
    io_service_t service;
    int rc;

    mmc->plugInInterface = NULL;
    mmc->mmcInterface = NULL;
    mmc->taskInterface = NULL;
    mmc->disk = NULL;
    mmc->session = NULL;
    mmc->disk_state = disk_mounted;

    /* get the bsd name associated with this mount */
    rc = get_mounted_device_from_path(mmc, path);
    if (0 != rc) {
        BD_DEBUG(DBG_MMC, "Could not locate mounted device associated with %s\n", path);
        return rc;
    }

    /* find a matching io service (IOBDServices) */
    rc = iokit_find_service_matching(mmc, &service);
    if (0 != rc) {
        BD_DEBUG(DBG_MMC, "Could not find matching IOBDServices mounted @ %s\n", path);
        return rc;
    }

    /* find mmc and scsi task interfaces */
    rc = iokit_find_interfaces(mmc, service);

    /* done with the ioservice. release it */
    (void) IOObjectRelease(service);

    /* Init DiskArbitration */
    rc = iokit_da_init(mmc);
    if (0 != rc) {
        return rc;
    }

    /* unmount the disk so exclusive access can be obtained (this is required
       to use the scsi task interface) */
    rc = iokit_unmount(mmc);
    if (0 != rc) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Failed to unmount the disc at %s\n", path);
        return rc;
    }

    /* finally, obtain exclusive access */
    rc = (*mmc->taskInterface)->ObtainExclusiveAccess(mmc->taskInterface);
    if (kIOReturnSuccess != rc) {
        BD_DEBUG(DBG_MMC, "Failed to obtain exclusive access. rc = %x\n", rc);
        return -1;
    }

    BD_DEBUG(DBG_MMC, "MMC Open complete\n");

    return 0;
}

MMCDEV *device_open(const char *path)
{
    MMCDEV *mmc;
    int     rc;

    mmc = calloc(1, sizeof(MMCDEV));
    if (!mmc) {
        BD_DEBUG(DBG_MKB | DBG_CRIT, "out of memory\n");
        return NULL;
    }

    rc = mmc_open_iokit(path, mmc);
    if (0 != rc) {
        device_close(&mmc);
        return NULL;
    }

    return mmc;
}

void device_close(MMCDEV **pp)
{
    __block int rc = 0;
    if (pp && *pp) {
        MMCDEV *mmc = *pp;

        /* When the exclusive access to the drive is released,
         * the OS will see the device like a "new" device and
         * try to mount it. Therefore we can't just mount the
         * disk we previously got immediately here as it would
         * fail with kDAReturnBadArgument as the disk is not
         * available yet.
         * Trying to mount the disk after it appears in peek
         * does not work either as the disk is not yet ready
         * or in the process of being mounted by the OS so
         * that would return an kDAReturnBusy error.
         * The only way that seems to reliably work is to use
         * a mount approval callback. When the OS tries to
         * mount the disk, the mount approval callback is
         * called and we can reject mounting and then proceed
         * to mount the disk ourselves.
         * Claiming exclusive access using DADiskClaim in order
         * to prevent the OS form mounting the disk does not work
         * either!
         */

        if (mmc->taskInterface) {
            (*mmc->taskInterface)->ReleaseExclusiveAccess(mmc->taskInterface);
            (*mmc->taskInterface)->Release(mmc->taskInterface);
            mmc->taskInterface = NULL;
        }

        if (mmc->mmcInterface) {
            (*mmc->mmcInterface)->Release(mmc->mmcInterface);
            mmc->mmcInterface = NULL;
        }

        if (mmc->plugInInterface) {
            IODestroyPlugInInterface(mmc->plugInInterface);
        }

        /* Wait for disc to re-appear for 20 seconds.
         * This timeout was figured out by experimentation with
         * a USB BD drive which sometimes can take really long to
         * be in a mountable state again.
         * For internal drives this is probably much faster
         * so the long timeout shouldnt do much harm for thse
         * cases.
         */
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 20 * 1E+9);
        dispatch_semaphore_wait(mmc->sync_sem, timeout);

        /* It is crucial that this is done on the event handling queue
         * else callbacks could be received while this code runs.
         */
        dispatch_sync(mmc->background_queue, ^{
            if (disk_appeared != mmc->disk_state) {
                BD_DEBUG(DBG_MMC | DBG_CRIT, "Timeout waiting for the disc to appear again!\n");
                iokit_da_destroy(mmc);
                rc = -1;
                return;
            }
            rc = 0;
        });

        if (rc == 0) {
            /* Disk appeared successfully, mount it.
             * Return value is ignored as logging of success or
             * error takes place in the callback already and there
             * is nothing we can do really if mounting fails.
             */
            (void) iokit_mount(mmc);
            iokit_da_destroy(mmc);
        }
        X_FREE(*pp);
    }
}
