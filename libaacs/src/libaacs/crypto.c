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
 *
 * In addition, as a special exception, the copyright holders of libaacs
 * gives permission to link the code of its release of libaacs with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked library.  You must obey the GNU Lesser General Public
 * License in all respects for all of the code used other than "OpenSSL".
 * If you modify this file, you may extend this exception to your
 * version of the file, but you are not obligated to do so.  If you do
 * not wish to do so, delete this exception statement from your version.
 */

#include "crypto.h"

#include <string.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/ecdsa.h>

void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc)
{
    int a;
    uint8_t seed[16] = { 0x7B, 0x10, 0x3C, 0x5D, 0xCB, 0x08, 0xC4, 0xE5,
                         0x1A, 0x27, 0xB0, 0x17, 0x99, 0x05, 0x3B, 0xD9 };
    seed[15] += inc;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_set_key_length(ctx, 16);
    EVP_DecryptInit(ctx, EVP_aes_128_ecb(), src_key, NULL);
    EVP_DecryptUpdate(ctx, dst_key, (int*)16, seed, 16);
    EVP_DecryptFinal(ctx, dst_key, (int*)16);
    EVP_CIPHER_CTX_cleanup(ctx);

    for (a = 0; a < 16; a++) {
        dst_key[a] ^= seed[a];
    }
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
    EC_GROUP *grp;
    const EC_GROUP *pkgrp;
    EC_POINT *p;
    EC_POINT *pk;
    EC_KEY *k;
    BIGNUM *bdp, *bda, *bdb, *bdx, *bdy, *bdr;
    BIGNUM *x, *y;
    BIGNUM *bpk;
    BN_CTX *ctx;
    EVP_MD_CTX mdctx;
    ECDSA_SIG *s;

    unsigned char md_value[20];
    unsigned int md_len;

    const char *dp = "900812823637587646514106462588455890498729007071";
    const char *da = "-3";
    const char *db = "366394034647231750324370400222002566844354703832";
    const char *dx = "264865613959729647018113670854605162895977008838";
    const char *dy = "51841075954883162510413392745168936296187808697";
    const char *dr = "900812823637587646514106555566573588779770753047";

    ctx = BN_CTX_new();

    bdp = BN_new();
    bda = BN_new();
    bdb = BN_new();
    bdx = BN_new();
    bdy = BN_new();
    bdr = BN_new();

    BN_dec2bn(&bdp, dp);
    BN_dec2bn(&bda, da);
    BN_dec2bn(&bdb, db);
    BN_dec2bn(&bdx, dx);
    BN_dec2bn(&bdy, dy);
    BN_dec2bn(&bdr, dr);

    grp = EC_GROUP_new_curve_GFp(bdp, bda, bdb, ctx);

    p = EC_POINT_new(grp);

    EC_POINT_set_affine_coordinates_GF2m(grp, p, bdx, bdy, ctx);

    BN_set_word(bdx, 1);

    EC_GROUP_set_generator(grp, p, bdr, bdx);

    k = EC_KEY_new();
    EC_KEY_set_group(k, grp);

    pkgrp = EC_KEY_get0_group(k);
    pk = EC_POINT_new(grp);

    x = BN_bin2bn(c + 12, 20, 0);
    y = BN_bin2bn(c + 32, 20, 0);

    EC_POINT_set_affine_coordinates_GFp(pkgrp, pk, x, y, NULL);

    EC_KEY_set_public_key(k, pk);

    memset(md_value, 0, 64);

    bpk = BN_bin2bn(pubk, 20, 0);

    EC_KEY_set_private_key(k, bpk);

    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit(&mdctx, EVP_sha1());
    EVP_DigestUpdate(&mdctx, n, 20);
    EVP_DigestUpdate(&mdctx, dhp, 40);
    EVP_DigestFinal(&mdctx, md_value, &md_len);

    s = ECDSA_do_sign(md_value, md_len, k);

    BN_bn2bin(s->r, sig);
    BN_bn2bin(s->s, sig + 20);

    ECDSA_SIG_free(s);
    EC_KEY_free(k);
    BN_clear_free(bpk);
    BN_clear_free(x);
    BN_clear_free(y);
    EC_POINT_free(pk);
    EC_POINT_free(p);
    BN_CTX_free(ctx);
    BN_free(bdp);
    BN_free(bda);
    BN_free(bdb);
    BN_free(bdx);
    BN_free(bdy);
    BN_free(bdr);
}

void crypto_aacs_title_hash(const uint8_t *ukf, uint64_t len, uint8_t *hash)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    EVP_DigestInit(ctx, EVP_sha1());
    EVP_DigestUpdate(ctx, ukf, len);
    EVP_DigestFinal(ctx, hash, NULL);
    EVP_MD_CTX_cleanup(ctx);
}
