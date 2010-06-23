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

#include "crypto.h"
#include "util/strutl.h"
#include "util/macro.h"

#include <string.h>
#include <stdio.h>
#include <gcrypt.h>
#include <pthread.h>
#include <errno.h>

/* Set this in CFLAGS to debug gcrypt MPIs and S-expressions */
#ifndef GCRYPT_DEBUG
#define GCRYPT_DEBUG 0
#endif

/* Use pthread in libgcrypt */
GCRY_THREAD_OPTION_PTHREAD_IMPL;

static int crypto_init_check = 0;

void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc)
{
    int a;
    gcry_cipher_hd_t gcry_h;
    uint8_t seed[16] = { 0x7B, 0x10, 0x3C, 0x5D, 0xCB, 0x08, 0xC4, 0xE5,
                         0x1A, 0x27, 0xB0, 0x17, 0x99, 0x05, 0x3B, 0xD9 };
    seed[15] += inc;

    gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(gcry_h, src_key, 16);
    gcry_cipher_decrypt (gcry_h, dst_key, 16, seed, 16);
    gcry_cipher_close(gcry_h);

    for (a = 0; a < 16; a++) {
        dst_key[a] ^= seed[a];
    }
}

/* Initializes libgcrypt */
int crypto_init()
{
  if (!crypto_init_check)
  {
    crypto_init_check = 1;
    gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
    if (!gcry_check_version(GCRYPT_VERSION))
    {
      crypto_init_check = 0;
    }
  }

  return crypto_init_check;
}

void crypto_aesg3(const uint8_t *D, uint8_t *lsubk, uint8_t* rsubk, uint8_t *pk)
{
    if (lsubk) {
        _aesg3(D, lsubk, 0);
    }

    if (pk) {
        _aesg3(D, pk, 1);
    }

    if (rsubk) {
        _aesg3(D, rsubk, 2);
    }
}

void crypto_aacs_sign(const uint8_t *c, const uint8_t *pubk, uint8_t *sig,
                      uint8_t *n, const uint8_t *dhp)
{
    gcry_mpi_t mpi_d, mpi_md;
    gcry_sexp_t sexp_key, sexp_data, sexp_sig, sexp_r, sexp_s;
    unsigned char Q[41], block[60], md[20], *r = NULL, *s = NULL;
    gpg_error_t err;
    char errstr[100];

    /* Assign MPI values for ECDSA parameters Q and d.
     * Values are:
     *   Q.x = c[12]..c[31]
     *   Q.y = c[32]..c[51]
     *   d = privk
     *
     * Note: The MPI values for Q are in the form "<format>||Q.x||Q.y".
     */
    memcpy(&Q[0], "\x04", 1);   // format
    memcpy(&Q[1], c + 12, 20);  // Q.x
    memcpy(&Q[21], c + 32, 20); // Q.y
    gcry_mpi_scan(&mpi_d, GCRYMPI_FMT_USG, pubk, 20, NULL);

    /* Show the values of the MPIs Q.x, Q.y, and d when debugging */
    if (GCRYPT_DEBUG)
    {
      gcry_mpi_t mpi_Q_x, mpi_Q_y;
      gcry_mpi_scan(&mpi_Q_x, GCRYMPI_FMT_USG, c + 12, 20, NULL);
      gcry_mpi_scan(&mpi_Q_y, GCRYMPI_FMT_USG, c + 32, 20, NULL);
      gcry_mpi_dump(mpi_Q_x);
      printf("\n");
      gcry_mpi_dump(mpi_Q_y);
      printf("\n");
      gcry_mpi_dump(mpi_d);
      printf("\n");
    }

    /* Build the s-expression for the ecdsa private key
     * Constant values are:
     *   p = 900812823637587646514106462588455890498729007071
     *   a = -3
     *   b = 366394034647231750324370400222002566844354703832
     *   G.x = 264865613959729647018113670854605162895977008838
     *   G.y = 51841075954883162510413392745168936296187808697
     *   n = 900812823637587646514106555566573588779770753047
     *
     * Note: Here a = -3 mod p
     */

    /* Points are currently only supported in standard format, so get a
     * hexstring out of Q.
     */
    char str_Q[sizeof(Q)*2];
    hex_array_to_hexstring(str_Q, Q, sizeof(Q));

    char *strfmt = (char*)malloc(
      sizeof("(private-key") +
      sizeof("(ecdsa") +
      sizeof("(p #9DC9D81355ECCEB560BDB09EF9EAE7C479A7D7DF#)") +
      sizeof("(a #9DC9D81355ECCEB560BDB09EF9EAE7C479A7D7DC#)") +
      sizeof("(b #402DAD3EC1CBCD165248D68E1245E0C4DAACB1D8#)") +
      sizeof("(g #04") +
          sizeof("2E64FC22578351E6F4CCA7EB81D0A4BDC54CCEC6") +
          sizeof("0914A25DD05442889DB455C7F23C9A0707F5CBB9") +
          sizeof("#)") +
      sizeof("(n #9DC9D81355ECCEB560BDC44F54817B2C7F5AB017#)") +
      sizeof("(q #") + sizeof(str_Q) + sizeof("#)") +
      sizeof("(d %m)))") + 1);

    sprintf(strfmt,
      "(private-key"
      "(ecdsa"
      "(p #9DC9D81355ECCEB560BDB09EF9EAE7C479A7D7DF#)"
      "(a #9DC9D81355ECCEB560BDB09EF9EAE7C479A7D7DC#)"
      "(b #402DAD3EC1CBCD165248D68E1245E0C4DAACB1D8#)"
      "(g #04"
          "2E64FC22578351E6F4CCA7EB81D0A4BDC54CCEC6"
          "0914A25DD05442889DB455C7F23C9A0707F5CBB9"
          "#)"
      "(n #9DC9D81355ECCEB560BDC44F54817B2C7F5AB017#)"
      "(q #%s#)"
      "(d %%m)))",
      str_Q
      );

    /* Now build the S-expression */
    err = gcry_sexp_build(&sexp_key, NULL, strfmt, mpi_d);

    if (err)
    {
      memset(errstr, 0, sizeof(errstr));
      gpg_strerror_r(err, errstr, sizeof(errstr));
      printf("error was: %s\n", errstr);
      return;
    }

    /* Dump information about the key s-expression when debugging */
    if (GCRYPT_DEBUG)
      gcry_sexp_dump(sexp_key);

    /* Calculate the sha1 hash from the nonce and host key point and covert
     * the hash into an MPI.
     */
    memcpy(&block, n, 20);
    memcpy(&block[20], dhp, 40);
    gcry_md_hash_buffer(GCRY_MD_SHA1, md, block, sizeof(block));
    gcry_mpi_scan(&mpi_md, GCRYMPI_FMT_USG, md, sizeof(md), NULL);

    /* Dump information about the md MPI when debugging */
    if (GCRYPT_DEBUG)
      gcry_mpi_dump(mpi_md);

    /* Build an s-expression for the hash */
    gcry_sexp_build(&sexp_data, NULL,
                    "(data"
                    "  (flags raw)"
                    "  (value %m))",
                    mpi_md
                    );

    /* Dump information about the data s-expression when debugging */
    if (GCRYPT_DEBUG)
      gcry_sexp_dump(sexp_data);

    /* Sign the hash with the ECDSA key. The resulting s-expression should be
     * in the form:
     * (sig-val
     *   (dsa
     *     (r r-mpi)
     *     (s s-mpi)))
     */
    err = gcry_pk_sign(&sexp_sig, sexp_data, sexp_key);

    if (err)
    {
      memset(errstr, 0, sizeof(errstr));
      gpg_strerror_r(err, errstr, sizeof(errstr));
      printf("error was: %s\n", errstr);
      return;
    }

    /* Dump information about the signature s-expression when debugging */
    if (GCRYPT_DEBUG)
      gcry_sexp_dump(sexp_sig);

    /* Get the resulting s-expressions for 'r' and 's' */
    sexp_r = gcry_sexp_find_token(sexp_sig, "r", 0);
    sexp_s = gcry_sexp_find_token(sexp_sig, "s", 0);

    /* Dump information about 'r' and 's' values when debugging */
    if (GCRYPT_DEBUG)
    {
      gcry_sexp_dump(sexp_r);
      gcry_sexp_dump(sexp_s);
    }

    /* Convert the data for 'r' and 's' into unsigned char form */
    r = (unsigned char*)gcry_sexp_nth_string(sexp_r, 1);
    s = (unsigned char*)gcry_sexp_nth_string(sexp_s, 1);

    /* Finally concatenate 'r' and 's' to get the ECDSA signature */
    memcpy(sig, r, 20);
    memcpy(sig + 20, s, 20);

    /* Free allocated memory */
    gcry_mpi_release(mpi_d);
    gcry_mpi_release(mpi_md);
    gcry_sexp_release(sexp_key);
    gcry_sexp_release(sexp_data);
    gcry_sexp_release(sexp_sig);
    gcry_sexp_release(sexp_r);
    gcry_sexp_release(sexp_s);
    gcry_free(r);
    gcry_free(s);
    X_FREE(strfmt);
}

void crypto_aacs_title_hash(const uint8_t *ukf, uint64_t len, uint8_t *hash)
{
    gcry_md_hash_buffer(GCRY_MD_SHA1, hash, ukf, len);
}
