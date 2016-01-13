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

#ifndef AACS_CCI_H_
#define AACS_CCI_H_

#include "util/attributes.h"

#include <stddef.h>

typedef struct aacs_cci AACS_CCI;

BD_PRIVATE AACS_CCI *cci_parse(const void *data, size_t len);
BD_PRIVATE void      cci_free(AACS_CCI **);

BD_PRIVATE int cci_is_unencrypted(AACS_CCI *cci);

BD_PRIVATE struct aacs_basic_cci *cci_get_basic_cci(AACS_CCI *cci);

#endif /* AACS_CCI_H_ */
