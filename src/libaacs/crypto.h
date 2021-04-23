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
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#include "util/attributes.h"

#include <stdlib.h>
#include <stdint.h>

BD_PRIVATE int  crypto_init(void);
BD_PRIVATE void crypto_aes128d(const uint8_t *key, const uint8_t *data, uint8_t *dst);
BD_PRIVATE void crypto_aesg3(const uint8_t *D, uint8_t *lsubk, uint8_t* rsubk,
                             uint8_t *pk);   // returns left, centre, right keys
BD_PRIVATE void crypto_aes_cmac_16(const unsigned char *data, const unsigned char *aes_key, unsigned char *cmac);

BD_PRIVATE void crypto_aacs_decrypt(const uint8_t *key, uint8_t *out, size_t out_size, const uint8_t *in, size_t in_size);

BD_PRIVATE void crypto_aacs_sign(const uint8_t *cert, const uint8_t *priv_key,
                                 uint8_t *signature,
                                 const uint8_t *nonce, const uint8_t *point);
BD_PRIVATE void crypto_aacs_title_hash(const uint8_t *ukf, uint64_t len, uint8_t *hash);

BD_PRIVATE int  crypto_aacs_verify(const uint8_t *cert, const uint8_t *signature, const uint8_t *data, uint32_t len);
BD_PRIVATE int  crypto_aacs_verify_aacsla(const uint8_t *signature, const uint8_t *data, uint32_t len);
BD_PRIVATE int  crypto_aacs_verify_aacscc(const uint8_t *signature, const uint8_t *data, uint32_t len);
BD_PRIVATE int  crypto_aacs_verify_host_cert(const uint8_t *cert);
BD_PRIVATE int  crypto_aacs_verify_drive_cert(const uint8_t *cert);

BD_PRIVATE void crypto_create_host_key_pair(uint8_t *key, uint8_t *key_point);
BD_PRIVATE void crypto_create_nonce(uint8_t *buf, size_t len);
BD_PRIVATE void crypto_create_bus_key(const uint8_t *priv_key, const uint8_t *drive_key_point, unsigned char *bus_key);

#endif /* CRYPTO_H_ */
