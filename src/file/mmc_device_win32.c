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

#include "util/macro.h"
#include "util/logging.h"

#include <windows.h>
#include <winsock.h>

#include <stdlib.h>
#include <string.h>


/*
 * from ntddscsi.h, Windows DDK
 */

#define SCSI_IOCTL_DATA_OUT             0
#define SCSI_IOCTL_DATA_IN              1
#define SCSI_IOCTL_DATA_UNSPECIFIED     2
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  0x4D014
#define MAX_SENSE_LEN                   18

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


/*
 *
 */

struct mmcdev {
    HANDLE fd;
};

int device_send_cmd(MMCDEV *dev, const uint8_t *cmd, uint8_t *buf, size_t tx, size_t rx)
{
    DWORD dwBytesReturned;

    struct {
        SCSI_PASS_THROUGH_DIRECT sptd;
        UCHAR                    SenseBuf[MAX_SENSE_LEN];
    } sptd_sb;

    if (dev->fd == INVALID_HANDLE_VALUE) {
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

    if (DeviceIoControl(dev->fd,
                        IOCTL_SCSI_PASS_THROUGH_DIRECT,
                        (void*)&sptd_sb, sizeof(sptd_sb),
                        (void*)&sptd_sb, sizeof(sptd_sb),
                        &dwBytesReturned, NULL)) {

        if (sptd_sb.sptd.ScsiStatus == 0 /* STATUS_GOOD */) {
            BD_DEBUG(DBG_MMC, "  Send succeeded!\n");
            return 1;
        }
    }

    BD_DEBUG(DBG_MMC, "  Send failed!\n");

    return 0;
}

MMCDEV *device_open(const char *path)
{
    char    drive[] = { path[0], ':', '\\', 0 };
    char    volume[] = {'\\', '\\', '.', '\\', path[0], ':', 0};
    UINT    type;
    HANDLE  fd;
    MMCDEV *dev;

    BD_DEBUG(DBG_MMC, "Opening Windows MMC drive %s...\n", drive);

    type = GetDriveType(drive);
    if (type != DRIVE_CDROM) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Drive %s is not CD/DVD drive\n", drive);
        return NULL;
    }

    fd = CreateFile(volume, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd == INVALID_HANDLE_VALUE) {
        fd = CreateFile(volume, GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fd == INVALID_HANDLE_VALUE) {
            BD_DEBUG(DBG_MMC, "Failed opening Windows MMC drive %s\n", volume);
            return NULL;
        }
    }

    BD_DEBUG(DBG_MMC, "Windows MMC drive %s opened\n", volume);

    dev = calloc(1, sizeof(MMCDEV));
    if (!dev) {
        BD_DEBUG(DBG_MKB | DBG_CRIT, "out of memory\n");
        CloseHandle(fd);
        return NULL;
    }

    dev->fd = fd;
    return dev;
}

void device_close(MMCDEV **pp)
{
    if (pp && *pp) {

        if ((*pp)->fd != INVALID_HANDLE_VALUE) {
            CloseHandle((*pp)->fd);
        }

        X_FREE(*pp);
    }
}
