/*
 * This file is part of libaacs
 * Copyright (C) 2010  npzacs
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

#include "libaacs/aacs.h"

#include <stdio.h>
#include <stdlib.h>

static const char *_hex2str(const uint8_t *s, unsigned n)
{
    static const char hex[] = "0123456789ABCDEF";
    static char *str = NULL;

    unsigned ii;

    str = realloc(str, n*2 + 1);
    for (ii = 0; ii < n; ii++) {
        str[2*ii]     = hex[ s[ii] >> 4];
        str[2*ii + 1] = hex[ s[ii] & 0x07];
    }
    str[ii] = 0;

    return str;
}

int main (int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path-to-disc-root> [<path-to-config-file>]\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    AACS *aacs = aacs_open(argv[1], argc > 2 ? argv[2] : NULL);

    if (!aacs) {
        fprintf(stderr, "libaacs open failed.\n");
	exit(EXIT_FAILURE);
    }

    printf("libaacs open succeed.\n");

    const uint8_t *vid = aacs_get_vid(aacs);
    printf("VID: %s\n", vid ? _hex2str(vid, 20) : "???");

    aacs_close(aacs);

    return EXIT_SUCCESS;
}
