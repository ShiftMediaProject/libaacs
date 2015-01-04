/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010-2013  npzacs
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

#include "mmc.h"
#include "crypto.h"
#include "file/path.h"
#include "util/macro.h"
#include "util/logging.h"

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

#ifdef USE_IOKIT

#include <Carbon/Carbon.h>

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

#endif

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_LINUX_CDROM_H
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <winsock.h>
#endif

#ifndef DEBUG_KEYS
#define DEBUG_KEYS 0
#endif

#if defined(_WIN32)
/*
 * from ntddscsi.h, Windows DDK
 */
#   define SCSI_IOCTL_DATA_OUT             0
#   define SCSI_IOCTL_DATA_IN              1
#   define SCSI_IOCTL_DATA_UNSPECIFIED     2
#   define IOCTL_SCSI_PASS_THROUGH_DIRECT  0x4D014
#   define MAX_SENSE_LEN                   18

typedef struct _SCSI_PASS_THROUGH_DIRECT {
    USHORT Length;
    UCHAR  ScsiStatus;
    UCHAR  PathId;
    UCHAR  TargetId;
    UCHAR  Lun;
    UCHAR  CdbLength;
    UCHAR  SenseInfoLength;
    UCHAR  DataIn;
    ULONG  DataTransferLength;
    ULONG  TimeOutValue;
    PVOID  DataBuffer;
    ULONG  SenseInfoOffset;
    UCHAR  Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

#endif // defined(_WIN32)

struct mmc {
#if defined(USE_IOKIT)
    MMCDeviceInterface **mmcInterface;
    SCSITaskDeviceInterface **taskInterface;

    /* device short name (ie disk1) */
    char bsd_name[128];

    /* for mounting/unmounting the disc */
    DADiskRef disk;
    DASessionRef session;
    bool is_mounted;
#elif defined(_WIN32)
    HANDLE fd;
#else
    int    fd;
#endif
    uint8_t host_nonce[20];
    uint8_t host_key[20];
    uint8_t host_key_point[40];

    uint8_t drive_cert[92];

    uint8_t read_drive_cert;
};

static int _mmc_send_cmd(MMC *mmc, const uint8_t *cmd, uint8_t *buf, size_t tx,
                         size_t rx)
{
#if defined(HAVE_LINUX_CDROM_H)
    if (mmc->fd >= 0) {
        struct cdrom_generic_command cgc;
        struct request_sense sense;

        memset(&cgc, 0, sizeof(cgc));
        memcpy(cgc.cmd, cmd, CDROM_PACKET_SIZE);
        cgc.sense = &sense;
        cgc.timeout = 5000;

        if (buf) {
            if (tx) {
                cgc.data_direction = CGC_DATA_WRITE;
                cgc.buflen = tx;
                cgc.buffer = buf;
            } else if (rx) {
                cgc.data_direction = CGC_DATA_READ;
                cgc.buflen = rx;
                cgc.buffer = buf;
            }
        } else {
            cgc.data_direction = CGC_DATA_NONE;
            cgc.buflen = 0;
            cgc.buffer = NULL;
        }

        int a = ioctl(mmc->fd, CDROM_SEND_PACKET, &cgc);

        char str[512];
        DEBUG(DBG_MMC, "Send LINUX MMC cmd %s:\n",
              print_hex(str, cmd, 16));
        if (tx) {
            DEBUG(DBG_MMC, "  Buffer: %s ->\n", print_hex(str, buf, tx>255?255:tx));
        } else {
            DEBUG(DBG_MMC, "  Buffer: %s <-\n", print_hex(str, buf, rx>255?255:rx));
        }

        if (a >= 0) {
            DEBUG(DBG_MMC, "  Send succeeded! [%d]\n", a);
            return 1;
        }

        DEBUG(DBG_MMC, "  Send failed! [%d] %s\n", a, strerror(errno));
    }

#elif defined(_WIN32)

    DWORD dwBytesReturned;

    struct {
        SCSI_PASS_THROUGH_DIRECT sptd;
        UCHAR                    SenseBuf[MAX_SENSE_LEN];
    } sptd_sb;

    if (mmc->fd == INVALID_HANDLE_VALUE) {
        return 0;
    }

    sptd_sb.sptd.Length          = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptd_sb.sptd.PathId          = 0;
    sptd_sb.sptd.TargetId        = 0;
    sptd_sb.sptd.Lun             = 0;
    sptd_sb.sptd.CdbLength       = 12;
    sptd_sb.sptd.SenseInfoLength = MAX_SENSE_LEN;
    sptd_sb.sptd.TimeOutValue    = 5;
    sptd_sb.sptd.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);

    if (buf) {
        if (tx) {
            sptd_sb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
            sptd_sb.sptd.DataTransferLength = tx;
            sptd_sb.sptd.DataBuffer = buf;
        } else if (rx) {
            sptd_sb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
            sptd_sb.sptd.DataTransferLength = rx;
            sptd_sb.sptd.DataBuffer = buf;
        }
    } else {
        sptd_sb.sptd.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
        sptd_sb.sptd.DataTransferLength = 0;
        sptd_sb.sptd.DataBuffer = NULL;
    }

    memcpy(sptd_sb.sptd.Cdb, cmd, 16);

    ZeroMemory(sptd_sb.SenseBuf, MAX_SENSE_LEN);

    if (DeviceIoControl(mmc->fd,
                        IOCTL_SCSI_PASS_THROUGH_DIRECT,
                        (void*)&sptd_sb, sizeof(sptd_sb),
                        (void*)&sptd_sb, sizeof(sptd_sb),
                        &dwBytesReturned, NULL)) {

        if (sptd_sb.sptd.ScsiStatus == 0 /* STATUS_GOOD */) {
            DEBUG(DBG_MMC, "  Send succeeded!\n");
            return 1;
        }
    }

    DEBUG(DBG_MMC, "  Send failed!\n");

#elif defined(USE_IOKIT)
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
            DEBUG(DBG_MMC, "Could not create SCSI Task\n");
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

        rc = (*task)->SetCommandDescriptorBlock (task, cmd, 16);
        if (kIOReturnSuccess != rc) {
            DEBUG(DBG_MMC, "Error setting SCSI command\n");
            break;
        }

        rc = (*task)->SetScatterGatherEntries (task, &iov, 1, iov.length, direction);
        if (kIOReturnSuccess != rc) {
            DEBUG(DBG_MMC, "Error setting SCSI scatter gather entries\n");
            break;
        }

        rc = (*task)->SetTimeoutDuration (task, 5000000);
        if (kIOReturnSuccess != rc) {
            DEBUG(DBG_MMC, "Error setting SCSI command timeout\n");
            break;
        }

        memset (&sense, 0, sizeof (sense));

        rc = (*task)->ExecuteTaskSync (task, &sense, &status, &sent);

        char str[512];
        DEBUG(DBG_MMC, "Send SCSI MMC cmd %s:\n", print_hex(str, cmd, 16));
        if (tx) {
            DEBUG(DBG_MMC, "  Buffer: %s ->\n", print_hex(str, buf, tx>255?255:tx));
        } else {
            DEBUG(DBG_MMC, "  Buffer: %s <-\n", print_hex(str, buf, rx>255?255:rx));
        }

        if (kIOReturnSuccess != rc || status != 0) {
            DEBUG(DBG_MMC, "  Send failed!\n");
            break;
        } else {
            DEBUG(DBG_MMC, "  Send succeeded! sent = %lld status = %u. response = %x\n",
                  (unsigned long long) sent, status, sense.VALID_RESPONSE_CODE);
        }

        (*task)->Release (task);

        return 1;
    } while (0);

    if (task) {
        (*task)->Release (task);
    }
#endif

    return 0;
}

/*
 *
 */

static int _mmc_report_key(MMC *mmc, uint8_t agid, uint32_t addr,
                           uint8_t blocks, uint8_t format, uint8_t *buf,
                           uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, len);

    DEBUG(DBG_MMC, "MMC report key...\n");

    cmd[0] = 0xa4;
    cmd[2] = (addr >> 24) & 0xff;
    cmd[3] = (addr >> 16) & 0xff;
    cmd[4] = (addr >> 8) & 0xff;
    cmd[5] = addr & 0xff;
    cmd[6] = blocks;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = (agid << 6) | (format & 0x3f);

    return _mmc_send_cmd(mmc, cmd, buf, 0, len);
}

static int _mmc_send_key(MMC *mmc, uint8_t agid, uint8_t format, uint8_t *buf,
                         uint16_t len)
{
    uint8_t cmd[16];
    char str[512];
    memset(cmd, 0, sizeof(cmd));

    DEBUG(DBG_MMC, "MMC send key [%d] %s...\n", len, print_hex(str, buf, len));

    cmd[0] = 0xa3;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = (agid << 6) | (format & 0x3f);

    DEBUG(DBG_MMC, "cmd: %s\n", print_hex(str, cmd, 16));
    return _mmc_send_cmd(mmc, cmd, buf, len, 0);
}

static int _mmc_report_disc_structure(MMC *mmc, uint8_t agid, uint8_t format,
                                      uint8_t layer, uint32_t address,
                                      uint8_t *buf, uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, len);

    DEBUG(DBG_MMC, "MMC report disc structure [format 0x%x layer %d address %d] ...\n", format, layer, address);

    cmd[0] = 0xad; // operation code
    cmd[1] = 0x01; // BluRay
    cmd[2] = (address >> 24) & 0xff;
    cmd[3] = (address >> 16) & 0xff;
    cmd[4] = (address >> 8) & 0xff;
    cmd[5] = address & 0xff;
    cmd[6] = layer;
    cmd[7] = format;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = (agid << 6);

    return _mmc_send_cmd(mmc, cmd, buf, 0, len);
}

static int _mmc_get_configuration(MMC *mmc, uint16_t feature, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, len);

    DEBUG(DBG_MMC, "MMC get configuration [feature 0x%x] ...\n", feature);

    cmd[0] = 0x46; // operation code
    cmd[1] = 0x01; // BluRay
    cmd[2] = (feature >> 8) & 0xff;
    cmd[3] = feature & 0xff;
    cmd[7] = (len >> 8) & 0xff;
    cmd[8] = len & 0xff;

    return _mmc_send_cmd(mmc, cmd, buf, 0, len);
}

/*
 *
 */

static int _mmc_invalidate_agid(MMC *mmc, uint8_t agid)
{
    uint8_t buf[2];
    memset(buf, 0, sizeof(buf));

    return _mmc_report_key(mmc, agid, 0, 0, 0x3f, buf, 2);
}

static void _mmc_invalidate_agids(MMC *mmc)
{
    int     agid;

    /* invalidate all agids */
    for (agid = 0; agid < 4; agid++) {
        _mmc_invalidate_agid(mmc, agid);
    }
}

static int _mmc_report_agid(MMC *mmc, uint8_t *agid)
{
    uint8_t buf[8];
    memset(buf, 0, sizeof(buf));

    int result = _mmc_report_key(mmc, 0, 0, 0, 0, buf, 8);
    if (result) {
        *agid = (buf[7] & 0xff) >> 6;
    }
    return result;
}

static int _mmc_send_host_cert(MMC *mmc, uint8_t agid,
                               const uint8_t *host_nonce,
                               const uint8_t *host_cert)
{
    uint8_t buf[116];
    memset(buf, 0, sizeof(buf));

    buf[1] = 0x72;
    memcpy(buf + 4,  host_nonce, 20);
    memcpy(buf + 24, host_cert,  92);

    return _mmc_send_key(mmc, agid, 0x01, buf, 116);
}

static int _mmc_read_drive_cert_challenge(MMC *mmc, uint8_t agid, uint8_t *drive_nonce,
                                          uint8_t *drive_cert)
{
    uint8_t buf[116];
    memset(buf, 0, sizeof(buf));

    if (_mmc_report_key(mmc, agid, 0, 0, 0x01, buf, 116)) {
        memcpy(drive_nonce, buf + 4,  20);
        memcpy(drive_cert,  buf + 24, 92);
        return 1;
    }
    return 0;
}

static int _mmc_read_drive_key(MMC *mmc, uint8_t agid, uint8_t *drive_key_point,
                               uint8_t *drive_key_signature)
{
    uint8_t buf[84];
    memset(buf, 0, sizeof(buf));

    if (_mmc_report_key(mmc, agid, 0, 0, 0x02, buf, 84)) {
        memcpy(drive_key_point,     buf + 4,  40);
        memcpy(drive_key_signature, buf + 44, 40);
        return 1;
    }
    return 0;
}


static int _mmc_check_aacs(MMC *mmc)
{
    uint8_t buf[16];
    memset(buf, 0, sizeof(buf));

    if (_mmc_get_configuration(mmc, 0x010d, buf, 16)) {
        uint16_t feature = MKINT_BE16(buf+8);
        if (feature == 0x010d) {
            DEBUG(DBG_MMC, "AACS feature descriptor:\n");
            DEBUG(DBG_MMC, "  AACS version: %d\n", buf[7+8]);
            DEBUG(DBG_MMC, "  AACS active: %d\n", buf[2+8] & 1);
            DEBUG(DBG_MMC, "  Binding Nonce generation support: %d\n", buf[4+8] & 1);
            DEBUG(DBG_MMC, "  Binding Nonce block count: %d\n", buf[5+8]);
            DEBUG(DBG_MMC, "  Bus encryption support: %d\n", !!(buf[4+8] & 2));
            DEBUG(DBG_MMC, "  Read drive certificate: %d\n", !!(buf[4+8] & 0x10));
            DEBUG(DBG_MMC, "  AGID count: %d\n", buf[6+8] & 0xf);

            mmc->read_drive_cert = !!(buf[4+8] & 0x10);

            return buf[2+8] & 1;
        }
        DEBUG(DBG_MMC, "incorrect feature ID %04x\n", feature);
    }

    DEBUG(DBG_MMC, "_mmc_get_configuration() failed\n");
    return 0;
}

static uint8_t *_mmc_read_mkb(MMC *mmc, uint8_t agid, int address, int *size)
{
    const int layer = 0;
    uint8_t *mkb = NULL;
    uint8_t buf[32772];

    *size = 0;
    if (_mmc_report_disc_structure(mmc, agid, 0x83, layer, address, buf, sizeof(buf))) {
        unsigned pack, num_packs = buf[3];
        int32_t  len = MKINT_BE16(buf) - 2;
        mkb = malloc(32768 * num_packs);

        DEBUG(DBG_MMC, "got mkb: pack 0/%d %d bytes\n", num_packs, len);
        memcpy(mkb, buf + 4, len);
       *size += len;

        for (pack = 1; pack < num_packs; pack++) {
            if (_mmc_report_disc_structure(mmc, agid, 0x83, layer, pack, buf, sizeof(buf))) {
                len = MKINT_BE16(buf) - 2;
                DEBUG(DBG_MMC, "got mkb: pack %d/%d %d bytes\n", pack, num_packs, len);

                memcpy(mkb + *size, buf + 4, len);
                *size += len;
            } else {
                break;
            }
        }
    }

    return mkb;
}

static int _mmc_send_host_key(MMC *mmc, uint8_t agid,
                              const uint8_t *host_key_point,
                              const uint8_t *host_key_signature)
{
    uint8_t buf[84];
    memset(buf, 0, sizeof(buf));

    buf[1] = 0x52;
    memcpy(buf + 4,  host_key_point,     40);
    memcpy(buf + 44, host_key_signature, 40);

    return _mmc_send_key(mmc, agid, 0x02, buf, 84);
}

static int _mmc_read_vid(MMC *mmc, uint8_t agid, uint8_t *volume_id,
                         uint8_t *mac)
{
    uint8_t buf[36];
    memset(buf, 0, sizeof(buf));

    if (_mmc_report_disc_structure(mmc, agid, 0x80, 0, 0, buf, 36)) {
        memcpy(volume_id, buf + 4,  16);
        memcpy(mac,       buf + 20, 16);
        return 1;
    }

    return 0;
}

static int _mmc_read_pmsn(MMC *mmc, uint8_t agid, uint8_t *pmsn,
                          uint8_t *mac)
{
    uint8_t buf[36];
    memset(buf, 0, sizeof(buf));

    if (_mmc_report_disc_structure(mmc, agid, 0x81, 0, 0, buf, 36)) {
        memcpy(pmsn, buf + 4,  16);
        memcpy(mac,  buf + 20, 16);
        return 1;
    }

    return 0;
}

static int _mmc_read_data_keys(MMC *mmc, uint8_t agid, uint8_t *read_data_key, uint8_t *write_data_key)
{
    uint8_t buf[36];

    if (_mmc_report_disc_structure(mmc, agid, 0x84, 0, 0, buf, 36)) {
        memcpy(read_data_key, buf + 4, 16);
        memcpy(write_data_key, buf + 20, 16);
        return 1;
    }

    return 0;
}

#ifdef USE_IOKIT
static int get_mounted_device_from_path (MMC *mmc, const char *path) {
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
        DEBUG(DBG_MMC, "Could not unmount the disc\n");
    } else {
        DEBUG(DBG_MMC, "Disc unmounted\n");
        ((MMC *)context)->is_mounted = 0;
    }
}

static void iokit_mount_complete (DADiskRef disk, DADissenterRef dissenter,
                                  void *context) {
    (void) disk; /* suppress warning */
    (void) dissenter; /* suppress warning */

    /* the disc mounts despite whether there is a dessenter */
    DEBUG(DBG_MMC, "Disc mounted\n");
    ((MMC *)context)->is_mounted = 1;
}

static int iokit_unmount (MMC *mmc) {
    if (0 == mmc->is_mounted) {
        return 0; /* nothing to do */
    }

    DEBUG(DBG_MMC, "Unmounting disk\n");

    mmc->session = DASessionCreate (kCFAllocatorDefault);
    if (NULL == mmc->session) {
        DEBUG(DBG_MMC, "Could not create a disc arbitration session\n");
        return -1;
    }

    mmc->disk = DADiskCreateFromBSDName (kCFAllocatorDefault, mmc->session, mmc->bsd_name);
    if (NULL == mmc->disk) {
        DEBUG(DBG_MMC, "Could not create a disc arbitration disc for the device\n");
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

static int iokit_mount (MMC *mmc) {
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

static int iokit_find_service_matching (MMC *mmc, io_service_t *servp) {
    CFMutableDictionaryRef matchingDict = IOServiceMatching("IOBDServices");
    io_iterator_t deviceIterator;
    io_service_t service;
    int rc;

    assert (NULL != servp);

    *servp = 0;

    if (!matchingDict) {
        DEBUG(DBG_MMC, "Could not create a matching dictionary for IOBDServices\n");
        return -1;
    }

    /* this call consumes the reference to the matchingDict. we do not need to release it */
    rc = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &deviceIterator);
    if (kIOReturnSuccess != rc) {
        DEBUG(DBG_MMC, "Could not create device iterator\n");
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

static int iokit_find_interfaces (MMC *mmc, io_service_t service) {
    IOCFPlugInInterface **plugInInterface = NULL;
    SInt32 score;
    int rc;

    rc = IOCreatePlugInInterfaceForService (service, kIOMMCDeviceUserClientTypeID,
                                            kIOCFPlugInInterfaceID, &plugInInterface,
                                            &score);
    if (kIOReturnSuccess != rc || NULL == plugInInterface) {
        return -1;
    }

    DEBUG(DBG_MMC, "Getting MMC interface\n");

    rc = (*plugInInterface)->QueryInterface(plugInInterface,
                                            CFUUIDGetUUIDBytes(kIOMMCDeviceInterfaceID),
                                            (LPVOID)&mmc->mmcInterface);
    /* call release instead of IODestroyPlugInInterface to avoid stopping IOBDServices */
    (*plugInInterface)->Release(plugInInterface);
    if (kIOReturnSuccess != rc || NULL == mmc->mmcInterface) {
        DEBUG(DBG_MMC, "Could not get multimedia commands (MMC) interface\n");
        return -1;
    }

    DEBUG(DBG_MMC, "Have an MMC interface (%p). Getting a SCSI task interface...\n", (void*)mmc->mmcInterface);

    mmc->taskInterface = (*mmc->mmcInterface)->GetSCSITaskDeviceInterface (mmc->mmcInterface);
    if (NULL == mmc->taskInterface) {
        DEBUG(DBG_MMC, "Could not get SCSI task device interface\n");
        return -1;
    }

    return 0;
}

int mmc_open_iokit (const char *path, MMC *mmc) {
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
        DEBUG(DBG_MMC, "Could not locate mounted device associated with %s\n", path);
        return rc;
    }

    /* find a matching io service (IOBDServices) */
    rc = iokit_find_service_matching (mmc, &service);
    if (0 != rc) {
        DEBUG(DBG_MMC, "Could not find matching IOBDServices mounted @ %s\n", path);
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
        DEBUG(DBG_MMC, "Failed to obtain exclusive access. rc = %x\n", rc);
        return -1;
    }

    DEBUG(DBG_MMC, "MMC Open complete\n");

    return 0;
}
#endif

MMC *mmc_open(const char *path)
{
    MMC *mmc = calloc(1, sizeof(MMC));

    crypto_create_nonce(mmc->host_nonce, sizeof(mmc->host_nonce));

    if (DEBUG_KEYS) {
        char str[sizeof(mmc->host_nonce)*2 + 1];
        DEBUG(DBG_MMC, "Created host nonce (Hn): %s\n",
              print_hex(str, mmc->host_nonce, sizeof(mmc->host_nonce)));
    }

    crypto_create_host_key_pair(mmc->host_key, mmc->host_key_point);

    if (DEBUG_KEYS) {
        char    str[sizeof(mmc->host_key_point)*2 + 1];
        DEBUG(DBG_MMC, "Created host key (Hk): %s\n",
              print_hex(str, mmc->host_key, sizeof(mmc->host_key)));
        DEBUG(DBG_MMC, "Created host key point (Hv): %s\n",
              print_hex(str, mmc->host_key_point, sizeof(mmc->host_key_point)));
    }

#if defined(USE_IOKIT)
    int rc = mmc_open_iokit (path, mmc);
    if (0 != rc) {
        mmc_close (mmc);
        return NULL;
    }

#elif defined(HAVE_MNTENT_H)

    char file_path [AACS_PATH_MAX];
    if (!aacs_resolve_path(path, file_path)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Failed resolving path %s\n", path);
        X_FREE(mmc);
        return NULL;
    }

    int   path_len  = strlen(file_path);
    FILE *proc_mounts;

    mmc->fd = -1;

    // strip trailing '/'s
    while (path_len > 0 && file_path[--path_len] == '/') {
        file_path[path_len] = '\0';
    }

    struct stat st;
    if (!stat(file_path, &st) && S_ISBLK(st.st_mode)) {
        DEBUG(DBG_MMC, "Opening block device %s\n", file_path);
        mmc->fd = open(file_path, O_RDONLY | O_NONBLOCK);
        if (mmc->fd < 0) {
            DEBUG(DBG_MMC | DBG_CRIT, "Error opening block device %s\n", file_path);
        }

    } else if ((proc_mounts = setmntent("/proc/mounts", "r"))) {
        struct mntent* mount_entry;

        DEBUG(DBG_MMC, "Opening LINUX MMC drive mounted to %s...\n", file_path);
        while ((mount_entry = getmntent(proc_mounts)) != NULL) {
            if (strcmp(mount_entry->mnt_dir, file_path) == 0) {
                mmc->fd = open(mount_entry->mnt_fsname, O_RDONLY | O_NONBLOCK);
                if (mmc->fd >= 0) {
                    DEBUG(DBG_MMC, "LINUX MMC drive %s opened - fd: %d\n",
                          mount_entry->mnt_fsname, mmc->fd);
                    break;
                }

                DEBUG(DBG_MMC, "Failed opening LINUX MMC drive %s mounted to %s\n",
                      mount_entry->mnt_fsname, file_path);
            }
        }

        endmntent(proc_mounts);

        if (mmc->fd < 0) {
            DEBUG(DBG_MMC | DBG_CRIT, "Error opening LINUX MMC drive mounted to %s\n", file_path);
        }

    } else {
        DEBUG(DBG_MMC | DBG_CRIT, "Error opening /proc/mounts\n");
    }

    if (mmc->fd < 0) {
        X_FREE(mmc);
    }

#elif defined(_WIN32)
    char drive[] = { path[0], ':', '\\', 0 };
    char volume[] = {'\\', '\\', '.', '\\', path[0], ':', 0};

    DEBUG(DBG_MMC, "Opening Windows MMC drive %s...\n", drive);

    UINT type = GetDriveType(drive);

    if (type != DRIVE_CDROM) {
        DEBUG(DBG_MMC | DBG_CRIT, "Drive %s is not CD/DVD drive\n", drive);
        X_FREE(mmc);
        return NULL;
    }

    mmc->fd = CreateFile(volume, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (mmc->fd == INVALID_HANDLE_VALUE) {
        mmc->fd = CreateFile(volume, GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (mmc->fd == INVALID_HANDLE_VALUE) {
            DEBUG(DBG_MMC, "Failed opening Windows MMC drive %s\n", volume);
            X_FREE(mmc);
            return NULL;
        }
    }

    DEBUG(DBG_MMC, "Windows MMC drive %s opened\n", volume);

#else

    DEBUG(DBG_MMC | DBG_CRIT, "No MMC drive support\n");
    X_FREE(mmc);

#endif

    if (mmc && !_mmc_check_aacs(mmc)) {
        DEBUG(DBG_MMC | DBG_CRIT, "AACS not active or supported by the drive\n");
#ifndef _WIN32
        mmc_close (mmc);
        return NULL;
#endif
    }

    if (mmc && mmc->read_drive_cert) {
        mmc_read_drive_cert(mmc, mmc->drive_cert);
    }

    return mmc;
}

void mmc_close(MMC *mmc)
{
    if (mmc) {

#if defined(HAVE_LINUX_CDROM_H)
        if (mmc->fd >= 0) {
            close(mmc->fd);
        }

#elif defined(_WIN32)
        if (mmc->fd != INVALID_HANDLE_VALUE) {
            CloseHandle(mmc->fd);
        }
#elif defined(USE_IOKIT)
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
#endif

        DEBUG(DBG_MMC, "Closed MMC drive\n");

        /* erase sensitive data */
        memset(mmc, 0, sizeof(*mmc));

        X_FREE(mmc);
    }
}

/*
 *
 */

static int _verify_signature(const uint8_t *cert, const uint8_t *signature,
                             const uint8_t *nonce, const uint8_t *point)
{
    uint8_t data[60];

    memcpy(data,      nonce, 20);
    memcpy(data + 20, point, 40);

    return crypto_aacs_verify(cert, signature, data, 60);
}

static int _mmc_aacs_auth(MMC *mmc, uint8_t agid, const uint8_t *host_priv_key, const uint8_t *host_cert, uint8_t *bus_key)
{
    uint8_t hks[40], dn[20], dkp[40], dks[40];
    char str[512];

    memset(hks, 0, sizeof(hks));

    if (DEBUG_KEYS) {
        DEBUG(DBG_MMC, "Host certificate   : %s\n", print_hex(str, host_cert,       92));
        DEBUG(DBG_MMC, "Host nonce         : %s\n", print_hex(str, mmc->host_nonce, 20));
    }

    // send host cert + nonce
    if (!_mmc_send_host_cert(mmc, agid, mmc->host_nonce, host_cert)) {

        if ((mmc->drive_cert[1] & 0x01) && !(host_cert[1] & 0x01)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Certificate (id 0x%s) can not be used with bus encryption capable drive\n",
                  print_hex(str, host_cert + 4, 6));
        } else {
            DEBUG(DBG_MMC | DBG_CRIT, "Host key / Certificate has been revoked by your drive ?\n");
        }
        return MMC_ERROR_CERT_REVOKED;
    }

    // receive mmc cert + nonce
    if (!_mmc_read_drive_cert_challenge(mmc, agid, dn, mmc->drive_cert)) {
        DEBUG(DBG_MMC | DBG_CRIT,
              "Drive doesn't give its certificate\n");
        return MMC_ERROR;
    }

    if (DEBUG_KEYS) {
        DEBUG(DBG_MMC, "Drive certificate   : %s\n", print_hex(str, mmc->drive_cert, 92));
        DEBUG(DBG_MMC, "Drive nonce         : %s\n", print_hex(str, dn, 20));
    }

    // verify drive certificate
    if (!crypto_aacs_verify_drive_cert(mmc->drive_cert)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Drive certificate is invalid\n");
        return MMC_ERROR;
    }

    // receive mmc key
    if (!_mmc_read_drive_key(mmc, agid, dkp, dks)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Drive doesn't give its drive key\n");
        return MMC_ERROR;
    }

    if (DEBUG_KEYS) {
        DEBUG(DBG_MMC, "Drive key point     : %s\n", print_hex(str, dkp, 40));
        DEBUG(DBG_MMC, "Drive key signature : %s\n", print_hex(str, dks, 40));
    }

    // verify drive signature
    if (!_verify_signature(mmc->drive_cert, dks, mmc->host_nonce, dkp)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Drive signature is invalid\n");
        return MMC_ERROR;
    }

    // sign
    crypto_aacs_sign(host_cert, host_priv_key, hks, dn,
                     mmc->host_key_point);

    // verify own signature
    if (!_verify_signature(host_cert, hks, dn, mmc->host_key_point)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Created signature is invalid ?\n");
        return MMC_ERROR;
    }

    // send signed host key and point
    if (!_mmc_send_host_key(mmc, agid, mmc->host_key_point, hks)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Error sending host signature\n");
        DEBUG(DBG_MMC,  "Host key signature : %s\n", print_hex(str, hks, 40));
        return MMC_ERROR;
    }

    // calculate bus key
    if (bus_key) {
        crypto_create_bus_key(mmc->host_key, dkp, bus_key);
        if (DEBUG_KEYS) {
            DEBUG(DBG_MMC, "Bus Key             : %s\n", print_hex(str, bus_key, 16));
        }
    }

    return MMC_SUCCESS;
}

/*
 *
 */

static int _read_vid(MMC *mmc, uint8_t agid, const uint8_t *bus_key, uint8_t *vid)
{
    uint8_t mac[16], calc_mac[16];
    char str[512];

    DEBUG(DBG_MMC, "Reading VID from drive...\n");

    if (_mmc_read_vid(mmc, agid, vid, mac)) {
        if (DEBUG_KEYS) {
            DEBUG(DBG_MMC, "VID                 : %s\n", print_hex(str, vid, 16));
            DEBUG(DBG_MMC, "VID MAC             : %s\n", print_hex(str, mac, 16));
        }

        /* verify MAC */
        crypto_aes_cmac_16(vid, bus_key, calc_mac);
        if (memcmp(calc_mac, mac, 16)) {
            DEBUG(DBG_MMC | DBG_CRIT, "VID MAC is incorrect. This means this Volume ID is not correct.\n");
        }

        return MMC_SUCCESS;
    }

    DEBUG(DBG_MMC | DBG_CRIT, "Unable to read VID from drive!\n");

    return MMC_ERROR;
}

static int _read_pmsn(MMC *mmc, uint8_t agid, const uint8_t *bus_key, uint8_t *pmsn)
{
    uint8_t mac[16], calc_mac[16];
    char str[512];

    DEBUG(DBG_MMC, "Reading PMSN from drive...\n");

    if (_mmc_read_pmsn(mmc, agid, pmsn, mac)) {
        if (DEBUG_KEYS) {
            DEBUG(DBG_MMC, "PMSN                : %s\n", print_hex(str, pmsn, 16));
            DEBUG(DBG_MMC, "PMSN MAC            : %s\n", print_hex(str, mac, 16));
        }

        /* verify MAC */
        crypto_aes_cmac_16(pmsn, bus_key, calc_mac);
        if (memcmp(calc_mac, mac, 16)) {
            DEBUG(DBG_MMC | DBG_CRIT, "PMSN MAC is incorrect. This means this Pre-recorded Medial Serial Number is not correct.\n");
        }

        return MMC_SUCCESS;
    }

    DEBUG(DBG_MMC, "Unable to read PMSN from drive!\n");

    return MMC_ERROR;
}

static int _read_data_keys(MMC *mmc, uint8_t agid, const uint8_t *bus_key,
                           uint8_t *read_data_key, uint8_t *write_data_key)
{
    uint8_t encrypted_read_data_key[16], encrypted_write_data_key[16];
    char str[512];

    DEBUG(DBG_MMC, "Reading data keys from drive...\n");

    if (_mmc_read_data_keys(mmc, agid, encrypted_read_data_key, encrypted_write_data_key)) {
        if (read_data_key) {
            crypto_aes128d(bus_key, encrypted_read_data_key, read_data_key);
            if (DEBUG_KEYS) {
                DEBUG(DBG_MMC, "READ DATA KEY       : %s\n", print_hex(str, read_data_key, 16));
            }
        }
        if (write_data_key) {
            crypto_aes128d(bus_key, encrypted_write_data_key, write_data_key);
            if (DEBUG_KEYS) {
                DEBUG(DBG_MMC, "WRITE DATA KEY      : %s\n", print_hex(str, write_data_key, 16));
            }
        }

        return MMC_SUCCESS;
    }

    DEBUG(DBG_MMC | DBG_CRIT, "Unable to read data keys from drive!\n");

    return MMC_ERROR;
}

int mmc_read_auth(MMC *mmc, const uint8_t *host_priv_key, const uint8_t *host_cert,
                  int request, uint8_t *p1, uint8_t *p2)
{
    uint8_t agid = 0, bus_key[16];
    int error_code;

    _mmc_invalidate_agids(mmc);

    if (!_mmc_report_agid(mmc, &agid)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Didn't get AGID from drive\n");
        return MMC_ERROR;
    }
    DEBUG(DBG_MMC, "Got AGID from drive: %d\n", agid);

    /*
     * NOTE: It seems that at least some drives require a new AACS-Auth every time
     * keys are being read
     */

    error_code = _mmc_aacs_auth(mmc, agid, host_priv_key, host_cert, bus_key);
    if (error_code) {
        _mmc_invalidate_agid(mmc, agid);
        return error_code;
    }

    switch (request) {
        case MMC_READ_VID:
            error_code = _read_vid(mmc, agid, bus_key, p1);
            break;
        case MMC_READ_PMSN:
            error_code = _read_pmsn(mmc, agid, bus_key, p1);
            break;
        case MMC_READ_DATA_KEYS:
            error_code = _read_data_keys(mmc, agid, bus_key, p1, p2);
            break;
        default:
            DEBUG(DBG_MMC | DBG_CRIT, "unknown mmc_read_auth() request %d\n", request);
            error_code = MMC_ERROR;
            break;
    }

    _mmc_invalidate_agid(mmc, agid);

    return error_code;
}

/*
 *
 */

int mmc_read_drive_cert(MMC *mmc, uint8_t *drive_cert)
{
    uint8_t buf[116];

    if (mmc->drive_cert[0] == 0x01) {
        memcpy(drive_cert, mmc->drive_cert, 92);
        return MMC_SUCCESS;
    }

    if (!mmc->read_drive_cert) {
        DEBUG(DBG_MMC | DBG_CRIT, "Drive does not support reading drive certificate\n");
    }

    if (!_mmc_report_key(mmc, 0, 0, 0, 0x38, buf, 116)) {
        if (mmc->read_drive_cert) {
            DEBUG(DBG_MMC | DBG_CRIT, "Failed reading drive certificate\n");
        }
        return MMC_ERROR;
    }

    if (!crypto_aacs_verify_drive_cert(buf + 4)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Drive certificate is invalid\n");
        return MMC_ERROR;
    }

    memcpy(mmc->drive_cert, buf + 4, 92);
    memcpy(drive_cert, buf + 4, 92);

    return MMC_SUCCESS;
}

const uint8_t *mmc_get_drive_cert(MMC *mmc)
{
    if (mmc->drive_cert[0] == 0x01) {
        return mmc->drive_cert;
    }
    return NULL;
}

uint8_t *mmc_read_mkb(MMC *mmc, int address, int *size)
{
    uint8_t agid = 0;

    _mmc_report_agid(mmc, &agid);

    uint8_t *mkb = _mmc_read_mkb(mmc, agid, address, size);

    _mmc_invalidate_agid(mmc, agid);

    return mkb;
}
