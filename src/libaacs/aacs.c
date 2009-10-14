
#include <stdio.h>
#include <malloc.h>

#include "aacs.h"
#include "crypto.h"
#include "../util/macro.h"

int _calc_pk(uint8_t *key);
int _calc_mk(uint8_t *key);
int _calc_vuk(uint8_t *key, const char *path);
int _calc_uks(AACS *aacs, uint8_t *vuk);
int _validate_pk(uint8_t *pk, uint8_t *cvalue, uint8_t *uv, uint8_t *vd);
int _verify_ts(uint8_t *buf);


AACS *aacs_open(const char *path)
{
    uint8_t key[16];
    AACS *aacs = malloc(sizeof(AACS));

    // perform aacs waterfall
    _calc_pk(key);
    _calc_mk(key);
    _calc_vuk(key, path);
    _calc_uks(aacs, key);

    return aacs;
}

void aacs_close(AACS *aacs)
{
    X_FREE(aacs);
}

int aacs_decrypt_unit(AACS *aacs, uint8_t *buf)
{
    uint8_t seed[16], iv[] = { 0x0b, 0xa0, 0xf8, 0xdd, 0xfe, 0xa6, 0x1f, 0xb3, 0xd8, 0xdf, 0x9f, 0x56, 0x6a, 0x05, 0x0f, 0x78 };

    //AES_set_encrypt_key(bluray->uk, 128, &bluray->aes);
    //AES_encrypt(buf, seed, &bluray->aes);

    int a;
    for (a = 0; a < 16; a++)
        seed[a] ^= buf[a];

    //AES_set_decrypt_key( seed, 128, &bluray->aes );
    //AES_cbc_encrypt( buf + 16, buf + 16, len - 16, &bluray->aes, bluray->iv, 0 );

    return 1;
}
