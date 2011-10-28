/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010       npzacs
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
#include "util/macro.h"
#include "util/logging.h"

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

#ifdef HAVE_LINUX_CDROM_H
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <winsock.h>
#endif

/* define in CFLAGS to skip drive certificate checks */
#ifndef PATCHED_DRIVE
#define PATCHED_DRIVE 0
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
#if defined(_WIN32)
    HANDLE fd;
#else
    int    fd;
#endif
    uint8_t host_priv_key[20], host_cert[92];
    uint8_t host_nonce[20];
    uint8_t host_key[20];
    uint8_t host_key_point[40];
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
        DEBUG(DBG_MMC, "Send LINUX MMC cmd %s: (%p)\n",
              print_hex(str, cmd, 16), mmc);
        if (tx) {
            DEBUG(DBG_MMC, "  Buffer: %s -> (%p)\n", print_hex(str, buf, tx), mmc);
        } else {
            DEBUG(DBG_MMC, "  Buffer: %s <- (%p)\n", print_hex(str, buf, rx), mmc);
        }

        if (a >= 0) {
            DEBUG(DBG_MMC, "  Send succeeded! [%d] (%p)\n", a, mmc);
            return 1;
        }

        DEBUG(DBG_MMC, "  Send failed! [%d] %s (%p)\n", a, strerror(errno),
              mmc);
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
            DEBUG(DBG_MMC, "  Send succeeded! (%p)\n", mmc);
            return 1;
        }
    }

    DEBUG(DBG_MMC, "  Send failed! (%p)\n", mmc);

#endif

    return 0;
}

static int _mmc_report_key(MMC *mmc, uint8_t agid, uint32_t addr,
                           uint8_t blocks, uint8_t format, uint8_t *buf,
                           uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, len);

    DEBUG(DBG_MMC, "MMC report key... (%p)\n", mmc);

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

    DEBUG(DBG_MMC, "MMC send key [%d] %s... (%p)\n", len, print_hex(str, buf, len),
          mmc);

    cmd[0] = 0xa3;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = (agid << 6) | (format & 0x3f);

    DEBUG(DBG_MMC, "cmd: %s (%p)\n", print_hex(str, cmd, 16), mmc);
    return _mmc_send_cmd(mmc, cmd, buf, len, 0);
}

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

static int _mmc_read_drive_cert(MMC *mmc, uint8_t agid, uint8_t *drive_nonce,
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
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, 36);

    cmd[0] = 0xad;
    cmd[1] = 1;    // 1 = BLURAY
    cmd[7] = 0x80;
    cmd[9] = 0x24;
    cmd[10] = (agid << 6) & 0xc0;

    if (_mmc_send_cmd(mmc, cmd, buf, 0, 36)) {
        memcpy(volume_id, buf + 4,  16);
        memcpy(mac,       buf + 20, 16);
        return 1;
    }

    return 0;
}

MMC *mmc_open(const char *path, const uint8_t *host_priv_key,
              const uint8_t *host_cert)
{
    MMC *mmc = calloc(1, sizeof(MMC));

    if (host_priv_key) memcpy(mmc->host_priv_key, host_priv_key, 20);
    if (host_cert) memcpy(mmc->host_cert, host_cert, 92);

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

#if defined(HAVE_MNTENT_H)

#ifdef HAVE_REALPATH
    char *file_path = realpath(path, NULL);
#else
    char *file_path = (char*)malloc(strlen(path) + 1);
    strcpy(file_path, path);
#endif

    int   path_len  = strlen(file_path);
    FILE *proc_mounts;

    mmc->fd = -1;

    // strip trailing '/'s
    while (path_len > 0 && file_path[--path_len] == '/') {
        file_path[path_len] = '\0';
    }

    DEBUG(DBG_MMC, "Opening LINUX MMC drive %s... (%p)\n", file_path, mmc);

    if ((proc_mounts = setmntent("/proc/mounts", "r"))) {
        struct mntent* mount_entry;

        while ((mount_entry = getmntent(proc_mounts)) != NULL) {
            if (strcmp(mount_entry->mnt_dir, file_path) == 0) {
                mmc->fd = open(mount_entry->mnt_fsname, O_RDONLY | O_NONBLOCK);
                if (mmc->fd >= 0) {
                    DEBUG(DBG_MMC, "LINUX MMC drive %s opened - fd: %d (%p)\n",
                          mount_entry->mnt_fsname, mmc->fd, mmc);
                    break;
                }

                DEBUG(DBG_MMC, "Failed opening LINUX MMC drive %s mounted to %s (%p)\n",
                      mount_entry->mnt_fsname, file_path, mmc);
            }
        }

        endmntent(proc_mounts);
    } else {
        DEBUG(DBG_MMC, "Error opening /proc/mounts (%p)\n", mmc);
    }

    if (mmc->fd < 0) {
        DEBUG(DBG_MMC, "Error opening LINUX MMC drive mounted to %s (%p)\n", file_path, mmc);
        X_FREE(mmc);
    }

    X_FREE(file_path);

#elif defined(_WIN32)
    char drive[] = { path[0], ':', '\\', 0 };
    char volume[] = {'\\', '\\', '.', '\\', path[0], ':', 0};

    DEBUG(DBG_MMC, "Opening Windows MMC drive %s... (%p)\n", drive, mmc);

    UINT type = GetDriveType(drive);

    if (type != DRIVE_CDROM) {
        DEBUG(DBG_MMC, "Drive %s is not CD/DVD drive !\n", drive);
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
            DEBUG(DBG_MMC, "Failed opening Windows MMC drive %s (%p)\n", volume, mmc);
            X_FREE(mmc);
            return NULL;
        }
    }

    DEBUG(DBG_MMC, "Windows MMC drive %s opened (%p)\n", volume, mmc);

#else

    DEBUG(DBG_MMC, "No MMC drive support !\n");
    X_FREE(mmc);

#endif

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
#endif

        DEBUG(DBG_MMC, "Closed MMC drive (%p)\n", mmc);

        X_FREE(mmc);
    }
}

static int _verify_signature(const uint8_t *cert, const uint8_t *signature,
                             const uint8_t *nonce, const uint8_t *point)
{
    uint8_t data[60];

    memcpy(data,      nonce, 20);
    memcpy(data + 20, point, 40);

    return crypto_aacs_verify(cert, signature, data, 60);
}

int mmc_read_vid(MMC *mmc, uint8_t *vid)
{
    uint8_t agid = 0, hks[40], dn[20], dc[92], dkp[40], dks[40], mac[16];
    char str[512];

    memset(hks, 0, sizeof(hks));

    DEBUG(DBG_MMC, "Reading VID from drive... (%p)\n", mmc);

    _mmc_invalidate_agids(mmc);

    if (!_mmc_report_agid(mmc, &agid)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Didn't get AGID from drive (%p)\n", mmc);
        return 0;
    }
    DEBUG(DBG_MMC, "Got AGID from drive: %d (%p)\n", agid, mmc);

    if (!PATCHED_DRIVE) do {

        if (DEBUG_KEYS) {
            DEBUG(DBG_MMC, "Host certificate   : %s (%p)\n", print_hex(str, mmc->host_cert, 92), mmc);
            DEBUG(DBG_MMC, "Host nonce         : %s (%p)\n", print_hex(str, mmc->host_nonce, 20), mmc);
        }

        // send host cert + nonce
        if (!_mmc_send_host_cert(mmc, agid, mmc->host_nonce, mmc->host_cert)) {
            DEBUG(DBG_MMC | DBG_CRIT,
                  "Host key / Certificate has been revoked by your drive ? "
                  "(%p)\n", mmc);
            break;
        }

        // receive mmc cert + nonce
        if (!_mmc_read_drive_cert(mmc, agid, dn, dc)) {
            DEBUG(DBG_MMC | DBG_CRIT,
                  "Drive doesn't give its certificate (%p)\n", mmc);
            break;
        }

        if (DEBUG_KEYS) {
            DEBUG(DBG_MMC, "Drive certificate   : %s (%p)\n", print_hex(str, dc, 92), mmc);
            DEBUG(DBG_MMC, "Drive nonce         : %s (%p)\n", print_hex(str, dn, 20), mmc);
        }

        // verify drive certificate
        if (!crypto_aacs_verify_drive_cert(dc)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Drive certificate is invalid (%p)\n", mmc);
            break;
        }

        // receive mmc key
        if (!_mmc_read_drive_key(mmc, agid, dkp, dks)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Drive doesn't give its drive key (%p)\n",
                  mmc);
            break;
        }

        if (DEBUG_KEYS) {
            DEBUG(DBG_MMC, "Drive key point     : %s (%p)\n", print_hex(str, dkp, 40), mmc);
            DEBUG(DBG_MMC, "Drive key signature : %s (%p)\n", print_hex(str, dks, 40), mmc);
        }

        // verify drive signature
        if (!_verify_signature(dc, dks, mmc->host_nonce, dkp)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Drive signature is invalid\n");
            break;
        }

        // sign
        crypto_aacs_sign(mmc->host_cert, mmc->host_priv_key, hks, dn,
                         mmc->host_key_point);

        // verify own signature
        if (!_verify_signature(mmc->host_cert, hks, dn, mmc->host_key_point)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Created signature is invalid ?\n");
            break;
        }

        // send signed host key and point
        if (!_mmc_send_host_key(mmc, agid, mmc->host_key_point, hks)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Error sending host signature (%p)\n",
                  mmc);
            DEBUG(DBG_MMC,  "Host key signature : %s (%p)\n",
                  print_hex(str, hks, 40), mmc);
            break;
        }

    } while (0);

    if (_mmc_read_vid(mmc, agid, vid, mac)) {
        DEBUG(DBG_MMC, "VID: %s (%p)\n", print_hex(str, vid, 16), mmc);
        DEBUG(DBG_MMC, "MAC: %s (%p)\n", print_hex(str, mac, 16), mmc);

        _mmc_invalidate_agid(mmc, agid);

        return 1;
    }

    DEBUG(DBG_MMC | DBG_CRIT, "Unable to read VID from drive! (%p)\n", mmc);

    _mmc_invalidate_agid(mmc, agid);

    return 0;
}
