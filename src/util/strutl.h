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

BD_PRIVATE int hexstring_to_hex_array(uint8_t *hex_array, uint32_t size,
                                        const char *hexstring);
BD_PRIVATE void hex_array_to_hexstring(char *str, const uint8_t *hex_array, uint32_t size);

BD_PRIVATE char * str_dup(const char *str) BD_ATTR_MALLOC;
BD_PRIVATE char * str_printf(const char *fmt, ...) BD_ATTR_FORMAT_PRINTF(1,2) BD_ATTR_MALLOC;

BD_PRIVATE const char * str_next_line     (const char *str);
BD_PRIVATE char *       str_get_hex_string(const char *str, int len) BD_ATTR_MALLOC;

BD_PRIVATE char * str_print_hex(char *out, const uint8_t *str, int count);

#endif // STRUTL_H_
