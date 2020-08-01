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

#ifndef AACS_FILESYSTEM_H_
#define AACS_FILESYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * file access
 */

typedef struct aacs_file_s AACS_FILE_H;
struct aacs_file_s
{
    void* internal;
    void (*close)(AACS_FILE_H *file);
    int64_t (*seek)(AACS_FILE_H *file, int64_t offset, int32_t origin);
    int64_t (*tell)(AACS_FILE_H *file);
    int (*eof)(AACS_FILE_H *file);
    int64_t (*read)(AACS_FILE_H *file, uint8_t *buf, int64_t size);
    int64_t (*write)(AACS_FILE_H *file, const uint8_t *buf, int64_t size);
};

typedef AACS_FILE_H* (*AACS_FILE_OPEN)(const char* filename, const char *mode);

/**
 *
 *  Register function pointer that will be used to open a file
 *
 * @param p function pointer
 * @return previous function pointer registered
 */
AACS_PUBLIC AACS_FILE_OPEN aacs_register_file(AACS_FILE_OPEN p);

/**
 *
 *  Function that will be used to open a file
 *
 *  NOTE: file name is relative to disc root directory !
 *
 * @param handle application-specific handle
 * @param filename file to open
 * @return pointer to AACS_FILE_H, NULL if error
 */
typedef AACS_FILE_H* (*AACS_FILE_OPEN2)(void *handle, const char* filename);

/**
 *
 *  Register function pointer that will be used to open a file
 *
 * @param aacs aacs instance
 * @param handle handle that will be passed to file open function
 * @param p function pointer
 */
struct aacs;
AACS_PUBLIC void aacs_set_fopen(struct aacs *aacs, void *handle, AACS_FILE_OPEN2 p);

#ifdef __cplusplus
}
#endif

#endif /* AACS_FILESYSTEM_H_ */
