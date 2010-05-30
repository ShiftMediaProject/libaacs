#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmc.h"
#include "crypto.h"
#include "util/macro.h"
#include "util/logging.h"

#include <stdlib.h>
#include <mntent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#if HAVE_LINUX_CDROM_H
#include <linux/cdrom.h>
#endif

static int _mmc_send_cmd(MMC *mmc, const uint8_t *cmd, uint8_t *buf, size_t tx,
                         size_t rx)
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

        DEBUG(DBG_MMC, "Send LINUX MMC cmd %s: (%p)\n",
              print_hex((uint8_t *)cmd, 16), mmc);
        if (tx) {
            DEBUG(DBG_MMC, "  Buffer: %s -> (%p)\n", print_hex(buf, tx), mmc);
        } else {
            DEBUG(DBG_MMC, "  Buffer: %s <- (%p)\n", print_hex(buf, rx), mmc);
        }

        mmc->sk = sense.sense_key & 0x0f;
        mmc->asc = sense.asc;
        mmc->ascq = sense.ascq;

        if (a >= 0) {
            DEBUG(DBG_MMC, "  Send succeeded! [%d] (%p)\n", a, mmc);
            return 1;
        }

        DEBUG(DBG_MMC, "  Send failed! [%d] %s (%p)\n", a, strerror(errno),
              mmc);
    }
#endif

    return 0;
}

static int _mmc_report_key(MMC *mmc, uint8_t agid, uint32_t addr,
                           uint8_t blocks, uint8_t format, uint8_t *buf,
                           uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, 16);
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
    memset(cmd, 0, 16);

    DEBUG(DBG_MMC, "MMC send key [%d] %s... (%p)\n", len, print_hex(buf, len),
          mmc);

    cmd[0] = 0xa3;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = (agid << 6) | (format & 0x3f);

    DEBUG(DBG_MMC, "cmd: %s (%p)\n", print_hex(cmd, 16), mmc);
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
    memset(buf, 0, 84);

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
    memset(cmd, 0, 16);
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
              const uint8_t *host_cert, const uint8_t *host_nonce,
              const uint8_t *host_key_point)
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

    DEBUG(DBG_MMC, "Opening LINUX MMC drive %s... (%p)\n", file_path, mmc);

    if ((proc_mounts = setmntent("/proc/mounts", "r"))) {
        struct mntent* mount_entry;

        while ((mount_entry = getmntent(proc_mounts)) != NULL) {
            if (strcmp(mount_entry->mnt_dir, file_path) == 0) {
                int a = open(mount_entry->mnt_fsname, O_RDONLY | O_NONBLOCK);
                if (a >= 0) {
                    mmc->fd = a;

                    DEBUG(DBG_MMC, "LINUX MMC drive opened - fd: %d (%p)\n", a,
                          mmc);
                    break;
                }

                DEBUG(DBG_MMC, "Failed opening LINUX MMC drive %s (%p)\n",
                      file_path, mmc);
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

    DEBUG(DBG_MMC, "Closed MMC drive (%p)\n", mmc);

    X_FREE(mmc);
}

int mmc_read_vid(MMC *mmc, uint8_t *vid)
{
    uint8_t agid = 0, hks[40], dn[20], dc[92], dkp[40], dks[40], mac[16];

    memset(hks, 0, 40);

    DEBUG(DBG_MMC, "Reading VID from drive... (%p)\n", mmc);

    _mmc_invalidate_agids(mmc);

    if (!_mmc_report_agid(mmc, &agid)) {
        DEBUG(DBG_MMC | DBG_CRIT, "Didn't get AGID from drive (%p)\n", mmc);
        return 0;
    }
    DEBUG(DBG_MMC, "Got AGID from drive: %d (%p)\n", agid, mmc);

    int patched = 0;
    if (!patched) do {

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
        //DEBUG(DBG_MMC, "Drive certificate   : %s (%p)\n", print_hex(dc, 92),
        //      mmc);
        //DEBUG(DBG_MMC, "Drive nonce         : %s (%p)\n", print_hex(dn, 20),
        //      mmc);

        // receive mmc key
        if (!_mmc_read_drive_key(mmc, agid, dkp, dks)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Drive doesn't give its drive key (%p)\n",
                  mmc);
            break;
        }
        //DEBUG(DBG_MMC, "Drive key point     : %s (%p)\n", print_hex(dkp, 40),
        //      mmc);
        //DEBUG(DBG_MMC, "Drive key signature : %s (%p)\n", print_hex(dks, 40),
        //      mmc);

        crypto_aacs_sign(mmc->host_cert, mmc->host_priv_key, hks, dn,
                         mmc->host_key_point);

        // send signed host key and point
        if (!_mmc_send_host_key(mmc, agid, mmc->host_key_point, hks)) {
            DEBUG(DBG_MMC | DBG_CRIT, "Error sending host signature (%p)\n",
                  mmc);
            DEBUG(DBG_MMC,  "Host key signature : %s (%p)\n",
                  print_hex(hks, 40), mmc);
            break;
        }

    } while (0);

    if (_mmc_read_vid(mmc, agid, vid, mac)) {
        DEBUG(DBG_MMC, "VID: %s (%p)\n", print_hex(vid, 16), mmc);
        DEBUG(DBG_MMC, "MAC: %s (%p)\n", print_hex(mac, 16), mmc);

        _mmc_invalidate_agid(mmc, agid);

        return 1;
    }

    DEBUG(DBG_MMC | DBG_CRIT, "Unable to read VID from drive! (%p)\n", mmc);

    _mmc_invalidate_agid(mmc, agid);

    return 0;
}
