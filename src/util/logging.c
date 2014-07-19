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

#include "logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

uint32_t debug_mask = (uint32_t)-1; /* set all bits to make sure aacs_debug() is called for initialization */

char *print_hex(char *out, const uint8_t *buf, int count)
{
    int zz;
    for(zz = 0; zz < count; zz++) {
        sprintf(out + (zz * 2), "%02X", buf[zz]);
    }

    return out;
}

void aacs_debug(const char *file, int line, uint32_t mask, const char *format, ...)
{
    static int debug_init = 0;
    static FILE *logfile = NULL;

    // Only call getenv() once.
    if (!debug_init) {
        debug_init = 1;
        logfile = stderr;
        debug_mask = DBG_CRIT;

        char *env = NULL;
        if ((env = getenv("AACS_DEBUG_MASK")))
            debug_mask = strtol(env, NULL, 0);
        else if ((env = getenv("BD_DEBUG_MASK")))
            debug_mask = strtol(env, NULL, 0);

        // Send DEBUG to file?
        if ((env = getenv("AACS_DEBUG_FILE"))) {
            FILE *fp = fopen(env, "wb");
            if (fp) {
                logfile = fp;
                setvbuf(logfile, NULL, _IOLBF, 0);
            } else {
                fprintf(logfile, "%s:%d: Error opening log file %s\n", __FILE__, __LINE__, env);
            }
        }
    }

    if (mask & debug_mask) {
        char buffer[512];
        va_list args;

        va_start(args, format);
        sprintf(buffer, "libaacs: %s:%d: %s", file, line, format);
        vfprintf(logfile, buffer, args);
        va_end(args);

    }
}
