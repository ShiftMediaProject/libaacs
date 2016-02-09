/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2009-2013  npzacs
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

#include "util/attributes.h"

#include "aacs-version.h"
#include "aacs.h"
#include "cci.h"
#include "cci_data.h"
#include "content_cert.h"
#include "crypto.h"
#include "mmc.h"
#include "mkb.h"
#include "file/file.h"
#include "file/keydbcfg.h"
#include "util/macro.h"
#include "util/logging.h"
#include "util/strutl.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <gcrypt.h>


#define SECTOR_LEN       2048  /* bus encryption block size */
#define ALIGNED_UNIT_LEN 6144  /* aacs block size */


struct aacs {
    void           *fopen_handle;
    AACS_FILE_OPEN2 fopen;

    /* current disc */
    char     *path;
    int       mkb_version;
    uint8_t   disc_id[20];

    /* VID is cached for BD-J */
    uint8_t   vid[16];
    /* PMSN is cached for BD-J */
    uint8_t   pmsn[16];
    /* Media key is cached for BD+ */
    uint8_t   mk[16];

    /* unit key for each CPS unit */
    uint32_t  num_uks;
    uint8_t  *uks;

    /* CPS unit of currently selected title */
    uint16_t  current_cps_unit;
    uint8_t   cps_unit_selected;

    /* title -> CPS unit mappings */
    uint32_t  num_titles;
    uint16_t *cps_units;  /* [0] = first play ; [1] = top menu ; [2] = title 1 ... */

    /* bus encryption */
    int       bee;        /* bus encryption enabled flag in content certificate */
    int       bec;        /* bus encryption capable flag in drive certificate */
    uint8_t   read_data_key[16];

    /* content certificate */
    CONTENT_CERT *cc;

    /* AACS Online (BD-J) */
    uint8_t   device_nonce[16];
    uint8_t   device_binding_id[16];
};

static const uint8_t empty_key[20] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00 };
static const uint8_t aacs_iv[16]   = { 0x0b, 0xa0, 0xf8, 0xdd, 0xfe, 0xa6, 0x1f, 0xb3,
                                       0xd8, 0xdf, 0x9f, 0x56, 0x6a, 0x05, 0x0f, 0x78 };

static int _validate_pk(const uint8_t *pk,
                        const uint8_t *cvalue, const uint8_t *uv, const uint8_t *vd,
                        uint8_t *mk)
{
    gcry_cipher_hd_t gcry_h;
    int a;
    uint8_t dec_vd[16];
    char str[40];

    BD_DEBUG(DBG_AACS, "Validate processing key %s...\n", str_print_hex(str, pk, 16));
    BD_DEBUG(DBG_AACS, " Using:\n");
    BD_DEBUG(DBG_AACS, "   UV: %s\n", str_print_hex(str, uv, 4));
    BD_DEBUG(DBG_AACS, "   cvalue: %s\n", str_print_hex(str, cvalue, 16));
    BD_DEBUG(DBG_AACS, "   Verification data: %s\n", str_print_hex(str, vd, 16));

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(gcry_h, pk, 16);
    gcry_cipher_decrypt(gcry_h, mk, 16, cvalue, 16);

    for (a = 0; a < 4; a++) {
        mk[a + 12] ^= uv[a];
    }

    gcry_cipher_setkey(gcry_h, mk, 16);
    gcry_cipher_decrypt (gcry_h, dec_vd, 16, vd, 16);
    gcry_cipher_close(gcry_h);

    if (!memcmp(dec_vd, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8)) {
        BD_DEBUG(DBG_AACS, "Processing key %s is valid!\n", str_print_hex(str, pk, 16));
        return AACS_SUCCESS;
    }

    return AACS_ERROR_NO_PK;
}

static int _rl_verify_signature(const uint8_t *rl, size_t size)
{
    int    entries = MKINT_BE32(rl + 12 + 8);
    size_t len     = 12 + 12 + 8 * entries; /* type_and_version_rec=12, rl_header=12, rl=entries*8 */

    if (len + 40 > size) {
        BD_DEBUG(DBG_AACS, "revocation list size mismatch\n");
        return 0;
    }

    return crypto_aacs_verify_aacsla(rl + len, rl, len);
}

static void _save_rl(const char *name, uint32_t version, const uint8_t *version_rec,
                     const uint8_t *rl_rec, size_t rl_len)
{
    int len     = MKINT_BE24(rl_rec - 3);
    int entries = MKINT_BE32(rl_rec + 4); /* entries in first signature block */
    if (len < 4 || !entries) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "ignoring empty %s\n", name);
        return;
    }

    if (rl_rec && version_rec) {
        rl_rec -= 4;
        rl_len += 4;

        uint8_t *data = malloc(12 + rl_len);
        memcpy(data,      version_rec, 12);
        memcpy(data + 12, rl_rec,      rl_len);
        if (!_rl_verify_signature(data, rl_len + 12)) {
          BD_DEBUG(DBG_AACS | DBG_CRIT, "invalid %s signature, not using it\n", name);
        } else {
          cache_save(name, version, data, rl_len + 12);
        }
        X_FREE(data);
    }
}

static void _update_rl(MKB *mkb)
{
    uint32_t version = mkb_version(mkb);
    uint32_t cache_version;
    size_t   rl_len;

    if (!cache_get("drl", &cache_version, NULL, NULL, 0) || cache_version < version) {
        const uint8_t *version_rec = mkb_type_and_version_record(mkb);
        const uint8_t *drl_rec     = mkb_drive_revokation_entries(mkb, &rl_len);
        if (drl_rec && version_rec && rl_len > 8) {
            _save_rl("drl", version, version_rec, drl_rec, rl_len);
        }
    }
    if (!cache_get("hrl", &cache_version, NULL, NULL, 0) || cache_version < version) {
        const uint8_t *version_rec = mkb_type_and_version_record(mkb);
        const uint8_t *hrl_rec     = mkb_host_revokation_entries(mkb, &rl_len);
        if (hrl_rec && version_rec && rl_len > 8) {
            _save_rl("hrl", version, version_rec, hrl_rec, rl_len);
        }
    }
}

static uint32_t _calc_v_mask(uint32_t uv)
{
    uint32_t v_mask = 0xffffffff;

    while (!(uv & ~v_mask)) {
        v_mask <<= 1;
    }

    return v_mask;
}

static void _calc_pk(const uint8_t *dk, uint8_t *pk, uint32_t uv, uint32_t v_mask, uint32_t dev_key_v_mask)
{
    unsigned char left_child[16], right_child[16];

    crypto_aesg3(dk, left_child, right_child, pk);

    while (dev_key_v_mask != v_mask) {

        int i;
        for (i = 31; i >= 0; i--) {
            if (!(dev_key_v_mask & (1ul << i))) {
                break;
            }
        }

        uint8_t curr_key[16];
        if (!(uv & (1ul << i))) {
            memcpy(curr_key, left_child, 16);
        } else {
            memcpy(curr_key, right_child, 16);
        }

        crypto_aesg3(curr_key, left_child, right_child, pk);

        dev_key_v_mask = ((int) dev_key_v_mask) >> 1;
    }

    char str[40];
    BD_DEBUG(DBG_AACS, "Processing key: %s\n",  str_print_hex(str, pk, 16));
}

static dk_list *_find_dk(dk_list *dkl, uint32_t *p_dev_key_v_mask, uint32_t uv, uint32_t u_mask)
{
    uint32_t device_number = dkl->node;
    uint32_t dev_key_uv, dev_key_u_mask, dev_key_v_mask;
    unsigned key_idx = 0;

    for (; dkl; dkl = dkl->next) {
        if (device_number != dkl->node) {
            /* wrong device */
            continue;
        }
        if (!dkl->uv) {
            continue;
        }
        key_idx++;
        dev_key_uv     = dkl->uv;
        dev_key_u_mask = 0xffffffff << dkl->u_mask_shift;
        dev_key_v_mask = _calc_v_mask(dev_key_uv);

        if ((u_mask == dev_key_u_mask) &&
            ((uv & dev_key_v_mask) == (dev_key_uv & dev_key_v_mask))) {
            break;
        }
    }

    if (!dkl) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "could not find applying device key (device 0x%x)\n", device_number);
    } else {
        char str[128];
        BD_DEBUG(DBG_AACS, "Applying device key is #%d %s\n", key_idx, str_print_hex(str, dkl->key, 16));
        BD_DEBUG(DBG_AACS, "  UV: 0x%08x  U mask: 0x%08x  V mask: 0x%08x\n", dev_key_uv, dev_key_u_mask, dev_key_v_mask);
        *p_dev_key_v_mask = dev_key_v_mask;
    }

    return dkl;
}

static int _calc_pk_mk(MKB *mkb, dk_list *dkl, uint8_t *mk)
{
    /* calculate processing key and media key using device keys */

    const uint8_t *uvs, *cvalues, *mk_dv;
    unsigned num_uvs;
    size_t len;
    char str[128];

    /* get mkb data */

    uvs     = mkb_subdiff_records(mkb, &len);
    cvalues = mkb_cvalues(mkb, &len);
    mk_dv   = mkb_mk_dv(mkb);

    if (!uvs || !cvalues || !mk_dv) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Missing MKB records (uvs %p, cvalues %p, mk_dv %p)\n",
                 (const void*)uvs, (const void*)cvalues, (const void*)mk_dv);
        return AACS_ERROR_CORRUPTED_DISC;
    }

    num_uvs = len / 5;

    if (num_uvs < 1) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "No UVS detected - corrupted disc\n");
        return AACS_ERROR_CORRUPTED_DISC;
    }

    /* loop over all known devices */

    dk_list *dk_num;
    uint32_t device_number = (uint32_t)-1;

    for (dk_num = dkl; dk_num; dk_num = dk_num->next) {

        /* find next device */

        if (device_number == dk_num->node) {
            continue;
        }
        device_number = dk_num->node;

        /* find applying subset difference */

        unsigned uvs_idx;
        uint32_t u_mask, v_mask, uv;

        for (uvs_idx = 0; uvs_idx < num_uvs; uvs_idx++) {
            const uint8_t *p_uv = uvs + 1 + 5 * uvs_idx;
            uint8_t u_mask_shift = p_uv[-1];

            uv = MKINT_BE32(p_uv);
            if (!uv) {
                continue;
            }

            if (u_mask_shift & 0xc0) {
                BD_DEBUG(DBG_AACS | DBG_CRIT, "device 0x%x is revoked\n", device_number);
                uvs_idx = num_uvs;

            } else {

                u_mask = 0xffffffff << u_mask_shift;
                v_mask = _calc_v_mask(uv);

                if (((device_number & u_mask) == (uv & u_mask)) && ((device_number & v_mask) != (uv & v_mask))) {
                    break;
                }
            }
        }

        if (uvs_idx >= num_uvs) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "could not find applying subset-difference for device 0x%x\n", device_number);
            /* try next device */
            continue;
        }

        BD_DEBUG(DBG_AACS, "Applying subset-difference for device 0x%x is #%d:\n", device_number, uvs_idx);
        BD_DEBUG(DBG_AACS,"  UV: 0x%08x  U mask: 0x%08x  V mask: 0x%08x\n", uv, u_mask, v_mask);

        /* find applying device key */

        uint32_t dev_key_v_mask = 0;
        dk_list *dk;

        dk = _find_dk(dk_num, &dev_key_v_mask, uv, u_mask);
        if (!dk) {
            /* try next device */
            continue;
        }

        /* calculate processing key */

        uint8_t pk[16];
        _calc_pk(dk->key, pk, uv, v_mask, dev_key_v_mask);

        /* calculate and verify media key */

        if ( _validate_pk(pk,
                          cvalues + uvs_idx * 16,
                          uvs + 1 + uvs_idx * 5,
                          mk_dv,
                          mk)
             == AACS_SUCCESS) {

            BD_DEBUG(DBG_AACS, "Media key: %s\n", str_print_hex(str, mk, 16));
            return AACS_SUCCESS;
        }

        BD_DEBUG(DBG_AACS | DBG_CRIT, "Processing key %s is invalid!\n", str_print_hex(str, pk, 16));

        /* try next device */
    }

    return AACS_ERROR_NO_DK;
}

static AACS_FILE_H *_file_open(AACS *aacs, const char *file)
{
    AACS_FILE_H *fp;
    char        *f_name;

    if (aacs->fopen) {
        return aacs->fopen(aacs->fopen_handle, file);
    }
    if (!aacs->path) {
        return NULL;
    }

    f_name = str_printf("%s" DIR_SEP "%s", aacs->path, file);
    if (!f_name) {
        return NULL;
    }
    fp = file_open(f_name, "rb");
    X_FREE(f_name);

    return fp;
}

static size_t _read_file(AACS *aacs, const char *file, void **data)
{
    AACS_FILE_H *fp = NULL;
    int64_t f_size;
    size_t size;

    *data = NULL;

    fp = _file_open(aacs, file);
    if (!fp) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Unable to open %s\n", file);
        return 0;
    }

    f_size = file_size(fp);
    size = f_size;
    if (f_size <= 0 || f_size != (int64_t)size) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid size %"PRId64" for %s\n", f_size, file);
        file_close(fp);
        return 0;
    }

    *data = malloc(size);
    if (*data) {
        if (file_read(fp, *data, f_size) != f_size) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "Failed reading %s\n", file);
            X_FREE(*data);
        }
    } else {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Out of memory\n");
    }

    file_close(fp);

    return *data ? size : 0;
}

static MKB *_mkb_open(AACS *aacs)
{
    size_t  size;
    void   *data;
    MKB    *mkb;

    size = _read_file(aacs, "AACS" DIR_SEP "MKB_RO.inf", &data);
    if (size < 4) {
        X_FREE(data);
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Error reading MKB file (AACS/MKB_RO.inf)\n");
        return NULL;
    }

    mkb = mkb_init(data, size);
    if (!mkb) {
        X_FREE(data);
    }
    return mkb;
}

static int _calc_mk(AACS *aacs, uint8_t *mk, pk_list *pkl, dk_list *dkl)
{
    int a, num_uvs = 0;
    size_t len;
    MKB *mkb = NULL;
    const uint8_t *rec, *uvs, *mk_dv;

    /* Skip if retrieved from config file */
    if (memcmp(mk, empty_key, 16)) {
        return AACS_SUCCESS;
    }

    BD_DEBUG(DBG_AACS, "Calculate media key...\n");

    if ((mkb = _mkb_open(aacs))) {

        aacs->mkb_version = mkb_version(mkb);
        _update_rl(mkb);

        /* try device keys first */
        if (dkl && _calc_pk_mk(mkb, dkl, mk) == AACS_SUCCESS) {
            memcpy(aacs->mk, mk, sizeof(aacs->mk));
            mkb_close(mkb);
            return AACS_SUCCESS;
        }

        mk_dv = mkb_mk_dv(mkb);
        if (!mk_dv) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "Missing MKB DV record\n");
            mkb_close(mkb);
            return AACS_ERROR_CORRUPTED_DISC;
        }

        BD_DEBUG(DBG_AACS, "Get UVS...\n");
        uvs = mkb_subdiff_records(mkb, &len);
        rec = uvs;
        while (rec < uvs + len) {
            if (rec[0] & 0xc0)
                break;
            rec += 5;
            num_uvs++;
        }

        BD_DEBUG(DBG_AACS, "Get cvalues...\n");
        rec = mkb_cvalues(mkb, &len);
        if (!rec) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "Missing MKB CVALUES record\n");
            mkb_close(mkb);
            return AACS_ERROR_CORRUPTED_DISC;
        }

        for (; pkl; pkl = pkl->next) {
                BD_DEBUG(DBG_AACS, "Trying processing key...\n");

                for (a = 0; a < num_uvs; a++) {
                    if (AACS_SUCCESS == _validate_pk(pkl->key, rec + a * 16, uvs + 1 + a * 5, mk_dv, mk)) {
                        mkb_close(mkb);

                        char str[40];
                        BD_DEBUG(DBG_AACS, "Media key: %s\n", str_print_hex(str, mk, 16));
                        memcpy(aacs->mk, mk, sizeof(aacs->mk));
                        return AACS_SUCCESS;
                    }
                }
            }

        mkb_close(mkb);

        BD_DEBUG(DBG_AACS | DBG_CRIT, "Error calculating media key. Missing right processing key ?\n");
        return AACS_ERROR_NO_PK;
    }

    BD_DEBUG(DBG_AACS | DBG_CRIT, "Error calculating media key - corrupted disc\n");
    return AACS_ERROR_CORRUPTED_DISC;
}

static int _mmc_read_auth(AACS *aacs, cert_list *hcl, int type, uint8_t *p1, uint8_t *p2)
{
    MMC* mmc = NULL;
    if (!(mmc = mmc_open(aacs->path))) {
        return AACS_ERROR_MMC_OPEN;
    }

    int error_code = AACS_ERROR_NO_CERT;

    const uint8_t *drive_cert = mmc_get_drive_cert(mmc);

    for (; hcl ; hcl = hcl->next) {

        char tmp_str[2*92+1];

        if (!crypto_aacs_verify_host_cert(hcl->host_cert)) {
            BD_DEBUG(DBG_AACS, "Not using invalid host certificate %s.\n",
                  str_print_hex(tmp_str, hcl->host_cert, 92));
            continue;
        }

        if (drive_cert && (drive_cert[1] & 0x01) && !(hcl->host_cert[1] & 0x01)) {
            BD_DEBUG(DBG_AACS, "Certificate (id 0x%s) does not support bus encryption\n",
                  str_print_hex(tmp_str, hcl->host_cert + 4, 6));
            //continue;
        }

        BD_DEBUG(DBG_AACS, "Trying host certificate (id 0x%s)...\n",
              str_print_hex(tmp_str, hcl->host_cert + 4, 6));

        int mmc_result = mmc_read_auth(mmc, hcl->host_priv_key, hcl->host_cert, type, p1, p2);
        switch (mmc_result) {
            case MMC_SUCCESS:
                mmc_close(mmc);
                return AACS_SUCCESS;
            case MMC_ERROR_CERT_REVOKED:
                error_code = AACS_ERROR_CERT_REVOKED;
                break;
            case MMC_ERROR:
            default:
                error_code = AACS_ERROR_MMC_FAILURE;
                break;
        }
    }

    mmc_close(mmc);

    return error_code;
}

static int _read_vid(AACS *aacs, cert_list *hcl)
{
    /* Use VID given in config file if available */
    if (memcmp(aacs->vid, empty_key, sizeof(aacs->vid))) {
        return AACS_SUCCESS;
    }

    int error_code = _mmc_read_auth(aacs, hcl, MMC_READ_VID, aacs->vid, NULL);
    if (error_code != AACS_SUCCESS) {
        BD_DEBUG(DBG_AACS, "Error reading VID!\n");
    } else {
        /* cache vid */
        keycache_save("vid", aacs->disc_id, aacs->vid, 16);
    }
    return error_code;
}

static int _read_read_data_key(AACS *aacs, cert_list *hcl)
{
    int error_code = _mmc_read_auth(aacs, hcl, MMC_READ_DATA_KEYS, aacs->read_data_key, NULL);
    if (error_code != AACS_SUCCESS) {
        BD_DEBUG(DBG_AACS, "Error reading data keys!\n");
    }
    return error_code;
}

static int _read_pmsn(AACS *aacs, cert_list *hcl)
{
    int error_code = _mmc_read_auth(aacs, hcl, MMC_READ_PMSN, aacs->pmsn, NULL);
    if (error_code != AACS_SUCCESS) {
        BD_DEBUG(DBG_AACS, "Error reading PMSN!\n");
    }
    return error_code;
}

static int _calc_vuk(AACS *aacs, uint8_t *mk, uint8_t *vuk, config_file *cf)
{
    int error_code;

    /* Skip if retrieved from config file */
    if (memcmp(vuk, empty_key, 16)) {
        BD_DEBUG(DBG_AACS, "Using VUK from config file\n");
        return AACS_SUCCESS;
    }

    /* get cached vuk */
    if (keycache_find("vuk", aacs->disc_id, vuk, 16)) {
        BD_DEBUG(DBG_AACS, "Using cached VUK\n");
        return AACS_SUCCESS;
    }

    if (!cf) {
        return AACS_ERROR_NO_CONFIG;
    }

    /* make sure we have media key */
    error_code = _calc_mk(aacs, mk, cf->pkl, cf->dkl);
    if (error_code != AACS_SUCCESS) {
        return error_code;
    }

    /* acquire VID */
    error_code = _read_vid(aacs, cf->host_cert_list);
    if (error_code != AACS_SUCCESS) {
        return error_code;
    }

    /* calculate VUK */

    crypto_aes128d(mk, aacs->vid, vuk);

    int a;
    for (a = 0; a < 16; a++) {
        vuk[a] ^= aacs->vid[a];
    }

    char str[40];
    BD_DEBUG(DBG_AACS, "Volume unique key: %s\n", str_print_hex(str, vuk, 16));

    /* cache vuk */
    keycache_save("vuk", aacs->disc_id, vuk, 16);

    return AACS_SUCCESS;
}

static uint16_t _read_u16(AACS_FILE_H *fp)
{
  uint8_t data[2] = {0, 0};

  file_read(fp, data, sizeof(uint16_t));

  return MKINT_BE16(data);
}

static void _read_uks_map(AACS *aacs, AACS_FILE_H *fp)
{
    uint16_t first_play, top_menu;
    unsigned i;

    BD_DEBUG(DBG_AACS, "Assigning CPS units to titles ...\n");

    X_FREE(aacs->cps_units);
    aacs->current_cps_unit = 0;

    file_seek(fp, 16 + 4, SEEK_SET);

    first_play = _read_u16(fp);
    top_menu   = _read_u16(fp);

    BD_DEBUG(DBG_AACS, "Title FP : CPS unit %d\n", first_play);
    BD_DEBUG(DBG_AACS, "Title TM : CPS unit %d\n", top_menu);

    aacs->num_titles   = _read_u16(fp);
    aacs->cps_units    = calloc(sizeof(uint16_t), aacs->num_titles + 2);
    aacs->cps_units[0] = first_play;
    aacs->cps_units[1] = top_menu;

    for (i = 2; i < aacs->num_titles + 2; i++) {
        _read_u16(fp); /* reserved */
        aacs->cps_units[i] = _read_u16(fp);
        BD_DEBUG(DBG_AACS, "Title %02d : CPS unit %d\n", i - 1, aacs->cps_units[i]);
    }

    /* validate */
    for (i = 0; i < aacs->num_titles + 2; i++) {
        if (aacs->cps_units[i])
            aacs->cps_units[i]--; /* number [1...N] --> index [0...N-1] */
        if (aacs->cps_units[i] >= aacs->num_uks) {
            BD_DEBUG(DBG_AACS, " *** Invalid CPS unit for title %d: %d !\n", (int) i - 1, aacs->cps_units[i]);
            aacs->cps_units[i] = 0;
        }
    }
}

/* Function that collects keys from keydb config entry */
static void _find_config_entry(AACS *aacs, title_entry_list *ce,
                               uint8_t *mk, uint8_t *vuk)
{
    char str[48];
    char str2[48];

    aacs->uks = NULL;
    aacs->num_uks = 0;

        while (ce && ce->entry.discid) {
            if (!memcmp(aacs->disc_id, ce->entry.discid, 20)) {
                BD_DEBUG(DBG_AACS, "Found config entry for discid %s\n",
                      str_print_hex(str, ce->entry.discid, 20));
                break;
            }

            ce = ce->next;
        }
        if (!ce) {
            return;
        }

        if (ce->entry.mek) {
            hexstring_to_hex_array(mk, 16, ce->entry.mek);

            BD_DEBUG(DBG_AACS, "Found media key for %s: %s\n",
                  str_print_hex(str2, ce->entry.discid, 20), str_print_hex(str, mk, 16));
        }

        if (ce->entry.vid) {
            hexstring_to_hex_array(aacs->vid, sizeof(aacs->vid),
                                    ce->entry.vid);

            BD_DEBUG(DBG_AACS, "Found volume id for %s: %s\n",
                  str_print_hex(str2, ce->entry.discid, 20), str_print_hex(str, aacs->vid, 16));
        }

        if (ce->entry.vuk) {
            hexstring_to_hex_array(vuk, 16, ce->entry.vuk);

            BD_DEBUG(DBG_AACS, "Found volume unique key for %s: %s\n",
                  str_print_hex(str2, ce->entry.discid, 20), str_print_hex(str, vuk, 16));
        }

        if (ce->entry.uk) {
            BD_DEBUG(DBG_AACS, "Acquire CPS unit keys from keydb config file...\n");

            digit_key_pair_list *ukcursor = ce->entry.uk;
            while (ukcursor && ukcursor->key_pair.key) {
                aacs->num_uks++;

                aacs->uks = (uint8_t*)realloc(aacs->uks, 16 * aacs->num_uks);
                hexstring_to_hex_array(aacs->uks + (16 * (aacs->num_uks - 1)), 16,
                                      ukcursor->key_pair.key);

                BD_DEBUG(DBG_AACS, "Unit key %d from keydb entry: %s\n",
                      aacs->num_uks,
                      str_print_hex(str, aacs->uks + (16 * (aacs->num_uks - 1)), 16));

                ukcursor = ukcursor->next;
            }
        }
}

static size_t _read_cci_file(AACS *aacs, int cps_unit, void **data)
{
    char   *fname;
    size_t  size = 0;

    fname = str_printf("AACS" DIR_SEP "CPSUnit%05d.cci", cps_unit);
    if (fname) {
        size = _read_file(aacs, fname, data);
        X_FREE(fname);

        if (size >= 2048) {
            return size;
        }

        X_FREE(*data);
    }

    /* try backup copy */

    fname = str_printf("AACS" DIR_SEP "DUPLICATE" DIR_SEP "CPSUnit%05d.cci", cps_unit);
    if (fname) {
        size = _read_file(aacs, fname, data);
        X_FREE(fname);

        if (size >= 2048) {
            return size;
        }

        X_FREE(*data);
    }

    return 0;
}

static AACS_CCI *_read_cci(AACS *aacs, int cps_unit)
{
    AACS_CCI *cci;
    void     *data;
    size_t    size;

    size = _read_cci_file(aacs, cps_unit, &data);
    if (!size) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Failed to read CPS unit usage file %d\n", cps_unit);
        return NULL;
    }

    cci = cci_parse(data, size);
    X_FREE(data);

    return cci;
}

static int _check_cci_unencrypted(AACS *aacs, int cps_unit)
{
    int result = -1;

    AACS_CCI *cci = _read_cci(aacs, cps_unit);
    if (cci) {
        result = cci_is_unencrypted(cci);
        cci_free(&cci);
    }

    return result;
}

static int _calc_uks(AACS *aacs, config_file *cf)
{
    AACS_FILE_H *fp = NULL;
    uint8_t  buf[16];
    uint64_t f_pos;
    unsigned int i;
    int error_code = AACS_SUCCESS;
    int vuk_error_code;

    uint8_t mk[16] = {0}, vuk[16] = {0};

    if (cf) {
        BD_DEBUG(DBG_AACS, "Searching for keydb config entry...\n");
        _find_config_entry(aacs, cf->list, mk, vuk);

        /* Skip if retrieved from config file */
        if (aacs->uks) {
            return AACS_SUCCESS;
        }
    }

    /* Try to calculate VUK */
    vuk_error_code = _calc_vuk(aacs, mk, vuk, cf);

    BD_DEBUG(DBG_AACS, "Calculate CPS unit keys...\n");

    fp = _file_open(aacs, "AACS" DIR_SEP "Unit_Key_RO.inf");
    if (!fp) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Error opening unit key file (AACS/Unit_Key_RO.inf)\n");
        return AACS_ERROR_CORRUPTED_DISC;
    }

    if ((file_read(fp, buf, 4)) == 4) {
        f_pos = MKINT_BE32(buf);

        // Read number of keys
        file_seek(fp, f_pos, SEEK_SET);
        if ((file_read(fp, buf, 2)) == 2) {
            aacs->num_uks = MKINT_BE16(buf);

            X_FREE(aacs->uks);
            aacs->uks = calloc(aacs->num_uks, 16);

            BD_DEBUG(DBG_AACS, "%d CPS unit keys\n", aacs->num_uks);

        } else {
            aacs->num_uks = 0;
            BD_DEBUG(DBG_AACS | DBG_CRIT, "Error reading number of unit keys\n");
            error_code = AACS_ERROR_CORRUPTED_DISC;
        }

        // Read keys
        for (i = 0; i < aacs->num_uks; i++) {

            /* error out if VUK calculation fails and encrypted CPS unit is found */
            if (vuk_error_code != AACS_SUCCESS) {
                if (!_check_cci_unencrypted(aacs, i + 1)) {
                    error_code = vuk_error_code;
                    break;
                }
                BD_DEBUG(DBG_AACS | DBG_CRIT, "WARNING: VUK calculation failed but disc seems to be unencrypted.\n");
            }

            f_pos += 48;
            file_seek(fp, f_pos, SEEK_SET);
            if ((file_read(fp, buf, 16)) != 16) {
                BD_DEBUG(DBG_AACS | DBG_CRIT, "Unit key %d: read error\n", i);
                aacs->num_uks = i;
                error_code = AACS_ERROR_CORRUPTED_DISC;
                break;
            }

            crypto_aes128d(vuk, buf, aacs->uks + 16*i);

            char str[40];
            BD_DEBUG(DBG_AACS, "Unit key %d: %s\n", i,
                  str_print_hex(str, aacs->uks + 16*i, 16));
        }

        /* failing next is not fatal, it just slows down things */
        _read_uks_map(aacs, fp);

        file_close(fp);

        return error_code;
    }

    file_close(fp);

    BD_DEBUG(DBG_AACS | DBG_CRIT, "Error reading unit keys\n");
    return AACS_ERROR_CORRUPTED_DISC;
}

static int _calc_title_hash(AACS *aacs)
{
    void    *data;
    size_t   size;
    char     str[48];
    int      result = AACS_SUCCESS;

    size = _read_file(aacs, "AACS" DIR_SEP "Unit_Key_RO.inf", &data);
    if (!size) {
        size = _read_file(aacs, "AACS" DIR_SEP "DUPLICATE" DIR_SEP "Unit_Key_RO.inf", &data);
    }

    if (size) {
        crypto_aacs_title_hash(data, size, aacs->disc_id);
        BD_DEBUG(DBG_AACS, "Disc ID: %s\n", str_print_hex(str, aacs->disc_id, 20));
        X_FREE(data);

    } else {
        result = AACS_ERROR_CORRUPTED_DISC;
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Failed to read %lu bytes from unit key file (AACS/Unit_Key_RO.inf)", (unsigned long)size);
    }

    return result;
}

static CONTENT_CERT *_read_cc_any(AACS *aacs)
{
    CONTENT_CERT *cc = NULL;
    void   *data;
    size_t  size;

    size = _read_file(aacs, "AACS" DIR_SEP "Content000.cer", &data);
    if (!size) {
        size = _read_file(aacs, "AACS" DIR_SEP "Content001.cer", &data);
    }

    if (size) {
        cc = cc_parse(data, size);
        X_FREE(data);
    } else {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Failed to read content certificate file\n");
    }

    return cc;
}

static int _get_bus_encryption_enabled(AACS *aacs)
{
    if (!aacs->cc) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Failed to read Bus Encryption Enabled flag from content certificate file\n");
        return 0;
    }

    if (aacs->cc->bus_encryption_enabled_flag) {
        BD_DEBUG(DBG_AACS, "Bus Encryption Enabled flag in content certificate: %d\n",
                 aacs->cc->bus_encryption_enabled_flag);
        return 1;
    }

    return 0;
}

static int _get_bus_encryption_capable(const char *path)
{
    MMC* mmc = NULL;
    uint8_t drive_cert[92];
    int bec = 0;

    if (!(mmc = mmc_open(path))) {
        return 0;
    }

    if (mmc_read_drive_cert(mmc, drive_cert) == MMC_SUCCESS) {
        bec = drive_cert[1] & 1;
        BD_DEBUG(DBG_AACS, "Bus Encryption Capable flag in drive certificate: %d\n", bec);
    } else {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Unable to read drive certificate\n");
    }

    mmc_close(mmc);
    return bec;
}

static int _verify_ts(uint8_t *buf)
{
    int i;
    for (i = 0; i < ALIGNED_UNIT_LEN; i += 192) {
        if (BD_UNLIKELY(buf[i + 4] != 0x47)) {
            return 0;
        }

        /* Clear copy_permission_indicator bits */
        buf[i] &= ~0xc0;
    }
    return 1;
}

static int _decrypt_unit(AACS *aacs, uint8_t *out_buf, const uint8_t *in_buf, uint32_t curr_uk)
{
    /* inbuf == NULL means in-place decryption */

    gcry_cipher_hd_t gcry_h;
    int a;
    uint8_t key[16];

    if (BD_UNLIKELY(in_buf != NULL)) {
        memcpy(out_buf, in_buf, 16); /* first 16 bytes are plain */
    }

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(gcry_h, aacs->uks + curr_uk * 16, 16);
    gcry_cipher_encrypt(gcry_h, key, 16, out_buf, 16); /* here out_buf is plain data fron in_buf */
    gcry_cipher_close(gcry_h);

    for (a = 0; a < 16; a++) {
        key[a] ^= out_buf[a]; /* here out_buf is plain data fron in_buf */
    }

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);
    gcry_cipher_setkey(gcry_h, key, 16);
    gcry_cipher_setiv(gcry_h, aacs_iv, 16);
    if (BD_UNLIKELY(in_buf != NULL)) {
        gcry_cipher_decrypt(gcry_h, out_buf + 16, ALIGNED_UNIT_LEN - 16, in_buf + 16, ALIGNED_UNIT_LEN - 16);
    } else {
        gcry_cipher_decrypt(gcry_h, out_buf + 16, ALIGNED_UNIT_LEN - 16, NULL, 0);
    }
    gcry_cipher_close(gcry_h);

    if (_verify_ts(out_buf)) {
        return 1;
    }

    return 0;
}

static void _decrypt_bus(AACS *aacs, uint8_t *buf)
{
    gcry_cipher_hd_t gcry_h;

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);
    gcry_cipher_setkey(gcry_h, aacs->read_data_key, 16);
    gcry_cipher_setiv(gcry_h, aacs_iv, 16);
    gcry_cipher_decrypt(gcry_h, buf + 16, SECTOR_LEN - 16, NULL, 0);
    gcry_cipher_close(gcry_h);
}

void aacs_get_version(int *major, int *minor, int *micro)
{
    *major = AACS_VERSION_MAJOR;
    *minor = AACS_VERSION_MINOR;
    *micro = AACS_VERSION_MICRO;
}

/* aacs_open2() wrapper for backwards compability */
AACS *aacs_open(const char *path, const char *configfile_path)
{
    int error_code;
    AACS *aacs;

    aacs = aacs_open2(path, configfile_path, &error_code);
    if (!aacs || error_code == AACS_SUCCESS) {
        return aacs;
    }

    aacs_close(aacs);
    return NULL;
}

/* aacs_open_device() wrapper for backward compability */
AACS *aacs_open2(const char *path, const char *configfile_path, int *error_code)
{
    AACS *aacs = aacs_init();
    if (aacs) {
        *error_code = aacs_open_device(aacs, path, configfile_path);
    }
    return aacs;
}

AACS *aacs_init()
{
    BD_DEBUG(DBG_AACS, "libaacs "AACS_VERSION_STRING" [%u]\n", (unsigned)sizeof(AACS));

    BD_DEBUG(DBG_AACS, "Initializing libgcrypt...\n");
    if (!crypto_init()) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Failed to initialize libgcrypt\n");
        return NULL;
    }

    return calloc(1, sizeof(AACS));
}

void aacs_set_fopen(AACS *aacs, void *handle, AACS_FILE_OPEN2 p)
{
    if (aacs) {
        aacs->fopen = p;
        aacs->fopen_handle = handle;
    }
}

int aacs_open_device(AACS *aacs, const char *path, const char *configfile_path)
{
    config_file *cf;
    int error_code;

    aacs->path = path ? str_dup(path) : NULL;

    error_code = _calc_title_hash(aacs);
    if (error_code != AACS_SUCCESS) {
        return error_code;
    }

    cf = keydbcfg_config_load(configfile_path);

    BD_DEBUG(DBG_AACS, "Starting AACS waterfall...\n");
    error_code = _calc_uks(aacs, cf);
    if (error_code != AACS_SUCCESS) {
        BD_DEBUG(DBG_AACS, "Failed to initialize AACS!\n");
    }

    aacs->cc = _read_cc_any(aacs);

    aacs->bee = _get_bus_encryption_enabled(aacs);
    aacs->bec = _get_bus_encryption_capable(path);

    if (error_code == AACS_SUCCESS && aacs->bee && aacs->bec) {

        if (!cf) {
            return AACS_ERROR_NO_CONFIG;
        }

        error_code = _read_read_data_key(aacs, cf->host_cert_list);
        if (error_code != AACS_SUCCESS) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "Unable to initialize bus encryption required by drive and disc\n");
        }
    }

    keydbcfg_config_file_close(cf);

    BD_DEBUG(DBG_AACS, "AACS initialized!\n");

    return error_code;
}

void aacs_close(AACS *aacs)
{
    if (!aacs)
        return;

    /* erase sensitive data */
    if (aacs->uks) {
        memset(aacs->uks, 0, 16 * aacs->num_uks);
    }

    X_FREE(aacs->uks);
    X_FREE(aacs->cps_units);
    X_FREE(aacs->path);

    cc_free(&aacs->cc);

    BD_DEBUG(DBG_AACS, "AACS destroyed!\n");

    /* erase sensitive data */
    memset(aacs, 0, sizeof(*aacs));

    X_FREE(aacs);
}

static void _decrypt_unit_bus(AACS *aacs, uint8_t *buf)
{
    if (aacs->bee && aacs->bec) {
        unsigned int i;
        for (i = 0; i < ALIGNED_UNIT_LEN; i += SECTOR_LEN) {
            _decrypt_bus(aacs, buf + i);
        }
    }
}

int aacs_decrypt_bus(AACS *aacs, uint8_t *buf)
{
    if (buf[0] & 0xc0) {
        _decrypt_unit_bus(aacs, buf);
    }

    return 1;
}

int aacs_decrypt_unit(AACS *aacs, uint8_t *buf)
{
    unsigned int i;

    if (!(buf[0] & 0xc0)) {
        // TP_extra_header Copy_permission_indicator == 0, unit is not encrypted
        return 1;
    }

    /* handle bus encryption first */
    _decrypt_unit_bus(aacs, buf);

    /* decrypt in-place if current unit key is known */
    if (BD_LIKELY(aacs->cps_unit_selected) || BD_LIKELY(aacs->num_uks == 1)) {
        if (BD_LIKELY(_decrypt_unit(aacs, buf, NULL, aacs->current_cps_unit))) {
            return 1;
        }

    } else {

        /* unit key is unknown (playback without menus), try each key until right key is found */
        uint8_t out_buf[ALIGNED_UNIT_LEN];
        for (i = 0; i < aacs->num_uks; i++) {
            if (_decrypt_unit(aacs, out_buf, buf, i)) {
                BD_DEBUG(DBG_AACS, "autodetected current CPS unit (%d)\n", i);
                aacs->current_cps_unit  = i;
                aacs->cps_unit_selected = 1;
                memcpy(buf, out_buf, ALIGNED_UNIT_LEN);
                return 1;
            }
        }
    }

    BD_DEBUG(DBG_AACS, "Failed decrypting unit [6144 bytes]\n");
    return 0;
}

int aacs_get_mkb_version(AACS *aacs)
{
    if (!aacs->mkb_version) {
        MKB *mkb;
        if ((mkb = _mkb_open(aacs))) {
            aacs->mkb_version = mkb_version(mkb);
            mkb_close(mkb);
        }
    }
    return aacs->mkb_version;
}

const uint8_t *aacs_get_disc_id(AACS *aacs)
{
    if (memcmp(aacs->disc_id, empty_key, sizeof(aacs->disc_id))) {
        return aacs->disc_id;
    }

    return NULL;
}

const uint8_t *aacs_get_content_cert_id(AACS *aacs)
{
    if (aacs && aacs->cc) {
        return aacs->cc->cc_id;
    }
    return NULL;
}

const uint8_t *aacs_get_bdj_root_cert_hash(AACS *aacs)
{
    if (aacs && aacs->cc) {
        return aacs->cc->bdj_root_cert_hash;
    }
    return NULL;
}

const uint8_t *aacs_get_mk(AACS *aacs)
{
    if (!memcmp(aacs->mk, empty_key, sizeof(aacs->mk))) {
        config_file *cf = keydbcfg_config_load(NULL);
        if (cf) {
            uint8_t mk[16] = {0};
            if (_calc_mk(aacs, mk, cf->pkl, cf->dkl) == AACS_SUCCESS) {
                memcpy(aacs->mk, mk, 16);
            }

            keydbcfg_config_file_close(cf);
        }

        if (!memcmp(aacs->mk, empty_key, sizeof(aacs->mk))) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "aacs_get_mk() failed\n");
            return NULL;
        }
    }

    return aacs->mk;
}

const uint8_t *aacs_get_vid(AACS *aacs)
{
    if (!memcmp(aacs->vid, empty_key, sizeof(aacs->vid))) {
        /* get cached vid */
        if (keycache_find("vid", aacs->disc_id, aacs->vid, 16)) {
            BD_DEBUG(DBG_AACS, "Using cached VID\n");
            return aacs->vid;
        }

        config_file *cf = keydbcfg_config_load(NULL);
        if (cf) {
            _read_vid(aacs, cf->host_cert_list);

            keydbcfg_config_file_close(cf);
        }

        if (!memcmp(aacs->vid, empty_key, sizeof(aacs->vid))) {
            BD_DEBUG(DBG_AACS | DBG_CRIT, "aacs_get_vid() failed\n");
            return NULL;
        }
    }

    return aacs->vid;
}

const uint8_t *aacs_get_pmsn(AACS *aacs)
{
    if (!memcmp(aacs->pmsn, empty_key, sizeof(aacs->pmsn))) {
        config_file *cf = keydbcfg_config_load(NULL);
        if (cf) {
            _read_pmsn(aacs, cf->host_cert_list);

            keydbcfg_config_file_close(cf);
        }

        if (!memcmp(aacs->pmsn, empty_key, sizeof(aacs->pmsn))) {
            BD_DEBUG(DBG_AACS, "aacs_get_pmsn() failed\n");
            return NULL;
        }
    }

    return aacs->pmsn;
}

const uint8_t *aacs_get_device_binding_id(AACS *aacs)
{
  /* Device binding ID is used to encrypt online content.
   * It needs to be cached so that downloaded content can be played later.
   */
    static const char config_file_name[] = "device_binding_id";
    uint32_t len = sizeof(aacs->device_binding_id);

    BD_DEBUG(DBG_AACS, "reading device binding id\n");
    if (!config_get(config_file_name, &len, aacs->device_binding_id) || len != sizeof(aacs->device_binding_id)) {
        BD_DEBUG(DBG_AACS, "creating device binding id\n");
        crypto_create_nonce(aacs->device_binding_id, sizeof(aacs->device_binding_id));
        config_save(config_file_name, aacs->device_binding_id, sizeof(aacs->device_binding_id));
    }

   return aacs->device_binding_id;
}

const uint8_t *aacs_get_device_nonce(AACS *aacs)
{
    BD_DEBUG(DBG_AACS, "creating device nonce\n");
    crypto_create_nonce(aacs->device_nonce, sizeof(aacs->device_nonce));
    return aacs->device_nonce;
}

static AACS_RL_ENTRY *_get_rl(const char *type, int *num_records, int *mkbv)
{
    uint32_t len, version;
    void *data = NULL;

    *num_records = *mkbv = 0;

    cache_get(type, &version, &len, NULL, 0);

    if (version > 0 && len > 24) {
        data = malloc(len);
        if (cache_get(type, &version, &len, data, len) && len > 24) {

            if (_rl_verify_signature(data, len)) {
                *mkbv = version;
                *num_records = MKINT_BE32((uint8_t*)data + 20);
                memmove(data, (uint8_t*)data + 24, len - 24);

                int ii;
                AACS_RL_ENTRY *rl = data;
                for (ii = 0; ii < *num_records; ii++) {
                    rl[ii].range = MKINT_BE16((uint8_t*)&rl[ii].range);
                }

                return rl;
            }

            BD_DEBUG(DBG_AACS | DBG_CRIT, "invalid signature in cached %s\n", type);
        }

        X_FREE(data);
    }

    return data;
}

AACS_RL_ENTRY *aacs_get_hrl(int *num_records, int *mkbv)
{
    return _get_rl("hrl", num_records, mkbv);
}

AACS_RL_ENTRY *aacs_get_drl(int *num_records, int *mkbv)
{
    return _get_rl("drl", num_records, mkbv);
}

void aacs_free_rl(AACS_RL_ENTRY **rl)
{
    X_FREE(*rl);
}

uint32_t aacs_get_bus_encryption(AACS *aacs)
{
  return (aacs->bee * AACS_BUS_ENCRYPTION_ENABLED) |
         (aacs->bec * AACS_BUS_ENCRYPTION_CAPABLE);
}

static int _cps_unit(AACS *aacs, uint32_t title)
{
    if (!aacs || !aacs->cps_units) {
        BD_DEBUG(DBG_AACS|DBG_CRIT, "CPS units not read !\n");
        return -1;
    }

    if (title == 0xffff) {
        return aacs->cps_units[0];
    } else if (title <= aacs->num_titles) {
        return aacs->cps_units[title + 1];
    }

    BD_DEBUG(DBG_AACS|DBG_CRIT, "invalid title %u\n", title);
    return -1;
}

struct aacs_basic_cci *aacs_get_basic_cci(AACS *aacs, uint32_t title)
{
    AACS_BASIC_CCI *data = NULL;
    int cps_unit;

    cps_unit = _cps_unit(aacs, title);
    if (cps_unit < 0) {
        return NULL;
    }

    AACS_CCI *cci = _read_cci(aacs, cps_unit);
    if (!cci) {
        return NULL;
    }

    const AACS_BASIC_CCI *bcci = cci_get_basic_cci(cci);
    if (bcci) {
        data = malloc(sizeof(*data));
        memcpy(data, bcci, sizeof(*data));
    }

    cci_free(&cci);
    return data;
}

void aacs_select_title(AACS *aacs, uint32_t title)
{
    if (!aacs) {
        return;
    }

    if (!aacs->cps_units) {
        BD_DEBUG(DBG_AACS|DBG_CRIT, "aacs_select_title(): CPS units not read !\n");
        return;
    }

    if (title == 0xffff) {
        /* first play */
        aacs->current_cps_unit = aacs->cps_units[0];
        aacs->cps_unit_selected = 0;
        BD_DEBUG(DBG_AACS, "aacs_set_title(first_play): CPS unit %d\n", aacs->current_cps_unit);
        return;
    }

    if (title <= aacs->num_titles) {
        aacs->current_cps_unit = aacs->cps_units[title + 1];
        aacs->cps_unit_selected = 1;
        BD_DEBUG(DBG_AACS, "aacs_set_title(%d): CPS unit %d\n", title, aacs->current_cps_unit);
        return;
    }

    BD_DEBUG(DBG_AACS|DBG_CRIT, "aacs_set_title(%d): invalid title !\n", title);
}
