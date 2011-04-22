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

#ifndef LOGGING_H_
#define LOGGING_H_

#include "attributes.h"

#include <stdint.h>

#define DEBUG(X,Y,...) aacs_debug(__FILE__,__LINE__,X,Y,##__VA_ARGS__)

enum debug_mask_enum {
    DBG_RESERVED   = 0x0001,
    DBG_CONFIGFILE = 0x0002,
    DBG_FILE       = 0x0004,
    DBG_AACS       = 0x0008,
    DBG_MKB        = 0x0010,
    DBG_MMC        = 0x0020,
  //DBG_BLURAY     = 0x0040,
    DBG_DIR        = 0x0080,
  //DBG_NAV        = 0x0100,
  //DBG_BDPLUS     = 0x0200,
    DBG_DLX        = 0x0400,
    DBG_CRIT       = 0x0800, /* this is the default debug mask so use this if you want to display critical info */
  //DBG_HDMV       = 0x1000,
};

typedef enum debug_mask_enum debug_mask_t;

AACS_PRIVATE char *print_hex(char *out, const uint8_t *str, int count);
AACS_PRIVATE void aacs_debug(const char *file, int line, uint32_t mask, const char *format, ...)
                  AACS_ATTR_FORMAT_PRINTF(4,5);

#endif /* LOGGING_H_ */
