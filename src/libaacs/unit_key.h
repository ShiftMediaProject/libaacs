/*
 * This file is part of libaacs
 * Copyright (C) 2016  npzacs
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

#ifndef AACS_UK_H_
#define AACS_UK_H_

#include "util/attributes.h"

#include <stddef.h>
#include <stdint.h>

typedef struct aacs_uk AACS_UK;

typedef struct UNIT_KEY {
    uint8_t key[16];
} UNIT_KEY;

struct aacs_uk {

    /* header */
    uint8_t num_bdmv_dir;
    uint8_t app_type;
    uint8_t use_skb_mkb;

    /* unit keys */
    unsigned   num_uk;
    UNIT_KEY  *uk;     /* decrypted unit keys (NULL if not decrypted yet) */
    UNIT_KEY  *enc_uk; /* encrypted unit keys */

    /* title -> CPS unit mappings */
    /* [0] = first play ; [1] = top menu ; [2] = title 1 ... */
    /* NOTE: may be unset (num_titles = 0). */
    uint16_t   num_titles;
    uint16_t  *title_cps_unit;  /* map title to CPS unit (key index) */
};

BD_PRIVATE AACS_UK *uk_parse(const void *data, size_t len, int aacs2);
BD_PRIVATE void     uk_free(AACS_UK **);

#endif /* AACS_UK_H_ */
