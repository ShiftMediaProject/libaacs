/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010       npzacs
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "crypto.h"
#include "util/strutl.h"
#include "util/macro.h"
#include "util/logging.h"

#include <string.h>
#include <stdio.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <gcrypt.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <errno.h>

/* elliptic curve from AACS specs */
#define AACS_EC_p   "9DC9D81355ECCEB560BDB09EF9EAE7C479A7D7DF"
#define AACS_EC_a   "9DC9D81355ECCEB560BDB09EF9EAE7C479A7D7DC"
#define AACS_EC_b   "402DAD3EC1CBCD165248D68E1245E0C4DAACB1D8"
#define AACS_EC_n   "9DC9D81355ECCEB560BDC44F54817B2C7F5AB017"
#define AACS_EC_G_x "2E64FC22578351E6F4CCA7EB81D0A4BDC54CCEC6"
#define AACS_EC_G_y "0914A25DD05442889DB455C7F23C9A0707F5CBB9"

/* Set this in CFLAGS to debug gcrypt MPIs and S-expressions */
#ifndef GCRYPT_DEBUG
#define GCRYPT_DEBUG 0
#endif

/* Use pthread in libgcrypt */
#ifdef HAVE_PTHREAD_H
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif

/* include some elliptic curve utils from libgcrypt */
#include "ec.c"

typedef struct {
    gcry_mpi_t  p, a, b, n;
    mpi_point_t G;
} elliptic_curve_t;

static void _aacs_curve_init(elliptic_curve_t *c)
{
    /* elliptic curve from AACS specs */
    const uint8_t p[20]   = { 0x9D,0xC9,0xD8,0x13,0x55,0xEC,0xCE,0xB5,0x60,0xBD,
                              0xB0,0x9E,0xF9,0xEA,0xE7,0xC4,0x79,0xA7,0xD7,0xDF };
    const uint8_t a[20]   = { 0x9D,0xC9,0xD8,0x13,0x55,0xEC,0xCE,0xB5,0x60,0xBD,
                              0xB0,0x9E,0xF9,0xEA,0xE7,0xC4,0x79,0xA7,0xD7,0xDC };
    const uint8_t b[20]   = { 0x40,0x2D,0xAD,0x3E,0xC1,0xCB,0xCD,0x16,0x52,0x48,
                              0xD6,0x8E,0x12,0x45,0xE0,0xC4,0xDA,0xAC,0xB1,0xD8 };
    const uint8_t n[20]   = { 0x9D,0xC9,0xD8,0x13,0x55,0xEC,0xCE,0xB5,0x60,0xBD,
                              0xC4,0x4F,0x54,0x81,0x7B,0x2C,0x7F,0x5A,0xB0,0x17 };
    const uint8_t G_x[20] = { 0x2E,0x64,0xFC,0x22,0x57,0x83,0x51,0xE6,0xF4,0xCC,
                              0xA7,0xEB,0x81,0xD0,0xA4,0xBD,0xC5,0x4C,0xCE,0xC6 };
    const uint8_t G_y[20] = { 0x09,0x14,0xA2,0x5D,0xD0,0x54,0x42,0x88,0x9D,0xB4,
                              0x55,0xC7,0xF2,0x3C,0x9A,0x07,0x07,0xF5,0xCB,0xB9 };

    memset(c, 0, sizeof(*c));

    gcry_mpi_scan (&c->p,   GCRYMPI_FMT_USG, p,   20, NULL);
    gcry_mpi_scan (&c->a,   GCRYMPI_FMT_USG, a,   20, NULL);
    gcry_mpi_scan (&c->b,   GCRYMPI_FMT_USG, b,   20, NULL);
    gcry_mpi_scan (&c->n,   GCRYMPI_FMT_USG, n,   20, NULL);
    gcry_mpi_scan (&c->G.x, GCRYMPI_FMT_USG, G_x, 20, NULL);
    gcry_mpi_scan (&c->G.y, GCRYMPI_FMT_USG, G_y, 20, NULL);
    c->G.z = mpi_alloc_set_ui(1);
}

static void _curve_free(elliptic_curve_t *c)
{
    gcry_mpi_release(c->p); c->p = NULL;
    gcry_mpi_release(c->a); c->a = NULL;
    gcry_mpi_release(c->b); c->b = NULL;
    gcry_mpi_release(c->n); c->n = NULL;
    point_free(&c->G);
}

static void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc)
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
    static int crypto_init_check = 0;

    if (!crypto_init_check) {
        crypto_init_check = 1;
#ifdef HAVE_PTHREAD_H
        gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
#endif
        if (!gcry_check_version(GCRYPT_VERSION)) {
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

#ifdef HAVE_STRERROR_R
#define LOG_GCRY_ERROR(msg, func, err)                                  \
  char errstr[100] = {0};                                               \
  gpg_strerror_r(err, errstr, sizeof(errstr));                          \
  DEBUG(DBG_AACS|DBG_CRIT, "%s: %s failed. error was: %s\n", func, msg, errstr);
#else
#define LOG_GCRY_ERROR(msg, func, err)                                  \
  DEBUG(DBG_AACS|DBG_CRIT, "%s: %s failed. error was: %s\n", func, msg, gcry_strerror(err));
#endif

#define GCRY_VERIFY(msg, op)                                \
    if ((err = (op))) {                                     \
        LOG_GCRY_ERROR(msg, __PRETTY_FUNCTION__, err);      \
        goto error;                                         \
    }

void crypto_aacs_sign(const uint8_t *cert, const uint8_t *priv_key,
                      uint8_t *signature,
                      const uint8_t *nonce, const uint8_t *point)
{
    gcry_mpi_t mpi_d, mpi_md;
    gcry_sexp_t sexp_key = NULL, sexp_data = NULL, sexp_sig = NULL, sexp_r = NULL, sexp_s = NULL;
    unsigned char Q[41], block[60], md[20], *r = NULL, *s = NULL;
    gcry_error_t err;

    /* Assign MPI values for ECDSA parameters Q and d.
     * Values are:
     *   Q.x = c[12]..c[31]
     *   Q.y = c[32]..c[51]
     *   d = privk
     *
     * Note: The MPI values for Q are in the form "<format>||Q.x||Q.y".
     */
    memcpy(&Q[0], "\x04", 1);   // format
    memcpy(&Q[1], cert + 12, 20);  // Q.x
    memcpy(&Q[21], cert + 32, 20); // Q.y
    gcry_mpi_scan(&mpi_d, GCRYMPI_FMT_USG, priv_key, 20, NULL);

    /* Show the values of the MPIs Q.x, Q.y, and d when debugging */
    if (GCRYPT_DEBUG) {
        gcry_mpi_t mpi_Q_x, mpi_Q_y;
        gcry_mpi_scan(&mpi_Q_x, GCRYMPI_FMT_USG, cert + 12, 20, NULL);
        gcry_mpi_scan(&mpi_Q_y, GCRYMPI_FMT_USG, cert + 32, 20, NULL);
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
    char str_Q[sizeof(Q)*2 + 1];
    hex_array_to_hexstring(str_Q, Q, sizeof(Q));

    char *strfmt = str_printf(
      "(private-key"
      "(ecdsa"
      "(p #"AACS_EC_p"#)"
      "(a #"AACS_EC_a"#)"
      "(b #"AACS_EC_b"#)"
      "(g #04"
          AACS_EC_G_x
          AACS_EC_G_y
          "#)"
      "(n #"AACS_EC_n"#)"
      "(q #%s#)"
      "(d %%m)))",
      str_Q
      );

    /* Now build the S-expression */
    GCRY_VERIFY("gcry_sexp_build",
                gcry_sexp_build(&sexp_key, NULL, strfmt, mpi_d));

    /* Dump information about the key s-expression when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(sexp_key);
    }

    /* Calculate the sha1 hash from the nonce and host key point and covert
     * the hash into an MPI.
     */
    memcpy(&block[0], nonce, 20);
    memcpy(&block[20], point, 40);
    gcry_md_hash_buffer(GCRY_MD_SHA1, md, block, sizeof(block));
    gcry_mpi_scan(&mpi_md, GCRYMPI_FMT_USG, md, sizeof(md), NULL);

    /* Dump information about the md MPI when debugging */
    if (GCRYPT_DEBUG) {
        gcry_mpi_dump(mpi_md);
    }

    /* Build an s-expression for the hash */
    gcry_sexp_build(&sexp_data, NULL,
                    "(data"
                    "  (flags raw)"
                    "  (value %m))",
                    mpi_md
                    );

    /* Dump information about the data s-expression when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(sexp_data);
    }

    /* Sign the hash with the ECDSA key. The resulting s-expression should be
     * in the form:
     * (sig-val
     *   (dsa
     *     (r r-mpi)
     *     (s s-mpi)))
     */
    GCRY_VERIFY("gcry_pk_sign",
                gcry_pk_sign(&sexp_sig, sexp_data, sexp_key));

    /* Dump information about the signature s-expression when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(sexp_sig);
    }

    /* Get the resulting s-expressions for 'r' and 's' */
    sexp_r = gcry_sexp_find_token(sexp_sig, "r", 0);
    sexp_s = gcry_sexp_find_token(sexp_sig, "s", 0);

    /* Dump information about 'r' and 's' values when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(sexp_r);
        gcry_sexp_dump(sexp_s);
    }

    /* Convert the data for 'r' and 's' into unsigned char form */
    r = (unsigned char*)gcry_sexp_nth_string(sexp_r, 1);
    s = (unsigned char*)gcry_sexp_nth_string(sexp_s, 1);

    /* Finally concatenate 'r' and 's' to get the ECDSA signature */
    memcpy(signature, r, 20);
    memcpy(signature + 20, s, 20);

 error:

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

void crypto_create_nonce(uint8_t *buf, size_t len)
{
    gcry_create_nonce(buf, len);
}

void crypto_create_host_key_pair(uint8_t *host_key, uint8_t *host_key_point)
{
    /*
     * AACS spec, section 4.3, steps 23-24
     */

    /* generate random number Hk (host_key) */

    gcry_mpi_t d;
    gcry_randomize(host_key, 20, 1);
    gcry_mpi_scan(&d, GCRYMPI_FMT_USG, host_key, 20, NULL);

    /* init AACS curve */

    elliptic_curve_t ec;
    _aacs_curve_init(&ec);

    /* init ec context */

    mpi_ec_t ctx = _gcry_mpi_ec_init (ec.p, ec.a);

    /* Compute point (Q) */

    mpi_point_t Q;
    point_init (&Q);
    _gcry_mpi_ec_mul_point (&Q, d, &ec.G, ctx);

    /* get affine coordinates (Hv) */

    gcry_mpi_t q_x = mpi_new(0);
    gcry_mpi_t q_y = mpi_new(0);
    _gcry_mpi_ec_get_affine (q_x, q_y, &Q, ctx);

    gcry_mpi_print (GCRYMPI_FMT_USG, host_key_point,      0, NULL, q_x);
    gcry_mpi_print (GCRYMPI_FMT_USG, host_key_point + 20, 0, NULL, q_y);

    /* cleanup */

    _gcry_mpi_ec_free (ctx);
    _curve_free(&ec);

    mpi_free(d);
    mpi_free(q_x);
    mpi_free(q_y);
    point_free(&Q);
}
