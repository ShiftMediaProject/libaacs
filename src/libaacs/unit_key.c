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

#include "unit_key.h"

#include "util/logging.h"
#include "util/macro.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DBG_UK DBG_AACS


static void _uk_clean(AACS_UK *p)
{
    /* erase sensitive data */
    if (p->uk) {
        memset(p->uk, 0, 16 * p->num_uk);
        X_FREE(p->uk);
    }

    X_FREE(p->enc_uk);
    X_FREE(p->title_cps_unit);
}

void uk_free(AACS_UK **pp)
{
    if (pp && *pp) {
        _uk_clean(*pp);
        X_FREE(*pp);
    }
}

static int _parse_header(AACS_UK *uk, const uint8_t *p, size_t size)
{
    /* parse header */

    if (size < 20) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "unexpected EOF (@20)\n");
        return -1;
    }

    uk->app_type     = p[16];
    uk->num_bdmv_dir = p[17];
    uk->use_skb_mkb  = p[18] >> 7;

    /* check if suported */

    if (uk->num_bdmv_dir < 1) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "No BDMV directories\n");
        return -1;
    }
    if (uk->num_bdmv_dir > 1) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "WARNING: possibly unsupported disc: %d BDMV directories\n",
                 uk->num_bdmv_dir);
    }

    if (uk->app_type != 1) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "WARNING: possibly unsupported disc: application type %d\n",
                 uk->app_type);
    }
    if (uk->use_skb_mkb) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "WARNING: possibly unsupported disc: SKB used\n");
    }

    return 0;
}

static int _assign_titles(AACS_UK *uk, const uint8_t *p, size_t size)
{
    uint16_t first_play, top_menu;
    unsigned i, num_titles;

    /* assign titles */

    BD_DEBUG(DBG_UK, "Assigning CPS units to titles ...\n");

    if (size < 26) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Unexpected EOF (@26)\n");
        return -1;
    }

    first_play = MKINT_BE16(p + 20);
    top_menu   = MKINT_BE16(p + 22);
    num_titles = MKINT_BE16(p + 24);

    BD_DEBUG(DBG_UK, "Title FP : CPS unit %d\n", first_play);
    BD_DEBUG(DBG_UK, "Title TM : CPS unit %d\n", top_menu);

    if (size < 26 + 4 * num_titles) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Unexpected EOF (title map)\n");
        uk->num_titles = 0;
        return -1;
    }

    uk->title_cps_unit = calloc(sizeof(uint16_t), num_titles + 2);
    if (!uk->title_cps_unit) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Out of memory\n");
        return -1;
    }
    uk->num_titles = num_titles;

    uk->title_cps_unit[0] = first_play;
    uk->title_cps_unit[1] = top_menu;

    for (i = 0; i < uk->num_titles; i++) {
        uk->title_cps_unit[i + 2] = MKINT_BE16(p + 26 + 2 + (4 * i));
        BD_DEBUG(DBG_UK, "Title %02d : CPS unit %d\n", i + 1, uk->title_cps_unit[i]);
    }

    /* validate */
    for (i = 0; i < num_titles + 2; i++) {
        if (uk->title_cps_unit[i] < 1 || uk->title_cps_unit[i] > uk->num_uk) {
            BD_DEBUG(DBG_UK | DBG_CRIT, "Invalid CPS unit for title #%d: %d\n", (int) i - 1, uk->title_cps_unit[i]);
            uk->title_cps_unit[i] = 0;
        } else {
            uk->title_cps_unit[i]--; /* number [1...N] --> index [0...N-1] */
        }
    }

    return 0;
}

static int _parse_uks(AACS_UK *uk, const uint8_t *p, size_t size)
{
    uint32_t uk_pos;
    unsigned int i;

    if (size < 4) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Empty unit key file\n");
        return -1;
    }

    /* check key storage area */

    uk_pos = MKINT_BE32(p);

    if (size < uk_pos + 2) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Unexpected EOF (key data missing)\n");
        return -1;
    }

    uk->num_uk = MKINT_BE16(p + uk_pos);
    if (uk->num_uk < 1) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "No unit keys\n");
        return 0;
    }

    if (size < uk_pos + 48 * uk->num_uk + 16) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Unexpected EOF (key data truncated)\n");
        return -1;
    }

    /* alloc storage for keys */

    uk->enc_uk = calloc(uk->num_uk, sizeof(AACS_UK));
    if (!uk->enc_uk) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Out of memory\n");
        return -1;
    }

    BD_DEBUG(DBG_UK, "%d CPS unit keys\n", uk->num_uk);

    /* get encrypted keys */

    for (i = 0; i < uk->num_uk; i++) {
        uk_pos += 48;
        memcpy(uk->enc_uk[i].key, p + uk_pos, 16);
    }

    return 0;
}

static int _parse(AACS_UK *uk, const uint8_t *data, size_t len)
{
    int result;

    if (_parse_header(uk, data, len) < 0) {
        return -1;
    }

    result = _parse_uks(uk, data, len);

    /* not fatal, just speeds up things ... */
    _assign_titles(uk, data, len);

    return result;
}

AACS_UK *uk_parse(const void *data, size_t len)
{
    AACS_UK *uk = calloc(1, sizeof(*uk));

    if (!uk) {
        return NULL;
    }

    if (_parse(uk, data, len) < 0) {
        BD_DEBUG(DBG_UK | DBG_CRIT, "Corrupt unit key file (AACS/Unit_Key_RO.inf)\n");
        X_FREE(uk);
    }

    return uk;
}
