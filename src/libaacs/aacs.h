/*
 * libaacs by Doom9 ppl 2009
 * The aim of libaacs is to achieve a complete open-source implementation of the AACS spec
 * The spec isfreely available at http://www.aacsla.com/specifications/
 */

#ifndef AACS_H_
#define AACS_H_

#include <stdint.h>
#include <unistd.h>

#include "mkb.h"

typedef struct aacs AACS;
struct aacs {
            *uks,           // unit key array (size = 16 * num_uks, each key is at 16-byte offset)
            iv[16];         // current decryption iv
    uint16_t num_uks;       // number of unit keys
};

AACS *aacs_open(const char *path);
void aacs_close(AACS *aacs);

#endif /* AACS_H_ */
