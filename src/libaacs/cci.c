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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "cci.h"

#include "cci_data.h"

#include "util/logging.h"
#include "util/macro.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DBG_CCI DBG_AACS


/* CCI type */
enum {
    cci_AACS_BASIC_CCI            = 0x0101,
    cci_AACS_ENHANCED_TITLE_USAGE = 0x0111,
};

typedef struct {
    uint16_t type;
    uint16_t version;
    uint16_t data_length;
    union {
        AACS_BASIC_CCI basic_cci;
    } u;
} AACS_CCI_ENTRY;

struct aacs_cci {
    unsigned int num_entry;
    AACS_CCI_ENTRY *entry;
};


static void _bitcpy(void *dst, const const void *src, size_t num_bits)
{
    size_t bytes = num_bits >> 3;
    size_t bits  = num_bits & 3;
    if (bytes) {
        memcpy(dst, src, bytes);
    }
    if (bits) {
        uint8_t mask = (((1 << (8 - bits))) - 1) ^ 0xff;
        ((uint8_t *)dst)[bytes] = ((const uint8_t *)src)[bytes] & mask;
    }
}

static int cci_parse_entry(AACS_CCI_ENTRY *e, const uint8_t *data, size_t size)
{
    memset(e, 0, sizeof(AACS_CCI_ENTRY));

    if (size < 6) {
        return -1;
    }

    e->type        = MKINT_BE16(data);
    e->version     = MKINT_BE16(data + 2);
    e->data_length = MKINT_BE16(data + 4);

    if (size + 6 < e->data_length) {
        BD_DEBUG(DBG_CCI, "CCI: not enough data (type 0x%04x)\n", e->type);
        return -1;
    }

    switch (e->type) {
        case cci_AACS_BASIC_CCI:
            if (e->data_length == 0x84) {
                BD_DEBUG(DBG_CCI, "CCI: AACS basic CCI found\n");
                e->u.basic_cci.epn = (data[6] & 0x04) >> 2;
                e->u.basic_cci.cci = data[6] & 0x03;
                e->u.basic_cci.image_constraint = (data[7] & 0x10) >> 4;
                e->u.basic_cci.digital_only     = (data[7] & 0x40) >> 3;
                e->u.basic_cci.apstb            = data[7] & 0x07;
                e->u.basic_cci.num_titles       = MKINT_BE16(data + 8);
                if (e->u.basic_cci.num_titles > 1024) {
                    BD_DEBUG(DBG_CCI, "CCI: title type map too large (%u)\n", e->u.basic_cci.num_titles);
                    return -1;
                }
                _bitcpy(e->u.basic_cci.title_type, data + 10, e->u.basic_cci.num_titles);
                return 1;
            }
            break;
        case cci_AACS_ENHANCED_TITLE_USAGE:
            BD_DEBUG(DBG_CCI, "CCI: AACS enhanced title usage CCI found\n");
            return 1;
        default:
            BD_DEBUG(DBG_CCI, "CCI: AACS CCI 0x%04x found\n", e->type);
            break;
    }

    return 0;
}

AACS_CCI *cci_parse(const void *data, size_t size)
{
    AACS_CCI *cci;
    const uint8_t *p = data;
    unsigned int ii;

    if (size < 16) {
        return NULL;
    }

    cci = calloc(1, sizeof(*cci));
    if (!cci) {
        return NULL;
    }

    cci->num_entry = MKINT_BE16(p);
    p += 16;
    size -= 16;

    cci->entry = calloc(cci->num_entry, sizeof(*(cci->entry)));
    if (!cci->entry) {
        cci_free(&cci);
        return NULL;
    }

    BD_DEBUG(DBG_CCI, "CCI: %d entries\n", cci->num_entry);
    for (ii = 0; ii < cci->num_entry; ii++) {
        AACS_CCI_ENTRY *e = &cci->entry[ii];
        if (cci_parse_entry(e, p, size) < 0) {
            BD_DEBUG(DBG_CCI, "CCI parsing failed\n");
            cci_free(&cci);
            return NULL;
        }
        p += 6 + e->data_length;
        size -= 6 + e->data_length;
    }

    return cci;
}

void cci_free(AACS_CCI **pp)
{
    if (pp && *pp) {
        X_FREE((*pp)->entry);
        X_FREE(*pp);
    }
}

int cci_is_unencrypted(AACS_CCI *cci)
{
    unsigned int ii;

    for (ii = 0; ii < cci->num_entry; ii++) {
        AACS_CCI_ENTRY *e = &cci->entry[ii];

        if (e->type == cci_AACS_ENHANCED_TITLE_USAGE) {
            BD_DEBUG(DBG_CCI, "Enhanced title usage CCI found\n");
            return 0;
        }

        if (e->type == cci_AACS_BASIC_CCI) {
            BD_DEBUG(DBG_CCI, "AACS basic CCI found\n");

            /* Blu-ray Disc Pre-recorded Book, chapters 3.9.4.2 and 7.2 */
            if (e->version == 0x0100 && e->data_length == 0x84 &&
                e->u.basic_cci.cci == 0 && e->u.basic_cci.epn == 1 && /* copy freely, EPN unasserted */
                e->u.basic_cci.image_constraint && /* High Definition Analog Output in High Definition Analog Form */
                !e->u.basic_cci.digital_only &&  /* Output of decrypted content is allowed for Analog/Digital Outputs */
                !e->u.basic_cci.apstb  /* APS off */ ) {

                /* check all titles are basic titles */
                int jj;
                for (jj = 0; jj < (e->u.basic_cci.num_titles + 7) / 8; jj++) {
                    if (e->u.basic_cci.title_type[jj]) {
                        BD_DEBUG(DBG_CCI, "CCI: Enhanced title found\n");
                        return 0;
                    }
                }

                return 1;
            }

            return 0;
        }
    }

    return 0;
}

AACS_BASIC_CCI *cci_get_basic_cci(AACS_CCI *cci)
{
    unsigned int ii;

    for (ii = 0; ii < cci->num_entry; ii++) {
        AACS_CCI_ENTRY *e = &cci->entry[ii];
        if (e->type == cci_AACS_BASIC_CCI) {
            return &e->u.basic_cci;
        }
    }
    return NULL;
}
