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

/*
 * AACS2 format specific data:
 *
 * XXX this needs to be verified from other sources / discs.
 *
 * offset 26: 34 0-bytes, unknown
 * offset 60: BD-J root certificate hash, SHA256 (32 bytes)
 * offset 92: 34 0-bytes
 * offset 126: num CPS units (uint16)
 * offset 128: .cci file hashes, SHA256 (32 bytes * num_cps_unit)
 */

CONTENT_CERT *cc_parse(const void *data, size_t len)
{
    const uint8_t *p = data;
    size_t signature_size;

    if (len < 26) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate (length %zu < 26)\n", len);
        return NULL;
    }

    switch (p[0]) {
    case 0x00: /* AACS 1 */
        signature_size = 40;
        break;
    case 0x10: /* AACS 2 */
        signature_size = 64;
        break;
    default:
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate type 0x%02x\n", p[0]);
        return NULL;
    }

    /* calculate certificate length */

    size_t length_format_specific = MKINT_BE16(p + 24);
    size_t num_digest = MKINT_BE16(p + 12);

    size_t cert_data_len = 26 + length_format_specific + num_digest*8;
    int crypto_error;

    if (len < cert_data_len + signature_size) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "Invalid content certificate (length %zu < %zu)\n",
                 len, cert_data_len + signature_size);
        return NULL;
    }

    /* check signature */

    crypto_error = crypto_aacs_verify_aacscc(p + cert_data_len, p, cert_data_len);
    if (crypto_error) {
        LOG_CRYPTO_ERROR(DBG_AACS, "content certificate signature verification failed", crypto_error);
        return NULL;
    }

    /* return useful data */

    CONTENT_CERT *c = calloc(1, sizeof(CONTENT_CERT));
    if (c) {
        c->bus_encryption_enabled_flag = p[1] >> 7;
        memcpy(c->cc_id,              p + 14, 6);

        /* aacs1 and aacs2 format specific data is different. */
        if (p[0] == 0) {
            if (length_format_specific >= 40) {
                memcpy(c->bdj_root_cert_hash, p + 46, 20);
            }
        } else {
            c->aacs2 = 1;
            // XXX this needs to be verified from other sources / discs.
            if (length_format_specific >= 66) {
                memcpy(c->bdj_root_cert_hash, p + 60, 32);
            }
        }
    }
    return c;
}

void cc_free(CONTENT_CERT **pc)
{
    X_FREE(*pc);
}
