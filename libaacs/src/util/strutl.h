/*
 * This file is part of libbluray
 * Copyright (C) 2009-2010  gates
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

#ifndef STRUTL_H_
#define STRUTL_H_

#include "attributes.h"

#include <stdint.h>

int hexstring_to_hex_array(uint8_t *hex_array, uint32_t size,
                           const char *hexstring) AACS_PRIVATE;
int hex_array_to_hexstring(char *str, const uint8_t *hex_array, uint32_t size) AACS_PRIVATE;

#endif // STRUTL_H_
