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

#ifndef AACS_H_
#define AACS_H_

#include <stdint.h>

typedef struct aacs AACS;

AACS *aacs_open(const char *path, const char *keyfile_path);
void aacs_close(AACS *aacs);
int aacs_decrypt_unit(AACS *aacs, uint8_t *buf, uint32_t len, uint64_t offset);
uint8_t *aacs_get_vid(AACS *aacs);

#endif /* AACS_H_ */
