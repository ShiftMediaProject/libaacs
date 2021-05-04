/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010-2013  npzacs
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
#if GCRYPT_VERSION_NUMBER < 0x010600
#include <pthread.h>
#include <errno.h>
#endif
#endif

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
# if GCRYPT_VERSION_NUMBER < 0x010600
GCRY_THREAD_OPTION_PTHREAD_IMPL;
# endif
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

BD_USED static int _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc)
{
    int a, err;
    uint8_t seed[16] = { 0x7B, 0x10, 0x3C, 0x5D, 0xCB, 0x08, 0xC4, 0xE5,
                         0x1A, 0x27, 0xB0, 0x17, 0x99, 0x05, 0x3B, 0xD9 };
    seed[15] += inc;

    err = crypto_aes128d(src_key, seed, dst_key);
    if (err)
        return err;

    for (a = 0; a < 16; a++) {
        dst_key[a] ^= seed[a];
    }

    return err;
}

/* Initializes libgcrypt */
int crypto_init()
{
    static int crypto_init_check = 0;

    if (!crypto_init_check) {
        crypto_init_check = 1;
#ifdef HAVE_PTHREAD_H
# if GCRYPT_VERSION_NUMBER < 0x010600
        gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
# endif
#endif
        if (!gcry_check_version(GCRYPT_VERSION)) {
            crypto_init_check = 0;
        }
        if(GCRYPT_DEBUG) {
            gcry_control(GCRYCTL_SET_DEBUG_FLAGS, 3);
        }
    }

    return crypto_init_check;
}

int crypto_aes128e(const uint8_t *key, const uint8_t *data, uint8_t *dst)
{
    gcry_cipher_hd_t gcry_h;
    gcry_error_t err;

    err = gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    if (err)
        return err;

    err = gcry_cipher_setkey(gcry_h, key, 16);
    if (err)
        goto error;
    err = gcry_cipher_encrypt(gcry_h, dst, 16, data, data ? 16 : 0);

 error:
    gcry_cipher_close(gcry_h);
    return err;
}

int crypto_aes128d(const uint8_t *key, const uint8_t *data, uint8_t *dst)
{
    gcry_cipher_hd_t gcry_h;
    gcry_error_t err;

    err = gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, 0);
    if (err)
        return err;

    err = gcry_cipher_setkey(gcry_h, key, 16);
    if (err)
        goto error;
    err = gcry_cipher_decrypt(gcry_h, dst, 16, data, 16);

 error:
    gcry_cipher_close(gcry_h);
    return err;
}

int crypto_aesg3(const uint8_t *D, uint8_t *lsubk, uint8_t* rsubk, uint8_t *pk)
{
    int err1 = 0, err2 = 0, err3 = 0;

    if (lsubk) {
        err1 = _aesg3(D, lsubk, 0);
    }

    if (pk) {
        err2 = _aesg3(D, pk, 1);
    }

    if (rsubk) {
        err3 = _aesg3(D, rsubk, 2);
    }

    return err1 ? err1 : err2 ? err2 : err3;
}

/*
 * AES CMAC
 */

static void _shl_128(unsigned char *dst, const unsigned char *src)
{
    uint8_t overflow = 0;
    int i;

    for (i = 15; i >= 0; i--) {
        dst[i] = (src[i] << 1) | overflow;
        overflow = src[i] >> 7;
    }
}

BD_USED static int _cmac_key(const unsigned char *aes_key, unsigned char *k1, unsigned char *k2)
{
    uint8_t key[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int err;

    err = crypto_aes128e(aes_key, NULL, key);
    if (err)
        return err;

    _shl_128(k1, key);
    if (key[0] & 0x80) {
        k1[15] ^= 0x87;
    }

    _shl_128(k2, k1);
    if (k1[0] & 0x80) {
        k2[15] ^= 0x87;
    }

    return err;
}

int crypto_aes_cmac_16(const unsigned char *data, const unsigned char *aes_key, unsigned char *cmac)
{
    uint8_t k1[16], k2[16];
    unsigned ii;
    int err;

    /*
     * Simplified version of AES CMAC. Spports only 16-byte input data.
     */

    /* generate CMAC keys */

    err = _cmac_key(aes_key, k1, k2);
    if (err)
        return err;

    memcpy(cmac, data, 16);
    for (ii = 0; ii < 16; ii++) {
        cmac[ii] ^= k1[ii];
    }

    err = crypto_aes128e(aes_key, NULL, cmac);

    return err;
}

/*
 *
 */

int crypto_aacs_decrypt(const uint8_t *key, uint8_t *out, size_t out_size, const uint8_t *in, size_t in_size)
{
    static const uint8_t aacs_iv[16]   = { 0x0b, 0xa0, 0xf8, 0xdd, 0xfe, 0xa6, 0x1f, 0xb3,
                                           0xd8, 0xdf, 0x9f, 0x56, 0x6a, 0x05, 0x0f, 0x78 };
    gcry_cipher_hd_t gcry_h;
    gcry_error_t err;

    err = gcry_cipher_open(&gcry_h, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_CBC, 0);
    if (err)
        return err;

    err = gcry_cipher_setkey(gcry_h, key, 16);
    if (err)
      goto error;
    err = gcry_cipher_setiv(gcry_h, aacs_iv, 16);
    if (err)
      goto error;
    err = gcry_cipher_decrypt(gcry_h, out, out_size, in, in_size);

 error:
    gcry_cipher_close(gcry_h);
    return err;
}

/*
 *
 */

void crypto_strerror(int err, char *buf, size_t buf_size)
{
#if defined(HAVE_STRERROR_R) && defined(HAVE_LIBGPG_ERROR)
  buf[0] = 0;
  gpg_strerror_r(err, buf, buf_size);
#else
  const char *msg = gcry_strerror(err);
  buf[0] = 0;
  if (msg) {
    strncpy(buf, msg, buf_size);
    buf[buf_size - 1] = 0;
  }
#endif
}

/*
 *
 */

#if defined(HAVE_STRERROR_R) && defined(HAVE_LIBGPG_ERROR)
#define LOG_GCRY_ERROR(msg, func, err)                                  \
  char errstr[100] = {0};                                               \
  gpg_strerror_r(err, errstr, sizeof(errstr));                          \
  BD_DEBUG(DBG_AACS|DBG_CRIT, "%s: %s failed. error was: %s\n", func, msg, errstr);
#else
#define LOG_GCRY_ERROR(msg, func, err)                                  \
  BD_DEBUG(DBG_AACS|DBG_CRIT, "%s: %s failed. error was: %s\n", func, msg, gcry_strerror(err));
#endif

#define GCRY_VERIFY(msg, op)                                \
    if ((err = (op))) {                                     \
        LOG_GCRY_ERROR(msg, __PRETTY_FUNCTION__, err);      \
        goto error;                                         \
    }

/*
 * build S-expressions
 */

static const char *_aacs1_curve(void)
{
  return
    "(p #00"AACS_EC_p"#)"
    "(a #00"AACS_EC_a"#)"
    "(b #00"AACS_EC_b"#)"
    "(g #04"
        AACS_EC_G_x
        AACS_EC_G_y
    "#)"
    "(n #00"AACS_EC_n"#)";
}

static const char *_aacs2_curve(void)
{
  return "(curve \"NIST P-256\")";
}

static gcry_error_t _aacs_sexp_key(gcry_sexp_t *p_sexp_key,
                                   const uint8_t *q_x, const uint8_t *q_y,
                                   const uint8_t *priv_key,
                                   const char *curve,
                                   size_t key_len)
{
    gcry_mpi_t    mpi_d = NULL;
    unsigned char Q[65];
    char          str_Q[sizeof(Q) * 2 + 1];
    gcry_error_t  err;

    BD_ASSERT (key_len == 20 || key_len == 32);

    /* Assign MPI values for ECDSA parameters Q and d.
     * Values are:
     *   Q.x = c[12]..c[31]
     *   Q.y = c[32]..c[51]
     *   d = priv_key
     *
     * Note: The MPI values for Q are in the form "<format>||Q.x||Q.y".
     */
    memcpy(&Q[0],  "\x04", 1); // format
    memcpy(&Q[1],  q_x, key_len);   // Q.x
    memcpy(&Q[1 + key_len], q_y, key_len);   // Q.y
    if (priv_key) {
        gcry_mpi_scan(&mpi_d, GCRYMPI_FMT_USG, priv_key, key_len, NULL);
    }

    /* Show the values of the MPIs Q.x, Q.y, and d when debugging */
    if (GCRYPT_DEBUG) {
        gcry_mpi_t mpi_Q_x, mpi_Q_y;
        gcry_mpi_scan(&mpi_Q_x, GCRYMPI_FMT_USG, q_x, key_len, NULL);
        gcry_mpi_scan(&mpi_Q_y, GCRYMPI_FMT_USG, q_y, key_len, NULL);
        gcry_mpi_dump(mpi_Q_x);
        printf("\n");
        gcry_mpi_dump(mpi_Q_y);
        printf("\n");
        if (mpi_d) {
            gcry_mpi_dump(mpi_d);
            printf("\n");
        }
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
    str_print_hex(str_Q, Q, 1 + 2*key_len);

    char *strfmt = str_printf(
      "(%s"
      "(ecdsa"
      "%s"
      "(q #%s#)"
      "%s))",
      mpi_d ? "private-key" : "public-key",
      curve,
      str_Q,
      mpi_d ? "(d %m)" : ""
      );

    if (!strfmt) {
        err = GPG_ERR_ENOMEM;
        goto error;
    }

    /* Now build the S-expression */
    GCRY_VERIFY("gcry_sexp_build",
                gcry_sexp_build(p_sexp_key, NULL, strfmt, mpi_d));

    /* Dump information about the key s-expression when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(*p_sexp_key);
    }

error:
    X_FREE(strfmt);

    if (mpi_d) {
        gcry_mpi_release(mpi_d);
    }

    return err;
}

static gcry_error_t _aacs_sexp_hash(gcry_sexp_t *p_sexp_data,
                                    const uint8_t *block, uint32_t len,
                                    enum gcry_md_algos hash_type)
{
    gcry_mpi_t   mpi_md = NULL;
    uint8_t      md[32];
    gcry_error_t err;
    size_t hash_size;

    switch (hash_type) {
    case GCRY_MD_SHA1:
        hash_size = 20;
        break;
    case GCRY_MD_SHA256:
        hash_size = 32;
        break;
    default:
        BD_ASSERT_UNREACHABLE ("unsupported hash algorithm");
        return GPG_ERR_UNKNOWN_ALGORITHM;
    }

    gcry_md_hash_buffer(hash_type, md, block, len);
    gcry_mpi_scan(&mpi_md, GCRYMPI_FMT_USG, md, hash_size, NULL);

    /* Dump information about the md MPI when debugging */
    if (GCRYPT_DEBUG) {
        fprintf(stderr, "SHA1: ");
        gcry_mpi_dump(mpi_md);
        fprintf(stderr, "\n");
    }

    /* Build an s-expression for the hash */
    GCRY_VERIFY("gcry_sexp_build",
                gcry_sexp_build(p_sexp_data, NULL,
                                "(data"
#if defined(GCRYPT_VERSION_NUMBER) && GCRYPT_VERSION_NUMBER >= 0x010600
                                /*
                                 * For some reason gcrypt 1.6.0
                                 * requires 'param' flag here and not
                                 * in key, probably a bug.
                                 */
                                "  (flags raw param)"
#else
                                "  (flags raw)"
#endif
                                "  (value %m))",
                                mpi_md
                                ));

    /* Dump information about the data s-expression when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(*p_sexp_data);
    }

 error:

    gcry_mpi_release(mpi_md);

    return err;
}

static gcry_error_t _aacs_sexp_signature(gcry_sexp_t *p_sexp_sign,
                                         const uint8_t *signature,
                                         size_t key_len)
{
    gcry_mpi_t   mpi_r = NULL;
    gcry_mpi_t   mpi_s = NULL;
    gcry_error_t err;

    gcry_mpi_scan(&mpi_r, GCRYMPI_FMT_USG, signature,           key_len, NULL);
    gcry_mpi_scan(&mpi_s, GCRYMPI_FMT_USG, signature + key_len, key_len, NULL);

    /* Dump information about the md MPI when debugging */
    if (GCRYPT_DEBUG) {
        fprintf(stderr, "signature: ");
        gcry_mpi_dump(mpi_r);
        gcry_mpi_dump(mpi_s);
        fprintf(stderr, "\n");
    }

    /* Build an s-expression for the signature */
    GCRY_VERIFY("gcry_sexp_build",
                gcry_sexp_build(p_sexp_sign, NULL,
                               "(sig-val"
                               "  (ecdsa"
                               "    (r %m) (s %m)))",
                               mpi_r, mpi_s));

    /* Dump information about the data s-expression when debugging */
    if (GCRYPT_DEBUG) {
        gcry_sexp_dump(*p_sexp_sign);
    }

error:

    gcry_mpi_release(mpi_r);
    gcry_mpi_release(mpi_s);

    return err;
}

/*
 *
 */

void crypto_aacs_sign(const uint8_t *cert, const uint8_t *priv_key, uint8_t *signature,
                      const uint8_t *nonce, const uint8_t *point)
{
    gcry_sexp_t sexp_key = NULL, sexp_data = NULL, sexp_sig = NULL, sexp_r = NULL, sexp_s = NULL;
    gcry_mpi_t mpi_r = NULL, mpi_s = NULL;
    unsigned char block[60];
    gcry_error_t err;

    GCRY_VERIFY("_aacs_sexp_key",
                _aacs_sexp_key(&sexp_key, cert + 12, cert + 32, priv_key, _aacs1_curve(), 20));

    /* Calculate the sha1 hash from the nonce and host key point and covert
     * the hash into an MPI.
     */
    memcpy(&block[0], nonce, 20);
    memcpy(&block[20], point, 40);

    GCRY_VERIFY("_aacs_sexp_hash",
                _aacs_sexp_hash(&sexp_data, block, sizeof(block), GCRY_MD_SHA1));

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

    /* Finally concatenate 'r' and 's' to get the ECDSA signature */
    mpi_r = gcry_sexp_nth_mpi (sexp_r, 1, GCRYMPI_FMT_USG);
    mpi_s = gcry_sexp_nth_mpi (sexp_s, 1, GCRYMPI_FMT_USG);
    size_t n;
    gcry_mpi_print (GCRYMPI_FMT_USG, signature,      20, &n, mpi_r);
    if (n < 20) {
        memmove (signature + 20 - n, signature, n);
        memset (signature, 0, 20 - n);
    }
    gcry_mpi_print (GCRYMPI_FMT_USG, signature + 20, 20, &n, mpi_s);
    if (n < 20) {
        memmove (signature + 40 - n, signature + 20, n);
        memset (signature + 20, 0, 20 - n);
    }

 error:

    /* Free allocated memory */
    gcry_sexp_release(sexp_key);
    gcry_sexp_release(sexp_data);
    gcry_sexp_release(sexp_sig);
    gcry_sexp_release(sexp_r);
    gcry_sexp_release(sexp_s);
    gcry_mpi_release(mpi_r);
    gcry_mpi_release(mpi_s);
}

static int _aacs_verify(const uint8_t *signature, enum gcry_md_algos hash_type,
                        const uint8_t *q_x, const uint8_t *q_y,
                        const uint8_t *data, uint32_t len)
{
    gcry_sexp_t  sexp_key  = NULL;
    gcry_sexp_t  sexp_sig  = NULL;
    gcry_sexp_t  sexp_data = NULL;
    gcry_error_t err;
    const char *curve;
    size_t key_len;

    switch (hash_type) {
    case GCRY_MD_SHA1:
        curve = _aacs1_curve();
        key_len = 20;
        break;
    case GCRY_MD_SHA256:
        curve = _aacs2_curve();
        key_len = 32;
        break;
    default:
        BD_ASSERT_UNREACHABLE ("invalid signature size");
        return 0;
    }

    GCRY_VERIFY("_aacs_sexp_key",
                _aacs_sexp_key(&sexp_key, q_x, q_y, NULL, curve, key_len));

    GCRY_VERIFY("_aacs_sexp_hash",
                _aacs_sexp_hash(&sexp_data, data, len, hash_type));

    GCRY_VERIFY("_aacs_sexp_signature",
                _aacs_sexp_signature(&sexp_sig, signature, key_len));

    GCRY_VERIFY("gcry_pk_verify",
                gcry_pk_verify(sexp_sig, sexp_data, sexp_key));

 error:
    gcry_sexp_release(sexp_sig);
    gcry_sexp_release(sexp_data);
    gcry_sexp_release(sexp_key);

    return err;
}

int crypto_aacs_verify(const uint8_t *cert, const uint8_t *signature, const uint8_t *data, uint32_t len)
{
    return !_aacs_verify(signature, GCRY_MD_SHA1, cert + 12, cert + 32, data, len);
}

int  crypto_aacs_verify_aacsla(const uint8_t *signature, const uint8_t *data, uint32_t len)
{
    static const uint8_t aacs_la_pubkey_x[] = {0x63, 0xC2, 0x1D, 0xFF, 0xB2, 0xB2, 0x79, 0x8A, 0x13, 0xB5,
                                               0x8D, 0x61, 0x16, 0x6C, 0x4E, 0x4A, 0xAC, 0x8A, 0x07, 0x72 };
    static const uint8_t aacs_la_pubkey_y[] = {0x13, 0x7E, 0xC6, 0x38, 0x81, 0x8F, 0xD9, 0x8F, 0xA4, 0xC3,
                                               0x0B, 0x99, 0x67, 0x28, 0xBF, 0x4B, 0x91, 0x7F, 0x6A, 0x27 };

    return !_aacs_verify(signature, GCRY_MD_SHA1, aacs_la_pubkey_x, aacs_la_pubkey_y, data, len);
}

int  crypto_aacs_verify_aacscc(const uint8_t *signature, const uint8_t *data, uint32_t len)
{
    static const uint8_t aacs_cc_pubkey_x[] = { 0x78, 0x4C, 0xF5, 0xC3, 0x63, 0x97, 0xA4, 0x39, 0x04, 0x06,
                                                0xA4, 0x9F, 0x78, 0x00, 0xC7, 0x7D, 0xE9, 0x0C, 0xB3, 0x4C };
    static const uint8_t aacs_cc_pubkey_y[] = { 0x00, 0x1D, 0xF3, 0x6B, 0x8F, 0x2E, 0xCF, 0x83, 0xCD, 0xEE,
                                                0x43, 0x8F, 0x7F, 0xD1, 0xF4, 0x80, 0x6F, 0xD2, 0x0D, 0xE7 };
    static const uint8_t aacs2_cc_pubkey_x[] = { 0xE7, 0x0D, 0x49, 0xD2, 0x6F, 0x45, 0xEA, 0xA7, 0x36, 0x93, 0x9D, 0x72, 0x88, 0x2E, 0xD8, 0xFB,
                                                 0xA1, 0x60, 0x70, 0x26, 0x96, 0x39, 0x49, 0x97, 0x04, 0x96, 0xC9, 0x10, 0xEA, 0x5C, 0x9D, 0xC2 };
    static const uint8_t aacs2_cc_pubkey_y[] = { 0xD1, 0xF5, 0x89, 0x7C, 0xEC, 0xB8, 0x44, 0x01, 0x4E, 0x0F, 0xB0, 0x8C, 0xC7, 0x6E, 0x20, 0xE8,
                                                 0x54, 0x5E, 0xCC, 0x27, 0x1E, 0xE4, 0x6C, 0x4A, 0xEF, 0x81, 0xD9, 0x16, 0x9B, 0xF8, 0x41, 0x72 };
    switch (data[0]) {
    case 0x00: /* AACS 1 */
        return !_aacs_verify(signature, GCRY_MD_SHA1, aacs_cc_pubkey_x, aacs_cc_pubkey_y, data, len);
    case 0x10: /* AACS 2 */
        return !_aacs_verify(signature, GCRY_MD_SHA256, aacs2_cc_pubkey_x, aacs2_cc_pubkey_y, data, len);
    default:
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Unknown content certificate type 0x%02x\n", data[0]);
        break;
    }

    return 0;
}

static int _aacs_verify_cert(const uint8_t *cert)
{
    if (MKINT_BE16(cert+2) != 0x5c) {
        BD_DEBUG(DBG_AACS, "Certificate length is invalid (0x%04x), expected 0x005c\n",
              MKINT_BE16(cert+2));
        return 0;
    }

    return crypto_aacs_verify_aacsla(cert + 52, cert, 52);
}

int crypto_aacs_verify_host_cert(const uint8_t *cert)
{
    switch (cert[0]) {
    case 0x02:
        break;
    case 0x12:
        // XXX checking the signature would cause buffer overread (certificate is truncated in config file)
        BD_DEBUG(DBG_AACS | DBG_CRIT, "AACS 2.0 host certificate not supported\n");
        return 0;
    default:
        BD_DEBUG(DBG_AACS, "Host certificate type is invalid (0x%02x)\n", cert[0]);
        return 0;
    }

    if (!_aacs_verify_cert(cert)) {
        BD_DEBUG(DBG_AACS, "Host certificate signature is invalid\n");
        return 0;
    }

    return 1;
}

int crypto_aacs_verify_drive_cert(const uint8_t *cert)
{
    switch (cert[0]) {
    case 0x01:
        break;
    case 0x11:
        BD_DEBUG(DBG_AACS | DBG_CRIT, "WARNING: Drive is using AACS 2.0 certificate\n");
        // XXX checking the signature would cause buffer overread (certificate is truncated at MMC layer)
        return 0;
    default:
        BD_DEBUG(DBG_AACS, "Drive certificate type is invalid (0x%02x)\n", cert[0]);
        return 0;
    }

    if (!_aacs_verify_cert(cert)) {
        BD_DEBUG(DBG_AACS, "Drive certificate signature is invalid\n");
        return 0;
    }

    return 1;
}

void crypto_aacs_title_hash(const uint8_t *ukf, uint64_t len, uint8_t *hash)
{
    gcry_md_hash_buffer(GCRY_MD_SHA1, hash, ukf, len);
}

void crypto_create_nonce(uint8_t *buf, size_t len)
{
    gcry_create_nonce(buf, len);
}

void crypto_create_bus_key(const uint8_t *priv_key, const uint8_t *drive_key_point, unsigned char *bus_key)
{
    /* init AACS curve */

    elliptic_curve_t ec;
    _aacs_curve_init(&ec);

    /* init ec context */

    mpi_ec_t ctx = _gcry_mpi_ec_init (ec.p, ec.a);

    /* parse input data */

    gcry_mpi_t mpi_priv_key = NULL;
    gcry_mpi_scan (&mpi_priv_key, GCRYMPI_FMT_USG, priv_key, 20, NULL);

    mpi_point_t Q;
    point_init (&Q);
    gcry_mpi_scan (&Q.x, GCRYMPI_FMT_USG, drive_key_point,      20, NULL);
    gcry_mpi_scan (&Q.y, GCRYMPI_FMT_USG, drive_key_point + 20, 20, NULL);
    Q.z = mpi_alloc_set_ui(1);

    /* calculate bus key point: multiply drive key point with private key */

    mpi_point_t bus_key_point;
    point_init (&bus_key_point);
    _gcry_mpi_ec_mul_point (&bus_key_point, mpi_priv_key, &Q, ctx);

    /* bus key is lowest 128 bits of bus_key_point x-coordinate */

    /* get affine coordinates (Hv) */
    gcry_mpi_t q_x = mpi_new(0);
    gcry_mpi_t q_y = mpi_new(0);
    _gcry_mpi_ec_get_affine (q_x, q_y, &bus_key_point, ctx);

    /* convert to binary */
    uint8_t q_x_bin[100];
    size_t n = 0;
    gcry_mpi_print (GCRYMPI_FMT_USG, q_x_bin, sizeof(q_x_bin), &n, q_x);

    memcpy(bus_key, q_x_bin + n - 16, 16);

    /* cleanup */

    _gcry_mpi_ec_free (ctx);
    _curve_free(&ec);
    mpi_free(mpi_priv_key);
    point_free(&Q);
    point_free(&bus_key_point);
    mpi_free(q_x);
    mpi_free(q_y);
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

    size_t n;
    gcry_mpi_print (GCRYMPI_FMT_USG, host_key_point,      20, &n, q_x);
    if (n < 20) {
        memmove (host_key_point + 20 - n, host_key_point, n);
        memset (host_key_point, 0, 20 - n);
    }
    gcry_mpi_print (GCRYMPI_FMT_USG, host_key_point + 20, 20, &n, q_y);
    if (n < 20) {
        memmove (host_key_point + 40 - n, host_key_point + 20, n);
        memset (host_key_point + 20, 0, 20 - n);
    }

    /* cleanup */

    _gcry_mpi_ec_free (ctx);
    _curve_free(&ec);

    mpi_free(d);
    mpi_free(q_x);
    mpi_free(q_y);
    point_free(&Q);
}
