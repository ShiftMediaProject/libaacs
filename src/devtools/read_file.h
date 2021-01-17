/*
 * This file is part of libaacs
 * Copyright (C) 2020  VideoLAN
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static size_t _read_file(const char *name, long min_size, long max_size, uint8_t **pdata)
{
    FILE    *f;
    uint8_t *data = NULL;
    long     file_size;
    size_t   size;

    f = fopen(name, "rb");
    if (!f) {
        fprintf(stderr, "error opening '%s': %s (%d)\n", name, strerror(errno), errno);
        return 0;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "error seeking to end: %s (%d)\n", strerror(errno), errno);
        goto fail;
    }

    file_size = ftell(f);

    if (file_size < 0) {
        fprintf(stderr, "error getting file size: %s (%d)\n", strerror(errno), errno);
        goto fail;
    }
    if (file_size < min_size || file_size > max_size) {
        fprintf(stderr, "weird file size: %lld\n", (long long)file_size);
        goto fail;
    }
    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "error seeking to start: %s (%d)\n", strerror(errno), errno);
        goto fail;
    }

    size = (size_t)file_size;

    data = malloc(size);
    if (!data) {
        fprintf(stderr, "error allocating %zu bytes: %s (%d)\n", size, strerror(errno), errno);
        goto fail;
    }
    if (fread(data, size, 1, f) != 1) {
        fprintf(stderr, "error reading %zu bytes: %s (%d)\n", size, strerror(errno), errno);
        goto fail;
    }
    fclose(f);

    *pdata = data;
    return size;

 fail:
    free(data);
    fclose(f);
    return 0;
}
