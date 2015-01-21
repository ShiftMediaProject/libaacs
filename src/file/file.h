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

#include "filesystem.h"

#include <util/attributes.h>

#include <stdint.h>

#ifdef _WIN32
# define DIR_SEP "\\"
#else
# define DIR_SEP "/"
#endif

/*
 * file access
 */

#define file_close(X) X->close(X)
#define file_seek(X,Y,Z) X->seek(X,Y,Z)
#define file_tell(X) X->tell(X)
#define file_read(X,Y,Z) X->read(X,Y,Z)


AACS_PRIVATE extern AACS_FILE_H *(*file_open)(const char* filename, const char *mode);


#endif /* FILE_H_ */
