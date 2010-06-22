/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
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
 *
 * In addition, as a special exception, the copyright holders of libaacs
 * gives permission to link the code of its release of libaacs with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked library.  You must obey the GNU Lesser General Public
 * License in all respects for all of the code used other than "OpenSSL".
 * If you modify this file, you may extend this exception to your
 * version of the file, but you are not obligated to do so.  If you do
 * not wish to do so, delete this exception statement from your version.
 */

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
#include <gcrypt.h>

int _validate_pk(uint8_t *pk, uint8_t *cvalue, uint8_t *uv, uint8_t *vd,
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
        DEBUG(DBG_AACS, "Processing key is valid!\n");
        ret = 1;
    }

    return ret;
}

int _calc_mk(AACS *aacs, const char *path)
{
    int a, num_uvs = 0;
    size_t len;
    uint8_t *buf = NULL, *rec, *uvs;
    MKB *mkb = NULL;

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

    return 0;
}

int _calc_vuk(AACS *aacs, const char *path)
{
    int a;
    MMC* mmc = NULL;

    DEBUG(DBG_AACS, "Calculate volume unique key...\n");

    cert_list *hccursor = aacs->cf->host_cert_list;
    while (hccursor && hccursor->host_priv_key) {
        uint8_t priv_key[20], cert[92], nonce[20], key_point[40];
        hexstring_to_hex_array(priv_key, sizeof(priv_key),
                               hccursor->host_priv_key);
        hexstring_to_hex_array(cert, sizeof(cert), hccursor->host_cert);
        hexstring_to_hex_array(nonce, sizeof(nonce), hccursor->host_nonce);
        hexstring_to_hex_array(key_point, sizeof(key_point),
                               hccursor->host_key_point);

        if ((mmc = mmc_open(path, priv_key, cert, nonce, key_point))) {
            if (mmc_read_vid(mmc, aacs->vid)) {
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

                return 1;
            }

            mmc_close(mmc);
        }

        hccursor = hccursor->next;
    }

    DEBUG(DBG_AACS, "Error calculating VUK!\n");

    return 0;
}

int _calc_uks(AACS *aacs, const char *path)
{
    FILE_H *fp = NULL;
    uint8_t buf[16];
    char f_name[100];
    uint64_t f_pos;
    unsigned int i;

    DEBUG(DBG_AACS, "Calculate CPS unit keys...\n");

    snprintf(f_name , 100, "/%s/AACS/Unit_Key_RO.inf", path);

    if ((fp = file_open(f_name, "rb"))) {
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

            file_close(fp);

            return aacs->num_uks;
        }

        file_close(fp);
    }

    DEBUG(DBG_AACS, "Could not calculate unit keys!\n");

    return 0;
}



int _verify_ts(uint8_t *buf, size_t size)
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

int _find_vuk(AACS *aacs, const char *path)
{
    uint8_t hash[20], *ukf_buf;
    FILE_H *fp = NULL;
    int64_t f_size;
    char f_name[100];
    char str[48];

    snprintf(f_name , 100, "/%s/AACS/Unit_Key_RO.inf", path);

    if ((fp = file_open(f_name, "rb"))) {
        file_seek(fp, 0, SEEK_END);
        f_size = file_tell(fp);
        file_seek(fp, 0, SEEK_SET);

        ukf_buf = malloc(f_size);

        if ((file_read(fp, ukf_buf, f_size)) == f_size) {

        } else {
            DEBUG(DBG_AACS,
                  "Failed to read %"PRIu64" bytes from unit key file!\n",
                  f_size);
            file_close(fp);
            X_FREE(ukf_buf);
            return 0;
        }
    } else {
        DEBUG(DBG_AACS, "Failed to open unit key file: %s!\n", f_name);
        return 0;
    }

    file_close(fp);

    crypto_aacs_title_hash(ukf_buf, f_size, hash);
    DEBUG(DBG_AACS, "Disc ID: %s\n", print_hex(str, hash, 20));

    X_FREE(ukf_buf);

    if (aacs->cf) {
        aacs->ce = aacs->cf->list;
        while (aacs->ce && aacs->ce->entry.discid) {
            uint8_t discid[20];
            memset(discid, 0, sizeof(discid));
            hexstring_to_hex_array(discid, sizeof(discid),
                                   aacs->ce->entry.discid);
            if (!memcmp(hash, discid, 20)) {
                hexstring_to_hex_array(aacs->vuk, sizeof(aacs->vuk),
                                       aacs->ce->entry.vuk);

                DEBUG(DBG_AACS, "Found volume unique key for %s: %s\n",
                      aacs->ce->entry.discid, print_hex(str, aacs->vuk, 16));

                return 1;
            }

            aacs->ce = aacs->ce->next;
        }
    }

    return 0;
}

int _decrypt_unit(AACS *aacs, uint8_t *buf, uint32_t len, uint64_t offset,
                  uint32_t curr_uk)
{
    gcry_cipher_hd_t gcry_h;
    uint8_t *tmp_buf = malloc(len);

    memcpy(tmp_buf, buf, len);

    int a;
    uint8_t key[16], iv[] = "\x0b\xa0\xf8\xdd\xfe\xa6\x1f\xb3"
                            "\xd8\xdf\x9f\x56\x6a\x05\x0f\x78";

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(gcry_h, aacs->uks + curr_uk * 16, 16);
    gcry_cipher_encrypt(gcry_h, key, 16, tmp_buf, 16);
    gcry_cipher_close(gcry_h);

    for (a = 0; a < 16; a++) {
        key[a] ^= tmp_buf[a];
    }

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);
    gcry_cipher_setkey(gcry_h, key, 16);
    gcry_cipher_setiv(gcry_h, iv, 16);
    gcry_cipher_decrypt(gcry_h, tmp_buf + 16, len - 16, tmp_buf + 16, len - 16);
    gcry_cipher_close(gcry_h);

    if (_verify_ts(tmp_buf,len)) {
        DEBUG(DBG_AACS, "Decrypted %s unit [%d bytes] from offset %ld (%p)\n",
              len % 6144 ? "PARTIAL" : "FULL", len, offset, aacs);

        memcpy(buf, tmp_buf, len);

        X_FREE(tmp_buf);

        return 1;
    }

    X_FREE(tmp_buf);

    if (curr_uk < aacs->num_uks - 1) {
        return _decrypt_unit(aacs, buf, len, offset, curr_uk++);
    }

    return 0;
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

    char *cfgfile = NULL;
    if (configfile_path) {
        cfgfile = (char*)malloc(strlen(configfile_path) + 1);
        strcpy(cfgfile, configfile_path);
    } else {
        /* If no configfile path given, check for configfiles in user's home or
         * under /etc.
         */
        char *userhome = getenv("HOME");
        cfgfile = (char*)malloc(strlen(userhome) +
            sizeof("/.libaacs/KEYDB.cfg") + 1);
        strcpy(cfgfile,userhome);
        strcat(cfgfile,"/.libaacs/KEYDB.cfg");
        FILE *fp = fopen(cfgfile, "r");
        if (!fp) {
            cfgfile = (char*)realloc(cfgfile, sizeof("/etc/libaacs/KEYDB.cfg"));
            strcpy(cfgfile, "/etc/libaacs/KEYDB.cfg");
            fp = fopen(cfgfile, "r");
            if (!fp) {
                DEBUG(DBG_AACS, "No configfile found!\n");
                X_FREE(cfgfile);
                return NULL;
            }
        }
        fclose(fp);
        fp = NULL;
    }

    AACS *aacs = calloc(1, sizeof(AACS));

    aacs->cf = keydbcfg_new_config_file();
    if (keydbcfg_parse_config(aacs->cf, cfgfile)) {
        DEBUG(DBG_AACS, "Searching for VUK...\n");
        if(_find_vuk(aacs, path)) {
            if (_calc_uks(aacs, path)) {
                keydbcfg_config_file_close(aacs->cf);
                aacs->cf = NULL;

                DEBUG(DBG_AACS, "AACS initialized! (%p)\n", aacs);
                return aacs;
            }
        }

        DEBUG(DBG_AACS, "Starting AACS waterfall...\n");
        //_calc_pk(aacs);
        if (_calc_mk(aacs, path)) {
           if (_calc_vuk(aacs, path)) {
                if (_calc_uks(aacs, path)) {
                    DEBUG(DBG_AACS, "AACS initialized! (%p)\n", aacs);
                    return aacs;
                }
            }
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

    DEBUG(DBG_AACS, "AACS destroyed! (%p)\n", aacs);

    X_FREE(aacs);
}

int aacs_decrypt_unit(AACS *aacs, uint8_t *buf, uint32_t len, uint64_t offset)
{
    if (len > 6144) {
        DEBUG(DBG_AACS, "aacs_decrypt_unit(): len > 6144 ! (%p)\n", aacs);
        return 0;
    }
    if (offset % 6144) {
        DEBUG(DBG_AACS, "aacs_decrypt_unit(): offset inside block ! (%p)\n", aacs);
        return 0;
    }

    return _decrypt_unit(aacs, buf, len, offset, 0);
}

uint8_t *aacs_get_vid(AACS *aacs)
{
    return aacs->vid;
}
