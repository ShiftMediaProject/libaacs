
#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <stdint.h>
#include <unistd.h>

void crypto_aesg3(const uint8_t *D, uint8_t *lsubk, uint8_t* rsubk, uint8_t *pk);   // returns left, centre, right keys
void crypto_aacs_sign(const uint8_t *c, const uint8_t *pubk, uint8_t *sig, uint8_t *n, const uint8_t *dhp);

#endif /* CRYPTO_H_ */
