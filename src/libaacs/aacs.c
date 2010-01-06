#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>

#include "aacs.h"
#include "crypto.h"
#include "mmc.h"
#include "../util/macro.h"
#include "../util/logging.h"
#include "../file/file.h"

int _validate_pk(uint8_t *pk, uint8_t *cvalue, uint8_t *uv, uint8_t *vd, uint8_t *mk)
{
    int a;
    AES_KEY aes;
    uint8_t dec_vd[16];

    DEBUG(DBG_AACS, "Validate processing key %s...\n", print_hex(pk, 16));
    DEBUG(DBG_AACS, " Using:\n");
    DEBUG(DBG_AACS, "   UV: %s\n", print_hex(uv, 4));
    DEBUG(DBG_AACS, "   cvalue: %s\n", print_hex(cvalue, 16));
    DEBUG(DBG_AACS, "   Verification data: %s\n", print_hex(vd, 16));

    AES_set_decrypt_key(pk, 128, &aes);
    AES_decrypt(cvalue, mk, &aes);

    for (a = 0; a < 4; a++) {
        mk[a + 12] ^= uv[a];
    }

    AES_set_decrypt_key(mk, 128, &aes);
    AES_decrypt(vd, dec_vd, &aes);

    if (!memcmp(dec_vd, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8)) {
        DEBUG(DBG_AACS, "Processing key is valid!\n");
        return 1;
    }

    return 0;
}

int _calc_mk(AACS_KEYS *aacs, const char *path)
{
    DEBUG(DBG_AACS, "Calculate media key...\n");

    int a, num_uvs = 0;
    size_t len;
    uint8_t *buf = NULL, *rec, *uvs, *key_pos, *pks;
    uint16_t num_pks;
    MKB *mkb = NULL;

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
        if ((pks = configfile_record(aacs->kf, KF_PK_ARRAY, &num_pks, NULL))) {
            key_pos = pks;
            while (key_pos < pks + num_pks * 16) {
                memcpy(aacs->pk, key_pos, 16);
                DEBUG(DBG_AACS, "Trying processing key...\n");

                for (a = 0; a < num_uvs; a++) {
                    if (_validate_pk(aacs->pk, rec + a * 16, uvs + 1 + a * 5, mkb_mk_dv(mkb), aacs->mk)) {
                        mkb_close(mkb);
                        X_FREE(buf);

                        DEBUG(DBG_AACS, "Media key: %s\n", print_hex(aacs->mk, 16));
                        return 1;
                    }
                }

                key_pos += 16;
            }
        }

        mkb_close(mkb);
        X_FREE(buf);
    }

    return 0;
}

int _calc_vuk(AACS_KEYS *aacs, const char *path)
{
    int a;
    AES_KEY aes;
    MMC* mmc = NULL;

    DEBUG(DBG_AACS, "Calculate volume unique key...\n");

    if ((mmc = mmc_open(path,
            configfile_record(aacs->kf, KF_HOST_PRIV_KEY, NULL, NULL),
            configfile_record(aacs->kf, KF_HOST_CERT, NULL, NULL),
            configfile_record(aacs->kf, KF_HOST_NONCE, NULL, NULL),
            configfile_record(aacs->kf, KF_HOST_KEY_POINT, NULL, NULL)))) {
        if (mmc_read_vid(mmc, aacs->vid)) {
            AES_set_decrypt_key(aacs->mk, 128, &aes);
            AES_decrypt(aacs->vid, aacs->vuk, &aes);

            for (a = 0; a < 16; a++) {
                aacs->vuk[a] ^= aacs->vid[a];
            }

            mmc_close(mmc);

            DEBUG(DBG_AACS, "Volume unique key: %s\n", print_hex(aacs->vuk, 16));

            return 1;
        }

        mmc_close(mmc);
    }

    DEBUG(DBG_AACS, "Error calculating VUK!\n");

    return 0;
}

int _calc_uks(AACS_KEYS *aacs, const char *path)
{
    AES_KEY aes;
    FILE_H *fp = NULL;
    uint8_t buf[16];
    char f_name[100];
    uint64_t f_pos;

    DEBUG(DBG_AACS, "Calculate CPS unit keys...\n");

    snprintf(f_name , 100, "/%s/AACS/Unit_Key_RO.inf", path);

    if ((fp = file_open(f_name, "rb"))) {
        if ((file_read(fp, buf, 4)) == 4) {
            f_pos = MKINT_BE32(buf) + 48;

            file_seek(fp, f_pos, SEEK_SET);
            if ((file_read(fp, buf, 16)) == 16) {
                // TODO: support more than a single UK!!!
                aacs->uks = malloc(16);

                AES_set_decrypt_key(aacs->vuk, 128, &aes);
                AES_decrypt(buf, aacs->uks, &aes);

                file_close(fp);

                DEBUG(DBG_AACS, "Unit key 1: %s\n", print_hex(aacs->uks, 16));

                return 1;
            }
        }

        file_close(fp);
    }

    DEBUG(DBG_AACS, "Could not calculate unit keys!\n");

    return 0;
}

int _verify_ts(uint8_t *buf, size_t size)
{
    uint8_t *ptr;

    for (ptr=buf; ptr < buf+size; ptr++) {
        if(*ptr == 0x47) {
            uint8_t *ptr2;

            for (ptr2=ptr; ptr2 < buf + size; ptr2 += 192) {
                if (*ptr2 != 0x47) {
                    return 0;
                }
            }
        }
        ptr++;
    }

    return 1;

}

int _find_vuk(AACS_KEYS *aacs, const char *path)
{
    uint8_t *vuks, *key_pos, hash[20], *ukf_buf;
    int num_vuks;
    char f_name[100];

    snprintf(f_name , 100, "/%s/AACS/Unit_Key_RO.inf", path);

    if ((fp = file_open(f_name, "rb"))) {
        uint64_t f_size;

        file_seek(fp, 0, SEEK_END);
        f_size = file_tell(fp);
        file_seek(fp, 0, SEEK_SET);

        ukf_buf = malloc(f_size);

        if ((file_read(fp, ukf_buf, f_size)) == f_size) {

        } else {
            DEBUG(DBG_AACS, "Failed to read %d bytes from unit key file!\n", f_size);
            file_close(fp);
            X_FREE(ukf_buf);
            return 0;
        }
    } else {
        DEBUG(DBG_AACS, "Failed to open unit key file: %s!\n", f_name);
        return 0;
    }

    file_close(fp);

    DEBUG(DBG_AACS, "Search for VUK...\n");

    crypto_aacs_title_hash(ukf_buf, 0, hash);
    DEBUG(DBG_AACS, "Disc ID: %s\n", print_hex(hash, 20));

    X_FREE(ukf_buf);

    if ((vuks = configfile_record(aacs->kf, KF_VUK_ARRAY, &num_vuks, NULL))) {
        key_pos = vuks;
        while (key_pos < vuks + num_vuks * 46) {
            if (!memcmp(hash, key_pos, 20)) {
                uint8_t desc[11];

                memcpy(key_pos + 20, desc, 10);
                desc[10] = 0;

                memcpy(aacs->vuk, key_pos + 36, 16);

                DEBUG(DBG_AACS, "Found volume unique key for %s: %s\n", desc, print_hex(aacs->vuk, 16));
            }

            key_pos += 46;
        }
    }

    return 0;
}

int _decrypt_unit(AACS_KEYS *aacs, uint8_t *buf, uint32_t len, uint64_t offset, uint32_t curr_uk)
{
    uint8_t *tmp_buf = malloc(len);

    memcpy(tmp_buf, buf, len);

    if (offset % 6144) {
        AES_cbc_encrypt(tmp_buf, tmp_buf, len, &aacs->aes, aacs->iv, 0);
    } else {
        int a;
        uint8_t key[16], iv[] = { 0x0b, 0xa0, 0xf8, 0xdd, 0xfe, 0xa6, 0x1f, 0xb3, 0xd8, 0xdf, 0x9f, 0x56, 0x6a, 0x05, 0x0f, 0x78 };

        memcpy(aacs->iv, iv, 16);

        AES_set_encrypt_key(aacs->uks + curr_uk * 16, 128, &aacs->aes);
        AES_encrypt(tmp_buf, key, &aacs->aes);

        for (a = 0; a < 16; a++) {
            key[a] ^= tmp_buf[a];
        }

        AES_set_decrypt_key(key, 128, &aacs->aes);
        AES_cbc_encrypt(tmp_buf + 16, tmp_buf + 16, len - 16, &aacs->aes, aacs->iv, 0);
    }

    if (_verify_ts(tmp_buf,len)) {
        DEBUG(DBG_AACS, "Decrypted %s unit [%d bytes] from offset %ld (0x%08x)\n", len % 6144 ? "PARTIAL" : "FULL", len, offset, aacs);

        memcpy(buf, tmp_buf, len);

        X_FREE(tmp_buf);

        return 1;
    } else if (curr_uk < aacs->num_uks - 1) {
        return _decrypt_unit(aacs, buf, len, offset, curr_uk++);
    }

    X_FREE(tmp_buf);

    return 0;
}

AACS_KEYS *aacs_open(const char *path, const char *configfile_path)
{
    DEBUG(DBG_AACS, "libaacs v%s [%ld]\n", LIBAACS_VERSION, sizeof(AACS_KEYS));

    AACS_KEYS *aacs = malloc(sizeof(AACS_KEYS));

    aacs->uks = NULL;
    aacs->kf = NULL;
    if ((aacs->kf = configfile_open(configfile_path))) {
        DEBUG(DBG_AACS, "Searching for VUK...\n");
        if(_find_vuk(aacs, path)) {
            if (_calc_uks(aacs, path)) {
                configfile_close(aacs->kf);

                DEBUG(DBG_AACS, "AACS initialized! (0x%08x)\n", aacs);
                return aacs;
            }
        }

        DEBUG(DBG_AACS, "Starting AACS waterfall...\n");
        //_calc_pk(aacs);
        if (_calc_mk(aacs, path)) {
           if (_calc_vuk(aacs, path)) {
                if (_calc_uks(aacs, path)) {
                    configfile_close(aacs->kf);

                    DEBUG(DBG_AACS, "AACS initialized! (0x%08x)\n", aacs);
                    return aacs;
                }
            }
        }
    }

    DEBUG(DBG_AACS, "Failed to initialize AACS! (0x%08x)\n", aacs);

    return NULL;
}

void aacs_close(AACS_KEYS *aacs)
{
    X_FREE(aacs->uks);

    DEBUG(DBG_AACS, "AACS destroyed! (0x%08x)\n", aacs);

    X_FREE(aacs);
}

int aacs_decrypt_unit(AACS_KEYS *aacs, uint8_t *buf, uint32_t len, uint64_t offset)
{
    return _decrypt_unit(aacs, buf, len, offset, 0);
}
