/*
 * libaacs by Doom9 ppl 2009, 2010
 */

#ifndef AACS_H_
#define AACS_H_

#include <stdint.h>
#include <unistd.h>

#include "mkb.h"
#include "../file/configfile.h"

#define LIBAACS_VERSION "1.0"

typedef struct aacs AACS;
struct aacs {
    uint8_t pk[16], mk[16], vuk[16], vid[16];
    uint8_t *uks;           // unit key array (size = 16 * num_uks, each key is at 16-byte offset)
    uint16_t num_uks;       // number of unit keys
    uint8_t iv[16];
    CONFIGFILE *kf;
};

AACS *aacs_open(const char *path, const char *keyfile_path);
void aacs_close(AACS *aacs);
int aacs_decrypt_unit(AACS *aacs, uint8_t *buf, uint32_t len, uint64_t offset);
uint8_t *aacs_get_vid(AACS *aacs);

#endif /* AACS_H_ */
