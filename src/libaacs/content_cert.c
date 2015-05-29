/*
 * This file is part of libaacs
 * Copyright (C) 2015  npzacs
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

#include "content_cert.h"

#include "crypto.h"

#include "util/logging.h"
#include "util/macro.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

CONTENT_CERT *cc_parse(const void *data, size_t len)
{
    const uint8_t *p = data;

    if (len < 87) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate (length %zd < 87)\n", len);
        return NULL;
    }
    if (p[0] != 0) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate type 0x%02x\n", p[0]);
        return NULL;
    }

    /* calculate certificate length */

    size_t length_format_specific = MKINT_BE16(p + 24);
    size_t num_digest = MKINT_BE16(p + 12);

    size_t cert_data_len = 26 + length_format_specific + num_digest*8;

    if (len < cert_data_len + 40) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate (length %zd < %zd)\n",
                 len, cert_data_len + 40);
        return NULL;
    }

    /* check signature */

    if (!crypto_aacs_verify_aacscc(p + cert_data_len, p, cert_data_len)) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate signature\n");
        return NULL;
    }

    /* return useful data */

    CONTENT_CERT *c = calloc(1, sizeof(CONTENT_CERT));
    if (c) {
        c->bus_encryption_enabled_flag = p[1] >> 7;
        memcpy(c->cc_id,              p + 14, 6);
        memcpy(c->bdj_root_cert_hash, p + 46, 20);
    }
    return c;
}

void cc_free(CONTENT_CERT **pc)
{
    if (pc && *pc) {
        X_FREE(*pc);
    }
}
