/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2009-2010  npzacs
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

#include <util/attributes.h>

#include "aacs.h"
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


struct aacs {
    uint8_t pk[16], mk[16], vuk[16], vid[16], disc_id[20], *uks;
    uint32_t num_uks;
    struct config_file_t *cf;
    struct title_entry_list_t *ce;

    uint32_t num_titles;
    uint16_t current_cps_unit;
    uint16_t *cps_units;  /* [0] = first play ; [1] = top menu ; [2] = title 1 ... */
};

static const uint8_t empty_key[] = "\x00\x00\x00\x00\x00\x00\x00\x00"
                                   "\x00\x00\x00\x00\x00\x00\x00\x00";

static int _validate_pk(const uint8_t *pk,
                        const uint8_t *cvalue, const uint8_t *uv, const uint8_t *vd,
                        uint8_t *mk)
{
    gcry_cipher_hd_t gcry_h;
    int a, ret = 0;
    uint8_t dec_vd[16];
    char str[40];

    DEBUG(DBG_AACS, "Validate processing key %s...\n", print_hex(str, pk, 16));
    DEBUG(DBG_AACS, " Using:\n");
    DEBUG(DBG_AACS, "   UV: %s\n", print_hex(str, uv, 4));
    DEBUG(DBG_AACS, "   cvalue: %s\n", print_hex(str, cvalue, 16));
    DEBUG(DBG_AACS, "   Verification data: %s\n", print_hex(str, vd, 16));

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
        DEBUG(DBG_AACS, "Processing key %s is valid!\n", print_hex(str, pk, 16));
        ret = 1;
    }

    return ret;
}

static int _calc_mk(AACS *aacs, const char *path)
{
    int a, num_uvs = 0;
    size_t len;
    uint8_t *buf = NULL;
    MKB *mkb = NULL;
    const uint8_t *rec, *uvs;

    /* Skip if retrieved from config file */
    if (memcmp(aacs->mk, empty_key, 16))
      return 1;

    DEBUG(DBG_AACS, "Calculate media key...\n");

    if ((mkb = mkb_open(path))) {
        DEBUG(DBG_AACS, "Get UVS...\n");
        uvs = mkb_subdiff_records(mkb, &len);
        rec = uvs;
        while (rec < uvs + len) {
            if (rec[0] & 0xc0)
                break;
            rec += 5;
            num_uvs++;
        }

        DEBUG(DBG_AACS, "Get cvalues...\n");
        rec = mkb_cvalues(mkb, &len);
        if (aacs->cf->pkl) {
            pk_list *pkcursor = aacs->cf->pkl;
            while (pkcursor && pkcursor->key) {
                hexstring_to_hex_array(aacs->pk, sizeof(aacs->pk),
                                       pkcursor->key);
                DEBUG(DBG_AACS, "Trying processing key...\n");

                for (a = 0; a < num_uvs; a++) {
                    if (_validate_pk(aacs->pk, rec + a * 16, uvs + 1 + a * 5,
                      mkb_mk_dv(mkb), aacs->mk)) {
                        mkb_close(mkb);
                        X_FREE(buf);

                        char str[40];
                        DEBUG(DBG_AACS, "Media key: %s\n", print_hex(str, aacs->mk,
                                                                     16));
                        return 1;
                    }
                }

                pkcursor = pkcursor->next;
            }
        }

        mkb_close(mkb);
        X_FREE(buf);
    }

    DEBUG(DBG_AACS, "Error calculating media key!\n");

    return 0;
}

static int _calc_vuk(AACS *aacs, const char *path)
{
    int a;
    MMC* mmc = NULL;

    /* Skip if retrieved from config file */
    if (memcmp(aacs->vuk, empty_key, 16))
      return 1;

    /* get cached vuk */
    if (keycache_find("vuk", aacs->disc_id, aacs->vuk, 16)) {
        DEBUG(DBG_AACS, "Using cached VUK\n");
        return 1;
    }

    /* make sure we have media key */
    if (!_calc_mk(aacs, path)) {
        return 0;
    }

    DEBUG(DBG_AACS, "Calculate volume unique key...\n");

    /* Use VID given in config file if available */
    if (memcmp(aacs->vid, empty_key, 16))
    {
        gcry_cipher_hd_t gcry_h;
        gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
        gcry_cipher_setkey(gcry_h, aacs->mk, 16);
        gcry_cipher_decrypt(gcry_h, aacs->vuk, 16, aacs->vid, 16);
        gcry_cipher_close(gcry_h);

        for (a = 0; a < 16; a++) {
            aacs->vuk[a] ^= aacs->vid[a];
        }

        char str[40];
        DEBUG(DBG_AACS, "Volume unique key: %s\n",
              print_hex(str, aacs->vuk, 16));

        /* cache vuk */
        keycache_save("vuk", aacs->disc_id, aacs->vuk, 16);

        return 1;
    }

    cert_list *hccursor = aacs->cf->host_cert_list;
    while (hccursor && hccursor->host_priv_key && hccursor->host_cert) {
        uint8_t priv_key[20], cert[92];
        hexstring_to_hex_array(priv_key, sizeof(priv_key),
                               hccursor->host_priv_key);
        hexstring_to_hex_array(cert, sizeof(cert), hccursor->host_cert);

        if (!crypto_aacs_verify_host_cert(cert)) {
	    char str[2*92+1];
	    DEBUG(DBG_AACS, "Not using invalid host certificate %s.\n",
		  print_hex(str, cert, 92));

	    hccursor = hccursor->next;
	    continue;
	}

        char id_str[20];
        DEBUG(DBG_AACS, "Trying host certificate (id 0x%s)...\n",
              print_hex(id_str, cert + 4, 6));

        if ((mmc = mmc_open(path))) {
            if (mmc_read_vid(mmc, priv_key, cert, aacs->vid)) {
                gcry_cipher_hd_t gcry_h;
                gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES,
                                 GCRY_CIPHER_MODE_ECB, 0);
                gcry_cipher_setkey(gcry_h, aacs->mk, 16);
                gcry_cipher_decrypt(gcry_h, aacs->vuk, 16, aacs->vid, 16);
                gcry_cipher_close(gcry_h);

                for (a = 0; a < 16; a++) {
                    aacs->vuk[a] ^= aacs->vid[a];
                }

                mmc_close(mmc);

                char str[40];
                DEBUG(DBG_AACS, "Volume unique key: %s\n", print_hex(str, aacs->vuk,
                                                                    16));

                /* cache vuk */
                keycache_save("vuk", aacs->disc_id, aacs->vuk, 16);

                return 1;
            }

            mmc_close(mmc);
        }

        hccursor = hccursor->next;
    }

    DEBUG(DBG_AACS, "Error calculating VUK!\n");

    return 0;
}

static uint16_t _read_u16(AACS_FILE_H *fp)
{
  uint8_t data[2];

  file_read(fp, data, sizeof(uint16_t));

  return MKINT_BE16(data);
}

static void _read_uks_map(AACS *aacs, AACS_FILE_H *fp)
{
    uint16_t first_play, top_menu;
    unsigned i;

    DEBUG(DBG_AACS, "Assigning CPS units to titles ...\n");

    X_FREE(aacs->cps_units);
    aacs->current_cps_unit = 0;

    file_seek(fp, 16 + 4, SEEK_SET);

    first_play = _read_u16(fp);
    top_menu   = _read_u16(fp);

    DEBUG(DBG_AACS, "Title FP : CPS unit %d\n", first_play);
    DEBUG(DBG_AACS, "Title TM : CPS unit %d\n", top_menu);

    aacs->num_titles   = _read_u16(fp);
    aacs->cps_units    = calloc(sizeof(uint16_t), aacs->num_titles + 2);
    aacs->cps_units[0] = first_play;
    aacs->cps_units[1] = top_menu;

    for (i = 2; i < aacs->num_titles + 2; i++) {
        _read_u16(fp); /* reserved */
        aacs->cps_units[i] = _read_u16(fp);
        DEBUG(DBG_AACS, "Title %02d : CPS unit %d\n", i - 1, aacs->cps_units[i]);
    }

    /* validate */
    for (i = 0; i < aacs->num_titles + 2; i++) {
        if (aacs->cps_units[i])
            aacs->cps_units[i]--; /* number [1...N] --> index [0...N-1] */
        if (aacs->cps_units[i] >= aacs->num_uks) {
            DEBUG(DBG_AACS, " *** Invalid CPS unit for title %d: %d !\n", (int) i - 1, aacs->cps_units[i]);
            aacs->cps_units[i] = 0;
        }
    }
}

static int _calc_uks(AACS *aacs, const char *path)
{
    AACS_FILE_H *fp = NULL;
    char    *f_name;
    uint8_t  buf[16];
    uint64_t f_pos;
    unsigned int i;

    /* Skip if retrieved from config file */
    if (aacs->uks)
        return 1;

    /* Make sure we have VUK */
    if (!_calc_vuk(aacs, path)) {
        return 0;
    }

    DEBUG(DBG_AACS, "Calculate CPS unit keys...\n");

    f_name = str_printf("%s/AACS/Unit_Key_RO.inf", path);
    fp = file_open(f_name, "rb");
    X_FREE(f_name);

    if (fp) {
        if ((file_read(fp, buf, 4)) == 4) {
            f_pos = MKINT_BE32(buf);

            // Read number of keys
            file_seek(fp, f_pos, SEEK_SET);
            if ((file_read(fp, buf, 2)) == 2) {
                aacs->num_uks = MKINT_BE16(buf);

                X_FREE(aacs->uks);
                aacs->uks = calloc(aacs->num_uks, 16);

                DEBUG(DBG_AACS, "%d CPS unit keys\n", aacs->num_uks);

            } else {
                aacs->num_uks = 0;
                DEBUG(DBG_AACS, "Error reading number of unit keys!\n");
            }

            // Read keys
            for (i = 0; i < aacs->num_uks; i++) {
                f_pos += 48;

                file_seek(fp, f_pos, SEEK_SET);
                if ((file_read(fp, buf, 16)) != 16) {
                    DEBUG(DBG_AACS, "Unit key %d: read error\n", i);
                    aacs->num_uks = i;
                    break;
                }

                gcry_cipher_hd_t gcry_h;
                gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES,
                                 GCRY_CIPHER_MODE_ECB, 0);
                gcry_cipher_setkey(gcry_h, aacs->vuk, 16);
                gcry_cipher_decrypt(gcry_h, aacs->uks + 16*i, 16, buf, 16);
                gcry_cipher_close(gcry_h);

                char str[40];
                DEBUG(DBG_AACS, "Unit key %d: %s\n", i,
                      print_hex(str, aacs->uks + 16*i, 16));
            }

            _read_uks_map(aacs, fp);

            file_close(fp);

            return aacs->num_uks;
        }

        file_close(fp);
    }

    DEBUG(DBG_AACS, "Could not calculate unit keys!\n");

    return 0;
}

static int _calc_title_hash(const char *path, uint8_t *title_hash)
{
    AACS_FILE_H *fp = NULL;
    uint8_t *ukf_buf;
    char     str[48];
    int64_t  f_size;
    char    *f_name;

    f_name = str_printf("%s/AACS/Unit_Key_RO.inf", path);

    if (!(fp = file_open(f_name, "rb"))) {
        DEBUG(DBG_AACS, "Failed to open unit key file: %s!\n", f_name);
        X_FREE(f_name);
        return 0;
    }

    X_FREE(f_name);

    file_seek(fp, 0, SEEK_END);
    f_size = file_tell(fp);
    file_seek(fp, 0, SEEK_SET);

    ukf_buf = malloc(f_size);

    if ((file_read(fp, ukf_buf, f_size)) != f_size) {

        DEBUG(DBG_AACS, "Failed to read %"PRIu64" bytes from unit key file!\n", f_size);

        file_close(fp);
        X_FREE(ukf_buf);

        return 0;
    }

    crypto_aacs_title_hash(ukf_buf, f_size, title_hash);
    DEBUG(DBG_AACS, "Disc ID: %s\n", print_hex(str, title_hash, 20));

    file_close(fp);
    X_FREE(ukf_buf);

    return 1;
}

static int _verify_ts(uint8_t *buf, size_t size)
{
    uint8_t *ptr;

    if (size < 192) {
        return 1;
    }

    for (ptr=buf; ptr < buf+192; ptr++) {
        int failed = 0;
        if (*ptr == 0x47) {
            uint8_t *ptr2;

            for (ptr2=ptr; ptr2 < buf + size; ptr2 += 192) {
                if (*ptr2 != 0x47) {
                    failed = 1;
                    break;
                }
            }
            if (!failed) {
                return 1;
            }
        }
        ptr++;
    }

    DEBUG(DBG_AACS, "Failed to verify TS!\n");

    return 0;
}

/* Function that collects keys from keydb config entry */
static uint32_t _find_config_entry(AACS *aacs, const char *path)
{
    uint8_t discid[20];
    char str[48];
    uint32_t retval = 0;
    aacs->uks = NULL;
    aacs->num_uks = 0;

    if (!_calc_title_hash(path, aacs->disc_id)) {
        return 0;
    }

    if (aacs->cf && aacs->cf->list) {
        aacs->ce = aacs->cf->list;
        while (aacs->ce && aacs->ce->entry.discid) {
            memset(discid, 0, sizeof(discid));
            hexstring_to_hex_array(discid, sizeof(discid),
                                   aacs->ce->entry.discid);
            if (!memcmp(aacs->disc_id, discid, 20)) {
                DEBUG(DBG_AACS, "Found config entry for discid %s\n",
                      aacs->ce->entry.discid);
                break;
            }

            aacs->ce = aacs->ce->next;
        }

        if (aacs->ce->entry.mek) {
            hexstring_to_hex_array(aacs->mk, sizeof(aacs->mk),
                                    aacs->ce->entry.mek);

            DEBUG(DBG_AACS, "Found media key for %s: %s\n",
                  aacs->ce->entry.discid, print_hex(str, aacs->mk, 16));

            retval = 1;
        }

        if (aacs->ce->entry.vid) {
            hexstring_to_hex_array(aacs->vid, sizeof(aacs->vid),
                                    aacs->ce->entry.vid);

            DEBUG(DBG_AACS, "Found volume id for %s: %s\n",
                  aacs->ce->entry.discid, print_hex(str, aacs->vid, 16));

            retval = 1;
        }

        if (aacs->ce->entry.vuk) {
            hexstring_to_hex_array(aacs->vuk, sizeof(aacs->vuk),
                                    aacs->ce->entry.vuk);

            DEBUG(DBG_AACS, "Found volume unique key for %s: %s\n",
                  aacs->ce->entry.discid, print_hex(str, aacs->vuk, 16));

            retval = 1;
        }

        if (aacs->ce && aacs->ce->entry.uk) {
            DEBUG(DBG_AACS, "Acquire CPS unit keys from keydb config file...\n");

            digit_key_pair_list *ukcursor = aacs->ce->entry.uk;
            while (ukcursor && ukcursor->key_pair.key) {
                aacs->num_uks++;

                aacs->uks = (uint8_t*)realloc(aacs->uks, 16 * aacs->num_uks);
                hexstring_to_hex_array(aacs->uks + (16 * (aacs->num_uks - 1)), 16,
                                      ukcursor->key_pair.key);

                char str[40];
                DEBUG(DBG_AACS, "Unit key %d from keydb entry: %s\n",
                      aacs->num_uks,
                      print_hex(str, aacs->uks + (16 * (aacs->num_uks - 1)), 16));

                ukcursor = ukcursor->next;
            }
        }
    }

    if (aacs->num_uks)
        retval = aacs->num_uks;

    return retval;
}

#define ALIGNED_UNIT_LEN 6144
static int _decrypt_unit(AACS *aacs, uint8_t *out_buf, const uint8_t *in_buf, uint32_t curr_uk)
{
    gcry_cipher_hd_t gcry_h;
    int a;
    uint8_t key[16], iv[] = "\x0b\xa0\xf8\xdd\xfe\xa6\x1f\xb3"
                            "\xd8\xdf\x9f\x56\x6a\x05\x0f\x78";

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(gcry_h, aacs->uks + curr_uk * 16, 16);
    gcry_cipher_encrypt(gcry_h, key, 16, in_buf, 16);
    gcry_cipher_close(gcry_h);

    for (a = 0; a < 16; a++) {
        key[a] ^= in_buf[a];
    }

    memcpy(out_buf, in_buf, 16); /* first 16 bytes are plain */

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);
    gcry_cipher_setkey(gcry_h, key, 16);
    gcry_cipher_setiv(gcry_h, iv, 16);
    gcry_cipher_decrypt(gcry_h, out_buf + 16, ALIGNED_UNIT_LEN - 16, in_buf + 16, ALIGNED_UNIT_LEN - 16);
    gcry_cipher_close(gcry_h);

    if (_verify_ts(out_buf, ALIGNED_UNIT_LEN)) {
        return 1;
    }

    if (curr_uk < aacs->num_uks - 1) {
        return _decrypt_unit(aacs, out_buf, in_buf, curr_uk++);
    }

    return 0;
}

static int _load_config(AACS *aacs, const char *configfile_path)
{
    int config_ok = 0;

    aacs->cf = keydbcfg_new_config_file();

    /* try to load KEYDB.cfg */

    if (configfile_path) {
        config_ok = keydbcfg_parse_config(aacs->cf, configfile_path);

    } else {
        /* If no configfile path given, check for config files in user's home or
         * under /etc.
         */
        char *cfgfile = keydbcfg_find_config_file();
        config_ok = keydbcfg_parse_config(aacs->cf, cfgfile);
        X_FREE(cfgfile);
    }

    /* Try to load simple (aacskeys) config files */

    config_ok = keydbcfg_load_pk_file(aacs->cf)   || config_ok;
    config_ok = keydbcfg_load_cert_file(aacs->cf) || config_ok;

    if (!config_ok) {
        DEBUG(DBG_AACS, "No valid configuration files found!\n");
    }

    return config_ok;
}

AACS *aacs_open(const char *path, const char *configfile_path)
{
    DEBUG(DBG_AACS, "libaacs [%zd]\n", sizeof(AACS));

    DEBUG(DBG_AACS, "Initializing libgcrypt...\n");
    if (!crypto_init())
    {
        DEBUG(DBG_AACS, "Failed to initialize libgcrypt\n");
        return NULL;
    }

    AACS *aacs = calloc(1, sizeof(AACS));

    if (_load_config(aacs, configfile_path)) {
        DEBUG(DBG_AACS, "Searching for keydb config entry...\n");
        _find_config_entry(aacs, path);

        DEBUG(DBG_AACS, "Starting AACS waterfall...\n");
        if (_calc_uks(aacs, path)) {
            keydbcfg_config_file_close(aacs->cf);
            aacs->cf = NULL;

            DEBUG(DBG_AACS, "AACS initialized! (%p)\n", aacs);
            return aacs;
        }

        keydbcfg_config_file_close(aacs->cf);
        aacs->cf = NULL;
    }

    DEBUG(DBG_AACS, "Failed to initialize AACS! (%p)\n", aacs);

    aacs_close(aacs);

    return NULL;
}

void aacs_close(AACS *aacs)
{
    X_FREE(aacs->uks);
    X_FREE(aacs->cps_units);

    DEBUG(DBG_AACS, "AACS destroyed! (%p)\n", aacs);

    X_FREE(aacs);
}

int aacs_decrypt_unit(AACS *aacs, uint8_t *buf)
{
    uint8_t out_buf[ALIGNED_UNIT_LEN];

    if (!(buf[0] & 0xc0)) {
        // TP_extra_header Copy_permission_indicator == 0, unit is not encrypted
        return 1;
    }

    if (_decrypt_unit(aacs, out_buf, buf, aacs->current_cps_unit)) {
        memcpy(buf, out_buf, ALIGNED_UNIT_LEN);

        // Clear copy_permission_indicator bits
        int i;
        for (i = 0; i < 6144; i += 192) {
            buf[i] &= ~0xc0;
        }

        return 1;
    }

    DEBUG(DBG_AACS, "Failed decrypting unit [6144 bytes] (%p)\n", aacs);

    return 0;
}

const uint8_t *aacs_get_disc_id(AACS *aacs)
{
    return aacs->disc_id;
}

const uint8_t *aacs_get_vid(AACS *aacs)
{
    return aacs->vid;
}

void aacs_select_title(AACS *aacs, uint32_t title)
{
    if (!aacs) {
        return;
    }

    if (!aacs->cps_units) {
        DEBUG(DBG_AACS|DBG_CRIT, "aacs_select_title(): CPS units not read ! (%p)\n", aacs);
        return;
    }

    if (title == 0xffff) {
        /* first play */
        aacs->current_cps_unit = aacs->cps_units[0];
        DEBUG(DBG_AACS, "aacs_set_title(first_play): CPS unit %d (%p)\n", aacs->current_cps_unit, aacs);
        return;
    }

    if (title <= aacs->num_titles) {
        aacs->current_cps_unit = aacs->cps_units[title + 1];
        DEBUG(DBG_AACS, "aacs_set_title(%d): CPS unit %d (%p)\n", title, aacs->current_cps_unit, aacs);
        return;
    }

    DEBUG(DBG_AACS|DBG_CRIT, "aacs_set_title(%d): invalid title ! (%p)\n", title, aacs);
}
