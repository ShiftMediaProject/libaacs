/*
 * This file is part of libbluray
 * Copyright (C) 2009-2010  gates
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

#include "strutl.h"

#include "macro.h"
#include "logging.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


/* Function to assigns proper hex value of character to uint8_t pointer */
static int _hexstring_to_unsigned_char(uint8_t *value, char c)
{
    *value = 0;

    switch (c)
    {
      case '0':
        break;

      case '1':
        *value = 1;
        break;

      case '2':
        *value = 2;
        break;

      case '3':
        *value = 3;
        break;

      case '4':
        *value = 4;
        break;

      case '5':
        *value = 5;
        break;

      case '6':
        *value = 6;
        break;

      case '7':
        *value = 7;
        break;

      case '8':
        *value = 8;
        break;

      case '9':
        *value = 9;
        break;

      case 'a':
      case 'A':
        *value = 10;
        break;

      case 'b':
      case 'B':
        *value = 11;
        break;

      case 'c':
      case 'C':
        *value = 12;
        break;

      case 'd':
      case 'D':
        *value = 13;
        break;

      case 'e':
      case 'E':
        *value = 14;
        break;

      case 'f':
      case 'F':
        *value = 15;
        break;

      default:
        BD_DEBUG(DBG_AACS, "Invalid hex value '%c'", c);
        return 0;
    }

    return 1;
}

/* Function used to convert hex string into an equivalent hex array of
 * unsigned characters
 */
int hexstring_to_hex_array(uint8_t *hex_array, uint32_t size,
                                  const char *hexstring)
{
  unsigned int i = 0;
  while (i < size)
  {
    uint8_t tmp1 = 0, tmp2 = 0;
    if (!(_hexstring_to_unsigned_char(&tmp1, hexstring[(i*2)])) ||
      !(_hexstring_to_unsigned_char(&tmp2, hexstring[(i*2)+1])))
      return 0;

    hex_array[i] = tmp1 * 16;
    hex_array[i] += tmp2;

    i++;
  }

  return 1;
}

/* Function to convert a hex array into a hex string.
 * str must be allocated by caller
 * size is the size of the hex_array
 */
int hex_array_to_hexstring(char *str, const uint8_t *hex_array, uint32_t size)
{
  unsigned int i;

  for (i = 0; i < size; i++)
  {
    sprintf(str + (i*2), "%02x", hex_array[i]);
  }

  return 1;
}

char * str_dup(const char *str)
{
  return str ? strcpy (malloc(strlen(str) + 1), str) : NULL;
}

char *str_printf(const char *fmt, ...)
{
    /* Guess we need no more than 100 bytes. */
    va_list ap;
    int     len;
    int     size = 100;
    char   *tmp, *str;

    str = malloc(size);
    while (1)
    {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        len = vsnprintf(str, size, fmt, ap);
        va_end(ap);

        /* If that worked, return the string. */
        if (len > -1 && len < size) {
            return str;
        }

        /* Else try again with more space. */
        if (len > -1)    /* glibc 2.1 */
            size = len+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        tmp = realloc(str, size);
        if (tmp == NULL) {
            return str;
        }
        str = tmp;
    }
}

const char *str_next_line(const char *p)
{
    while (*p && *p != '\r' && *p != '\n') {
        p++;
    }
    while (*p && (*p == '\r' || *p == '\n')) {
        p++;
    }

    return p;
}

static int _str_is_hex_string(const char *str, int len)
{
    int ii;

    for (ii = 0; ii < len; ii++) {
        if (!isxdigit(str[ii])) {
            return 0;
        }
    }

    /* check end */
    if (isxdigit(str[ii])) {
        //BD_DEBUG(DBG_AACS, "Invalid hex string (too long): %s", str);
        return 0;
    }

    return 1;
}

static const char *_str_skip_space(const char *p)
{
    while (*p && (*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t')) {
        p++;
    }

    return p;
}

char *str_get_hex_string(const char *p, int n)
{
    p = _str_skip_space(p);

    if (!_str_is_hex_string(p, n)) {
        return NULL;
    }

    char *s = malloc(n + 1);
    memcpy(s, p, n);
    s[n] = 0;

    return s;
}

char *str_print_hex(char *out, const uint8_t *buf, int count)
{
    int zz;
    for (zz = 0; zz < count; zz++) {
        sprintf(out + (zz * 2), "%02x", buf[zz]);
    }

    return out;
}
