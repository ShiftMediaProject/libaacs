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
 */

#include <util/attributes.h>

#ifndef CONFIGFILE_H_
#define CONFIGFILE_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct configfile CONFIGFILE;
struct configfile {
    size_t size;
    uint8_t *buf;
};

enum configfile_types {
    KF_DK_ARRAY = 0x01,
    KF_PK_ARRAY,
    KF_HOST_PRIV_KEY,
    KF_HOST_CERT,
    KF_HOST_NONCE,
    KF_HOST_KEY_POINT,
    KF_VUK_ARRAY
};

CONFIGFILE *configfile_open(const char *path) AACS_PRIVATE;
void configfile_close(CONFIGFILE *kf) AACS_PRIVATE;
uint8_t *configfile_record(CONFIGFILE *kf, enum configfile_types type,
                           uint16_t *entries, size_t *entry_len) AACS_PRIVATE;

/* Keys are stored in a binary file in a record format which is queried from
 * this code
 *
 * Record format:
 *          0                   | type
 *          1-3                 | length
 *          4-5                 | num entries
 *          6-9                 | entry length
 *          10-(9+entry_length) | entry 1
 *          .
 *          .
 *          length-1            | end
 *
 */

#endif /* CONFIGFILE_H_ */
