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

typedef struct aacs AACS;
struct aacs {
    uint8_t *uks;           // unit key array (size = 16 * num_uks, each key is at 16-byte offset)
    uint16_t num_uks;       // number of unit keys
};

AACS *aacs_open(const char *path);
void aacs_close(AACS *aacs);
int aacs_decrypt_unit(AACS *aacs, uint8_t *buf);

#endif /* AACS_H_ */
