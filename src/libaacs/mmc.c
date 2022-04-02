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
#include "file/mmc_device.h"
#include "util/logging.h"
#include "util/macro.h"
#include "util/strutl.h"

#include <stdlib.h>
#include <string.h>

#ifndef DEBUG_KEYS
#define DEBUG_KEYS 0
#endif

struct mmc {
    MMCDEV *dev;

    uint8_t host_nonce[20];
    uint8_t host_key[20];
    uint8_t host_key_point[40];

    uint8_t drive_cert[92];

    uint8_t read_drive_cert;
    uint8_t aacs_version;
};

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

    BD_DEBUG(DBG_MMC, "MMC report key...\n");

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

    return device_send_cmd(mmc->dev, cmd, buf, 0, len);
}

static int _mmc_send_key(MMC *mmc, uint8_t agid, uint8_t format, uint8_t *buf,
                         uint16_t len)
{
    uint8_t cmd[16];
    char str[512];
    memset(cmd, 0, sizeof(cmd));

    BD_DEBUG(DBG_MMC, "MMC send key [%d] %s...\n", len, str_print_hex(str, buf, len));

    cmd[0] = 0xa3;
    cmd[7] = 0x02;
    cmd[8] = (len >> 8) & 0xff;
    cmd[9] = len & 0xff;
    cmd[10] = (agid << 6) | (format & 0x3f);

    BD_DEBUG(DBG_MMC, "cmd: %s\n", str_print_hex(str, cmd, 16));
    return device_send_cmd(mmc->dev, cmd, buf, len, 0);
}

static int _mmc_report_disc_structure(MMC *mmc, uint8_t agid, uint8_t format,
                                      uint8_t layer, uint32_t address,
                                      uint8_t *buf, uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, len);

    BD_DEBUG(DBG_MMC, "MMC report disc structure [format 0x%x layer %d address %d] ...\n", format, layer, address);

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

    return device_send_cmd(mmc->dev, cmd, buf, 0, len);
}

static int _mmc_get_configuration(MMC *mmc, uint16_t feature, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[16];
    memset(cmd, 0, sizeof(cmd));
    memset(buf, 0, len);

    BD_DEBUG(DBG_MMC, "MMC get configuration [feature 0x%x] ...\n", feature);

    cmd[0] = 0x46; // operation code
    cmd[1] = 0x01; // BluRay
    cmd[2] = (feature >> 8) & 0xff;
    cmd[3] = feature & 0xff;
    cmd[7] = (len >> 8) & 0xff;
    cmd[8] = len & 0xff;

    return device_send_cmd(mmc->dev, cmd, buf, 0, len);
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
            mmc->read_drive_cert = !!(buf[4+8] & 0x10);
            mmc->aacs_version = buf[7+8];
            BD_DEBUG(DBG_MMC, "AACS feature descriptor:\n");
            BD_DEBUG(DBG_MMC, "  AACS version: %d\n", mmc->aacs_version);
            BD_DEBUG(DBG_MMC, "  AACS active: %d\n", buf[2+8] & 1);
            BD_DEBUG(DBG_MMC, "  Binding Nonce generation support: %d\n", buf[4+8] & 1);
            BD_DEBUG(DBG_MMC, "  Binding Nonce block count: %d\n", buf[5+8]);
            BD_DEBUG(DBG_MMC, "  Bus encryption support: %d\n", !!(buf[4+8] & 2));
            BD_DEBUG(DBG_MMC, "  Read drive certificate: %d\n", mmc->read_drive_cert);
            BD_DEBUG(DBG_MMC, "  AGID count: %d\n", buf[6+8] & 0xf);

            return buf[2+8] & 1;
        }
        BD_DEBUG(DBG_MMC, "incorrect feature ID %04x\n", feature);
    }

    BD_DEBUG(DBG_MMC, "_mmc_get_configuration() failed\n");
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

        BD_DEBUG(DBG_MMC, "got mkb: pack 0/%d %d bytes\n", num_packs, len);

        if (len <= 0 || len > 32768) {
              BD_DEBUG(DBG_MMC | DBG_CRIT, "invalid pack\n");
              return NULL;
        }

        mkb = malloc(32768 * num_packs);
        if (!mkb) {
              BD_DEBUG(DBG_MMC | DBG_CRIT, "out of memory\n");
              return NULL;
        }

        memcpy(mkb, buf + 4, len);
       *size += len;

        for (pack = 1; pack < num_packs; pack++) {
            if (_mmc_report_disc_structure(mmc, agid, 0x83, layer, pack, buf, sizeof(buf))) {
                len = MKINT_BE16(buf) - 2;
                BD_DEBUG(DBG_MMC, "got mkb: pack %d/%d %d bytes\n", pack, num_packs, len);
                if (len <= 0 || len > 32768) {
                    BD_DEBUG(DBG_MMC | DBG_CRIT, "invalid pack\n");
                    break;
                }
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

/*
 *
 */

MMC *mmc_open(const char *path)
{
    MMC *mmc;

    if (!path) {
        return NULL;
    }

    mmc = calloc(1, sizeof(MMC));
    if (!mmc) {
        return NULL;
    }

    crypto_create_nonce(mmc->host_nonce, sizeof(mmc->host_nonce));

    if (DEBUG_KEYS) {
        char str[sizeof(mmc->host_nonce)*2 + 1];
        BD_DEBUG(DBG_MMC, "Created host nonce (Hn): %s\n",
              str_print_hex(str, mmc->host_nonce, sizeof(mmc->host_nonce)));
    }

    crypto_create_host_key_pair(mmc->host_key, mmc->host_key_point);

    if (DEBUG_KEYS) {
        char    str[sizeof(mmc->host_key_point)*2 + 1];
        BD_DEBUG(DBG_MMC, "Created host key (Hk): %s\n",
              str_print_hex(str, mmc->host_key, sizeof(mmc->host_key)));
        BD_DEBUG(DBG_MMC, "Created host key point (Hv): %s\n",
              str_print_hex(str, mmc->host_key_point, sizeof(mmc->host_key_point)));
    }

    mmc->dev = device_open(path);
    if (!mmc->dev) {
        mmc_close (mmc);
        return NULL;
    }

    if (!_mmc_check_aacs(mmc)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "AACS not active or supported by the drive\n");
#ifndef _WIN32
        mmc_close (mmc);
        return NULL;
#endif
    }

    if (mmc->aacs_version > 1) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "WARNING: unsupported AACS2 drive detected.\n");
#if 0
        mmc_close (mmc);
        return NULL;
#endif
    }

    if (mmc->read_drive_cert) {
        mmc_read_drive_cert(mmc, mmc->drive_cert);
    }

    return mmc;
}

void mmc_close(MMC *mmc)
{
    if (mmc) {

        device_close(&mmc->dev);
        BD_DEBUG(DBG_MMC, "Closed MMC drive\n");

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
    int crypto_error;

    memcpy(data,      nonce, 20);
    memcpy(data + 20, point, 40);

    crypto_error = crypto_aacs_verify(cert, signature, data, 60);
    if (crypto_error) {
        LOG_CRYPTO_ERROR(DBG_MMC, "signature verification failed", crypto_error);
    }

    return (crypto_error == 0);
}

static int _mmc_aacs_auth(MMC *mmc, uint8_t agid, const uint8_t *host_priv_key, const uint8_t *host_cert, uint8_t *bus_key)
{
    uint8_t hks[40], dn[20], dkp[40], dks[40];
    char str[512];
    int crypto_error;

    memset(hks, 0, sizeof(hks));

    if (DEBUG_KEYS) {
        BD_DEBUG(DBG_MMC, "Host certificate   : %s\n", str_print_hex(str, host_cert,       92));
        BD_DEBUG(DBG_MMC, "Host nonce         : %s\n", str_print_hex(str, mmc->host_nonce, 20));
    }

    // send host cert + nonce
    if (!_mmc_send_host_cert(mmc, agid, mmc->host_nonce, host_cert)) {

        str_print_hex(str, host_cert + 4, 6);
        if ((mmc->drive_cert[1] & 0x01) && !(host_cert[1] & 0x01)) {
            BD_DEBUG(DBG_MMC | DBG_CRIT, "Certificate (id 0x%s) can not be used with bus encryption capable drive\n",
                     str);
        } else {
            BD_DEBUG(DBG_MMC | DBG_CRIT, "Host key / Certificate (id 0x%s) has been revoked by your drive ?\n",
                     str);
        }
        return MMC_ERROR_CERT_REVOKED;
    }

    // receive mmc cert + nonce
    if (!_mmc_read_drive_cert_challenge(mmc, agid, dn, mmc->drive_cert)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT,
              "Drive doesn't give its certificate\n");
        return MMC_ERROR;
    }

    if (DEBUG_KEYS) {
        BD_DEBUG(DBG_MMC, "Drive certificate   : %s\n", str_print_hex(str, mmc->drive_cert, 92));
        BD_DEBUG(DBG_MMC, "Drive nonce         : %s\n", str_print_hex(str, dn, 20));
    }

    if (mmc->drive_cert[0] == 0x11) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "WARNING: Drive is using AACS 2.0 certificate\n");
        return MMC_ERROR;
    }

    // verify drive certificate
    crypto_error = crypto_aacs_verify_drive_cert(mmc->drive_cert);
    if (crypto_error) {
        LOG_CRYPTO_ERROR(DBG_MMC, "drive certificate signature verification failed", crypto_error);
        return MMC_ERROR;
    }

    // receive mmc key
    if (!_mmc_read_drive_key(mmc, agid, dkp, dks)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Drive doesn't give its drive key\n");
        return MMC_ERROR;
    }

    if (DEBUG_KEYS) {
        BD_DEBUG(DBG_MMC, "Drive key point     : %s\n", str_print_hex(str, dkp, 40));
        BD_DEBUG(DBG_MMC, "Drive key signature : %s\n", str_print_hex(str, dks, 40));
    }

    // verify drive signature
    if (!_verify_signature(mmc->drive_cert, dks, mmc->host_nonce, dkp)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Drive signature is invalid\n");
        return MMC_ERROR;
    }

    // sign
    crypto_error = crypto_aacs_sign(host_cert, host_priv_key, hks, dn, mmc->host_key_point);
    if (crypto_error) {
        LOG_CRYPTO_ERROR(DBG_MMC, "Signing failed", crypto_error);
        return MMC_ERROR;
    }

    // verify own signature
    if (!_verify_signature(host_cert, hks, dn, mmc->host_key_point)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Created signature is invalid ?\n");
        return MMC_ERROR;
    }

    // send signed host key and point
    if (!_mmc_send_host_key(mmc, agid, mmc->host_key_point, hks)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Error sending host signature\n");
        BD_DEBUG(DBG_MMC,  "Host key signature : %s\n", str_print_hex(str, hks, 40));
        return MMC_ERROR;
    }

    // calculate bus key
    if (bus_key) {
        crypto_create_bus_key(mmc->host_key, dkp, bus_key);
        if (DEBUG_KEYS) {
            BD_DEBUG(DBG_MMC, "Bus Key             : %s\n", str_print_hex(str, bus_key, 16));
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
    int err;

    BD_DEBUG(DBG_MMC, "Reading VID from drive...\n");

    if (_mmc_read_vid(mmc, agid, vid, mac)) {
        if (DEBUG_KEYS) {
            BD_DEBUG(DBG_MMC, "VID                 : %s\n", str_print_hex(str, vid, 16));
            BD_DEBUG(DBG_MMC, "VID MAC             : %s\n", str_print_hex(str, mac, 16));
        }

        /* verify MAC */
        err = crypto_aes_cmac_16(vid, bus_key, calc_mac);
        if (err) {
            LOG_CRYPTO_ERROR(DBG_MMC, "VID MAC calculation failed", err);
            return MMC_ERROR;
        }
        if (memcmp(calc_mac, mac, 16)) {
            BD_DEBUG(DBG_MMC | DBG_CRIT, "VID MAC is incorrect. This means this Volume ID is not correct.\n");
            return MMC_ERROR;
        }

        return MMC_SUCCESS;
    }

    BD_DEBUG(DBG_MMC | DBG_CRIT, "Unable to read VID from drive!\n");

    return MMC_ERROR;
}

static int _read_pmsn(MMC *mmc, uint8_t agid, const uint8_t *bus_key, uint8_t *pmsn)
{
    uint8_t mac[16], calc_mac[16];
    char str[512];
    int err;

    BD_DEBUG(DBG_MMC, "Reading PMSN from drive...\n");

    if (_mmc_read_pmsn(mmc, agid, pmsn, mac)) {
        if (DEBUG_KEYS) {
            BD_DEBUG(DBG_MMC, "PMSN                : %s\n", str_print_hex(str, pmsn, 16));
            BD_DEBUG(DBG_MMC, "PMSN MAC            : %s\n", str_print_hex(str, mac, 16));
        }

        /* verify MAC */
        err = crypto_aes_cmac_16(pmsn, bus_key, calc_mac);
        if (err) {
            LOG_CRYPTO_ERROR(DBG_MMC, "PMSN MAC calculation failed", err);
            return MMC_ERROR;
        }
        if (memcmp(calc_mac, mac, 16)) {
            BD_DEBUG(DBG_MMC | DBG_CRIT, "PMSN MAC is incorrect. This means this Pre-recorded Medial Serial Number is not correct.\n");
            return MMC_ERROR;
        }

        return MMC_SUCCESS;
    }

    BD_DEBUG(DBG_MMC, "Unable to read PMSN from drive!\n");

    return MMC_ERROR;
}

static int _read_data_keys(MMC *mmc, uint8_t agid, const uint8_t *bus_key,
                           uint8_t *read_data_key, uint8_t *write_data_key)
{
    uint8_t encrypted_read_data_key[16], encrypted_write_data_key[16];
    char str[512];

    BD_DEBUG(DBG_MMC, "Reading data keys from drive...\n");

    if (_mmc_read_data_keys(mmc, agid, encrypted_read_data_key, encrypted_write_data_key)) {
        if (read_data_key) {
            int err = crypto_aes128d(bus_key, encrypted_read_data_key, read_data_key);
            if (err) {
                LOG_CRYPTO_ERROR(DBG_MMC, "decrypting read data key failed", err);
                return MMC_ERROR;
            }
            if (DEBUG_KEYS) {
                BD_DEBUG(DBG_MMC, "READ DATA KEY       : %s\n", str_print_hex(str, read_data_key, 16));
            }
        }
        if (write_data_key) {
            int err = crypto_aes128d(bus_key, encrypted_write_data_key, write_data_key);
            if (err) {
                LOG_CRYPTO_ERROR(DBG_MMC, "decrypting write data key failed", err);
                return MMC_ERROR;
            }
            if (DEBUG_KEYS) {
                BD_DEBUG(DBG_MMC, "WRITE DATA KEY      : %s\n", str_print_hex(str, write_data_key, 16));
            }
        }

        return MMC_SUCCESS;
    }

    BD_DEBUG(DBG_MMC | DBG_CRIT, "Unable to read data keys from drive!\n");

    return MMC_ERROR;
}

int mmc_read_auth(MMC *mmc, const uint8_t *host_priv_key, const uint8_t *host_cert,
                  int request, uint8_t *p1, uint8_t *p2)
{
    uint8_t agid = 0, bus_key[16];
    int error_code;

    _mmc_invalidate_agids(mmc);

    if (!_mmc_report_agid(mmc, &agid)) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Didn't get AGID from drive\n");
        return MMC_ERROR;
    }
    BD_DEBUG(DBG_MMC, "Got AGID from drive: %d\n", agid);

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
            BD_DEBUG(DBG_MMC | DBG_CRIT, "unknown mmc_read_auth() request %d\n", request);
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
    int crypto_error;

    if (mmc->drive_cert[0] == 0x01) {
        memcpy(drive_cert, mmc->drive_cert, 92);
        return MMC_SUCCESS;
    }

    if (!mmc->read_drive_cert) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Drive does not support reading drive certificate\n");
    }

    if (!_mmc_report_key(mmc, 0, 0, 0, 0x38, buf, 116)) {
        if (mmc->read_drive_cert) {
            BD_DEBUG(DBG_MMC | DBG_CRIT, "Failed reading drive certificate\n");
        }
        return MMC_ERROR;
    }

    crypto_error = crypto_aacs_verify_drive_cert(buf + 4);
    if (crypto_error) {
        LOG_CRYPTO_ERROR(DBG_MMC, "drive certificate signature verification failed", crypto_error);
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
