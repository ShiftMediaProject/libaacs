
#include <string.h>
#include <openssl/aes.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>

#include "crypto.h"

void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc);

void _aesg3(const uint8_t *src_key, uint8_t *dst_key, uint8_t inc)
{
    int a;
    AES_KEY aes;
    uint8_t seed[16] = { 0x7B, 0x10, 0x3C, 0x5D, 0xCB, 0x08, 0xC4, 0xE5, 0x1A, 0x27, 0xB0, 0x17, 0x99, 0x05, 0x3B, 0xD9 };

    seed[15] += inc;

    AES_set_decrypt_key(src_key, 128, &aes);
    AES_decrypt(seed, dst_key, &aes);

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

void crypto_aacs_sign(const uint8_t *c, const uint8_t *pubk, uint8_t *sig, uint8_t *n, const uint8_t *dhp)
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

    unsigned char md_value[64];
    unsigned int md_len;

    const char dp[] = "900812823637587646514106462588455890498729007071";
    const char da[] = "-3";
    const char db[] = "366394034647231750324370400222002566844354703832";
    const char dx[] = "264865613959729647018113670854605162895977008838";
    const char dy[] = "51841075954883162510413392745168936296187808697";
    const char dr[] = "900812823637587646514106555566573588779770753047";

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
    EVP_DigestInit(&mdctx, EVP_ecdsa());
    EVP_DigestUpdate(&mdctx, n, 20);
    EVP_DigestUpdate(&mdctx, dhp, 40);
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);

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
