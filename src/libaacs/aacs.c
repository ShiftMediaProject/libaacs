
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <openssl/aes.h>

#include "aacs.h"
#include "crypto.h"
#include "../util/macro.h"

int _calc_pk(uint8_t *key);
int _calc_mk(uint8_t *key);
int _calc_vuk(uint8_t *key, const char *path);
int _calc_uks(AACS *aacs, uint8_t *vuk, const char *path);
int _validate_pk(uint8_t *pk, uint8_t *cvalue, uint8_t *uv, uint8_t *vd, uint8_t *mk);
int _verify_ts(uint8_t *buf);



int _calc_vuk(uint8_t *key, const char *path)
{
    int a;
    AES_KEY aes;
    uint8_t vid[16];
    /*MMC* mmc = NULL;

    if ((mmc = mmc_open(path))) {
        if (mmc_read_vid(mmc, vid)) {*/
            AES_set_decrypt_key(key, 128, &aes);
            AES_decrypt(vid, key, &aes);

            for (a = 0; a < 16; a++) {
                key[a] ^= vid[a];
            }
/*
            mmc_close(drive);

            return 1;
        }

        mmc_close(drive);
    }*/

    return 0;
}

int _calc_uks(AACS *aacs, uint8_t *vuk, const char *path)
{
    AES_KEY aes;
    FILE *fp = NULL;
    unsigned char buf[16];
    char f_name[100];
    off_t f_pos;

    snprintf(f_name, 100, "/%s/AACS/Unit_Key_RO.inf", path);

    if ((fp = fopen(f_name, "rb"))) {
        fread(buf, 1, 4, fp);

        f_pos = MKINT_BE32(buf) + 48;

        fseek(fp, f_pos, SEEK_SET);
        fread(buf, 1, 16, fp);

        AES_set_decrypt_key(vuk, 128, &aes);
        AES_decrypt(buf, aacs->uks, &aes);

        fclose(fp);

        return 1;
    }

    return 0;
}

int _validate_pk(uint8_t *pk, uint8_t *cvalue, uint8_t *uv, uint8_t *vd, uint8_t *mk)
{
    int a;
    AES_KEY aes;
    uint8_t dec_vd[16];

    AES_set_decrypt_key(pk, 128, &aes);
    AES_decrypt(cvalue, mk, &aes);

    for (a = 0; a < 4; a++) {
        mk[a + 12] ^= uv[a];
    }

    AES_set_decrypt_key(mk, 128, &aes);
    AES_decrypt(vd, dec_vd, &aes);

    if (!memcmp(dec_vd, "\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8)) {
        return 1;
    }

    return 0;
}

AACS *aacs_open(const char *path)
{
    uint8_t key[16];
    AACS *aacs = malloc(sizeof(AACS));

    // perform aacs waterfall
    _calc_pk(key);
    _calc_mk(key);
    _calc_vuk(key, path);
    _calc_uks(aacs, key, path);

    return aacs;
}

void aacs_close(AACS *aacs)
{
    X_FREE(aacs);
}

int aacs_decrypt_unit(AACS *aacs, uint8_t *buf)
{
    int a;
    AES_KEY aes;
    uint8_t seed[16], iv[] = { 0x0b, 0xa0, 0xf8, 0xdd, 0xfe, 0xa6, 0x1f, 0xb3, 0xd8, 0xdf, 0x9f, 0x56, 0x6a, 0x05, 0x0f, 0x78 };

    AES_set_encrypt_key(aacs->uks, 128, &aes);
    AES_encrypt(buf, seed, &aes);

    for (a = 0; a < 16; a++) {
        seed[a] ^= buf[a];
    }

    AES_set_decrypt_key( seed, 128, &aes );
    AES_cbc_encrypt(buf + 16,buf + 16, 6144 - 16, &aes, iv, 0);

    return 1;
}
