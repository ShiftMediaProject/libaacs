
#include <openssl/aes.h>

#include "crypto.h"

void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc);

void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc)
{
    int a;
    AES_KEY aes;
    uint8_t seed[16] = { 0x7B, 0x10, 0x3C, 0x5D, 0xCB, 0x08, 0xC4, 0xE5, 0x1A, 0x27, 0xB0, 0x17, 0x99, 0x05, 0x3B, 0xD9 };

    seed[15] += inc;

    AES_set_decrypt_key(src_key, 128, &aes);
    AES_decrypt(seed, dst_key, &aes);

    for (a = 0; a < 16; a++)
        dst_key[a] ^= seed[a];
}

void crypto_aesg3(const uint8_t *D, uint8_t *lsubk, uint8_t* rsubk, uint8_t *pk)
{
    if (lsubk)
        _aesg3(D, lsubk, 0);

    if (pk)
        _aesg3(D, pk, 1);

    if (rsubk)
        _aesg3(D, rsubk, 2);
}
