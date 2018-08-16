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

#ifndef CONTENT_CERT_H_
#define CONTENT_CERT_H_

#include "util/attributes.h"

#include <stdlib.h>
#include <stdint.h>

typedef struct content_cert CONTENT_CERT;

struct content_cert {
    uint8_t bus_encryption_enabled_flag;

    uint8_t cc_id[6];

    /*
     * BD-J root certificte hash
     * aacs1: SHA1   (20 bytes)
     * aacs2: SHA256 (32 bytes)
     */
    uint8_t aacs2; /* 0 - aacs1, 1 - aacs2 */
    uint8_t bdj_root_cert_hash[32];
};


BD_PRIVATE CONTENT_CERT *cc_parse(const void *data, size_t len);
BD_PRIVATE void          cc_free(CONTENT_CERT **);

#endif /* CONTENT_CERT_H_ */
