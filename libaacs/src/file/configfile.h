/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  cRTrn13
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

CONFIGFILE *configfile_open(const char *path);
void configfile_close(CONFIGFILE *kf);
uint8_t *configfile_record(CONFIGFILE *kf, enum configfile_types type,
                           uint16_t *entries, size_t *entry_len);

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
