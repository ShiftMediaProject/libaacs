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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "file.h"
#include "util/macro.h"
#include "util/logging.h"

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

static void file_close_linux(AACS_FILE_H *file)
{
    if (file) {
        fclose((FILE *)file->internal);

        BD_DEBUG(DBG_FILE, "Closed LINUX file (%p)\n", (void*)file);

        X_FREE(file);
    }
}

static int64_t file_seek_linux(AACS_FILE_H *file, int64_t offset, int32_t origin)
{
    return fseeko((FILE *)file->internal, offset, origin);
}

static int64_t file_tell_linux(AACS_FILE_H *file)
{
    return ftello((FILE *)file->internal);
}

static int64_t file_read_linux(AACS_FILE_H *file, uint8_t *buf, int64_t size)
{
    if (size <= 0 || size >= BD_MAX_SSIZE) {
        BD_DEBUG(DBG_FILE | DBG_CRIT, "Ignoring invalid read of size %"PRId64" (%p)\n", size, (void*)file);
        return 0;
    }

    return fread(buf, 1, size, (FILE *)file->internal);
}

static AACS_FILE_H *file_open_linux(const char* filename, const char *mode)
{
    FILE *fp = NULL;
    AACS_FILE_H *file = calloc(1, sizeof(AACS_FILE_H));

    BD_DEBUG(DBG_FILE, "Opening LINUX file %s... (%p)\n", filename, (void*)file);
    file->close = file_close_linux;
    file->seek = file_seek_linux;
    file->read = file_read_linux;
    file->tell = file_tell_linux;

    if ((fp = fopen(filename, mode))) {
        file->internal = fp;

        return file;
    }

    BD_DEBUG(DBG_FILE, "Error opening file %s ! (%p)\n", filename, (void*)file);

    X_FREE(file);

    return NULL;
}

AACS_FILE_H* (*file_open)(const char* filename, const char *mode) = file_open_linux;
