/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2009-2013  npzacz
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

#ifndef AACS_H_
#define AACS_H_

#include <stdint.h>

#ifndef AACS_PUBLIC
#  define AACS_PUBLIC
#endif

/* aacs_open2() error codes */
#define AACS_SUCCESS               0 /* no errors */
#define AACS_ERROR_CORRUPTED_DISC -1 /* opening or reading of AACS files failed */
#define AACS_ERROR_NO_CONFIG      -2 /* missing config file */
#define AACS_ERROR_NO_PK          -3 /* no matching processing key */
#define AACS_ERROR_NO_CERT        -4 /* no valid certificate */
#define AACS_ERROR_CERT_REVOKED   -5 /* certificate has been revoked */
#define AACS_ERROR_MMC_OPEN       -6 /* MMC open failed (no MMC drive ?) */
#define AACS_ERROR_MMC_FAILURE    -7 /* MMC failed */
#define AACS_ERROR_NO_DK          -8 /* no matching device key */

typedef struct aacs AACS;

AACS_PUBLIC void aacs_get_version(int *major, int *minor, int *micro);

AACS_PUBLIC AACS *aacs_init       (void);
AACS_PUBLIC int   aacs_open_device(AACS *, const char *path, const char *keyfile_path);

AACS_PUBLIC AACS *aacs_open(const char *path, const char *keyfile_path);
AACS_PUBLIC AACS *aacs_open2(const char *path, const char *keyfile_path, int *error_code);

AACS_PUBLIC void aacs_close(AACS *aacs);
AACS_PUBLIC void aacs_select_title(AACS *aacs, uint32_t title); /* 0 - top menu, 0xffff - first play */
AACS_PUBLIC int  aacs_decrypt_unit(AACS *aacs, uint8_t *buf);

/* Disc information */
AACS_PUBLIC int aacs_get_mkb_version(AACS *aacs);
AACS_PUBLIC const uint8_t *aacs_get_disc_id(AACS *aacs);
AACS_PUBLIC const uint8_t *aacs_get_vid(AACS *aacs);  /* may fail even if disc can be decrypted */
AACS_PUBLIC const uint8_t *aacs_get_pmsn(AACS *aacs); /* may fail even if disc can be decrypted */
AACS_PUBLIC const uint8_t *aacs_get_mk(AACS *aacs);   /* may fail even if disc can be decrypted */
AACS_PUBLIC const uint8_t *aacs_get_content_cert_id(AACS *aacs);
AACS_PUBLIC const uint8_t *aacs_get_bdj_root_cert_hash(AACS *aacs);

/* AACS Online */
AACS_PUBLIC const uint8_t *aacs_get_device_binding_id(AACS *aacs);
AACS_PUBLIC const uint8_t *aacs_get_device_nonce(AACS *aacs);

/* revocation lists */
typedef struct {
    uint16_t  range;
    uint8_t   id[6];
} AACS_RL_ENTRY;

AACS_PUBLIC AACS_RL_ENTRY *aacs_get_hrl(int *num_entries, int *mkb_version);
AACS_PUBLIC AACS_RL_ENTRY *aacs_get_drl(int *num_entries, int *mkb_version);
AACS_PUBLIC void           aacs_free_rl(AACS_RL_ENTRY **rl);

/* bus encryption info */

#define AACS_BUS_ENCRYPTION_ENABLED  0x01  /* Bus encryption enabled in the media */
#define AACS_BUS_ENCRYPTION_CAPABLE  0x02  /* Bus encryption capable drive */

AACS_PUBLIC uint32_t aacs_get_bus_encryption(AACS *);

/* Copy Control Information */

struct aacs_basic_cci;

AACS_PUBLIC struct aacs_basic_cci *aacs_get_basic_cci(AACS *, uint32_t title);

#endif /* AACS_H_ */
