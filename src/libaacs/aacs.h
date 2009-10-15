/*
 * libaacs by Doom9 ppl 2009
 * The aim of libaacs is to achieve a complete open-source implementation of the AACS spec
 * The spec is freely available at http://www.aacsla.com/specifications/
 */

#ifndef AACS_H_
#define AACS_H_

#include <stdint.h>
#include <unistd.h>

#include "mkb.h"

enum aacs_key_type {
    PK_ARRAY,
    HOST_PRIV_KEY,
    HOST_CERT,
    HOST_NONCE,
    HOST_KEY_POINT
};

typedef struct aacs_keys AACS_KEYS;
struct aacs_keys {
    uint8_t *pk, *mk, *vuk;
    uint8_t *pks,           // processing key array (size = 16 * num_pks, each key is at 16-byte offset)
            *uks;           // unit key array (size = 16 * num_uks, each key is at 16-byte offset)
    uint16_t num_uks;       // number of unit keys
    uint16_t num_pks;       // number of processing keys
    uint8_t host_priv_key[10], host_cert[92], host_nonce[20], host_key_point[40]; // host keys
};

AACS_KEYS *aacs_open(const char *path, const char *keyfile_path);
void aacs_close(AACS_KEYS *aacs);
int aacs_decrypt_unit(AACS_KEYS *aacs, uint8_t *buf);

#endif /* AACS_H_ */
