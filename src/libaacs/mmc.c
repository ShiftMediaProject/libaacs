#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_MNTENT_H
#include <mntent.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <errno.h>  /* errno */
#include <string.h> /* strerror() */

#if HAVE_LINUX_CDROM_H
#include <linux/cdrom.h>
#endif

#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "mmc.h"
#include "crypto.h"
#include "../file/file.h"
#include "../util/macro.h"
#include "../util/logging.h"


static int _mmc_send_cmd(MMC *mmc, const uint8_t *cmd, uint8_t *buf, size_t tx, size_t rx)
{
#if HAVE_LINUX_CDROM_H
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

        DEBUG(DBG_MMC, "Send LINUX MMC cmd %s: (0x%08x)\n", print_hex((uint8_t *)cmd, 16), mmc);
        if (tx) {
            DEBUG(DBG_MMC, "  Buffer: %s -> (0x%08x)\n", print_hex(buf, tx), mmc);
        } else {
            DEBUG(DBG_MMC, "  Buffer: %s <- (0x%08x)\n", print_hex(buf, rx), mmc);
        }

        mmc->sk = sense.sense_key & 0x0f;
        mmc->asc = sense.asc;
        mmc->ascq = sense.ascq;

        if (a >= 0) {
            DEBUG(DBG_MMC, "  Send succeeded! [%d] (0x%08x)\n", a, mmc);
            return 1;
        }

        DEBUG(DBG_MMC, "  Send failed! [%d] %s (0x%08x)\n", a, strerror(errno), mmc);
    }
#endif

    return 0;
}

static void _mmc_report_key(MMC *mmc, uint8_t agid, uint32_t addr, uint8_t blocks, uint8_t format, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, 16);
    memset(buf, 0, len);

    DEBUG(DBG_MMC, "MMC report key... (0x%08x)\n", mmc);

    cmd[0] = 0xa4;
    cmd[2] = (addr >> 24) & 0xff;
    cmd[3] = (addr >> 16) & 0xff;
    cmd[4] = (addr >> 8) & 0xff;
    cmd[5] = addr & 0xff;
    cmd[6] = blocks;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = agid << 6 | (format & 0x3f);

    _mmc_send_cmd(mmc, cmd, buf, 0, len);
}

static void _mmc_send_key(MMC *mmc, uint8_t agid, uint8_t format, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, 16);

    DEBUG(DBG_MMC, "MMC send key [%d] %s... (0x%08x)\n", len, print_hex(buf, len),mmc);

    cmd[0] = 0xa3;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = agid << 6 | (format & 0x3f);

    DEBUG(DBG_MMC, "cmd: %s\n", print_hex(cmd, 16), 16);
    _mmc_send_cmd(mmc, cmd, buf, len, 0);
}

MMC *mmc_open(const char *path, uint8_t *host_priv_key, uint8_t *host_cert, uint8_t *host_nonce, uint8_t *host_key_point)
{
#if HAVE_LINUX_CDROM_H
    char *file_path = strdup(path);
    int   path_len  = strlen(file_path);
    FILE *proc_mounts;
    MMC *mmc = malloc(sizeof(MMC));

    if (host_priv_key) memcpy(mmc->host_priv_key, host_priv_key, 20);
    if (host_cert) memcpy(mmc->host_cert, host_cert, 92);
    if (host_nonce) memcpy(mmc->host_nonce, host_nonce, 20);
    if (host_key_point) memcpy(mmc->host_key_point, host_key_point, 40);
    mmc->fd = -1;

    // strip trailing '/'s
    while (path_len > 0 && file_path[--path_len] == '/') {
        file_path[path_len] = '\0';
    }

    DEBUG(DBG_MMC, "Opening LINUX MMC drive %s... (0x%08x)\n", file_path, mmc);

    if ((proc_mounts = setmntent("/proc/mounts", "r"))) {
        struct mntent* mount_entry;

        while ((mount_entry = getmntent(proc_mounts)) != NULL) {
            if (strcmp(mount_entry->mnt_dir, file_path) == 0) {
                int a = open(mount_entry->mnt_fsname, O_RDONLY | O_NONBLOCK);
                if (a >= 0) {
                    mmc->fd = a;

                    DEBUG(DBG_MMC, "LINUX MMC drive opened - fd: %d (0x%08x)\n", a, mmc);
                    break;
                }

                DEBUG(DBG_MMC, "Failed opening LINUX MMC drive %s (0x%08x)\n", file_path, mmc);
            }
        }

        endmntent(proc_mounts);
    }

    X_FREE(file_path);

    if (mmc->fd < 0) {
        X_FREE(mmc);
    }

    return mmc;
#endif

    return NULL;
}

void mmc_close(MMC *mmc)
{
    if (mmc->fd >= 0)
        close(mmc->fd);

    DEBUG(DBG_MMC, "Closed MMC drive (0x%08x)\n", mmc);

    X_FREE(mmc);
}

int mmc_read_vid(MMC *mmc, uint8_t *vid)
{
    int a;
    uint8_t agid, buf[116], cmd[16], hks[40], dn[20], dc[92], dkp[40], dks[40];

    memset(cmd, 0, 16);
    memset(hks, 0, 40);
    memset(buf, 0, 116);

    DEBUG(DBG_MMC, "Reading VID from drive... (0x%08x)\n", mmc);

    for (a = 0; a < 4; a++) {
        _mmc_report_key(mmc, a, 0, 0, 0x3f, buf, 2);
    }

    _mmc_report_key(mmc, 0, 0, 0, 0, buf, 8);
    agid = (buf[7] & 0xff) >> 6;

    int patched = 0;
    if (!patched) {
        memset(buf, 0, 116);
        buf[1] = 0x72;
        memcpy(buf + 4, mmc->host_nonce, 20);
        memcpy(buf + 24, mmc->host_cert, 92);
        _mmc_send_key(mmc, agid, 0x01, buf, 116); // send host cert + nonce

        memset(buf, 0, 116);
        _mmc_report_key(mmc, agid, 0, 0, 0x01, buf, 116); // receive mmc cert + nonce
        memcpy(dn, buf + 4, 20);
        memcpy(dc, buf + 24, 92);

        memset(buf, 0, 84);
        _mmc_report_key(mmc, agid, 0, 0, 0x02, buf, 84); // receive mmc key
        memcpy(dkp, buf + 4, 40);
        memcpy(dks, buf + 44, 40);

        crypto_aacs_sign(mmc->host_cert, mmc->host_priv_key, hks, dn, mmc->host_key_point);

        memset(buf, 0, 84);
        buf[1] = 0x52;
        memcpy(buf + 4, mmc->host_key_point, 40);
        memcpy(buf + 44, hks, 40);
        _mmc_send_key(mmc, agid, 0x02, buf, 84); // send signed host key and point
    }

    memset(cmd, 0, 16);
    memset(buf, 0, 116);

    cmd[0] = 0xad;
    cmd[1] = 1;
    cmd[7] = 0x80;
    cmd[9] = 0x24;
    cmd[10] = (agid << 6) & 0xc0;

    if (_mmc_send_cmd(mmc, cmd, buf, 0, 36)) {
        memcpy(vid, buf + 4, 16);

        DEBUG(DBG_MMC, "VID: %s (0x%08x)\n", print_hex(vid, 16), mmc);

        return 1;
    }

    DEBUG(DBG_MMC, "Unable to read VID from drive! (0x%08x)\n", mmc);

    return 0;
}
