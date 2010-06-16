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
 * the linked executables.  You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL".  If you
 * modify this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to do
 * so, delete this exception statement from your version.
 */

#ifndef AACS_H_
#define AACS_H_

#include "configfile.h"

typedef struct aacs AACS;
struct aacs {
    uint8_t pk[16], mk[16], vuk[16], vid[16];
    uint8_t *uks;           /* unit key array (size = 16 * num_uks, each key is
                             * at 16-byte offset)
                             */
    uint16_t num_uks;       /* number of unit keys */
    uint8_t iv[16];
    CONFIGFILE *kf;
};

AACS *aacs_open(const char *path, const char *keyfile_path);
void aacs_close(AACS *aacs);
int aacs_decrypt_unit(AACS *aacs, uint8_t *buf, uint32_t len, uint64_t offset);
uint8_t *aacs_get_vid(AACS *aacs);

#endif /* AACS_H_ */
