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

#ifndef FILE_H_
#define FILE_H_

#include "util/attributes.h"

#include "filesystem.h"

#include <stdint.h>

#ifdef _WIN32
# define DIR_SEP "\\"
# define DIR_SEP_CHAR '\\'
#else
# define DIR_SEP "/"
# define DIR_SEP_CHAR '/'
#endif

/*
 * file access
 */

#define file_close(X)    X->close(X)
#define file_seek(X,Y,Z) X->seek(X,Y,Z)
#define file_tell(X)     X->tell(X)

static inline int64_t file_read(AACS_FILE_H *fp, void *buf, int64_t size) {
    return fp->read(fp, buf, size);
}

static inline int64_t file_write(AACS_FILE_H *fp, const void *buf, int64_t size) {
    return fp->write ? fp->write(fp, buf, size) : 0;
}


BD_PRIVATE int64_t file_size(AACS_FILE_H *fp);

BD_PRIVATE extern AACS_FILE_H *(*file_open)(const char* filename, const char *mode);

/*
 * local filesystem
 */

BD_PRIVATE int file_unlink(const char *file);
BD_PRIVATE int file_path_exists(const char *path);
BD_PRIVATE int file_mkdir(const char *dir);
BD_PRIVATE int file_mkdirs(const char *path);

#endif /* FILE_H_ */
