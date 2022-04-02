/*
 * This file is part of libaacs
 * Copyright (C) 2013  VideoLAN
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

#include "path.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "util/strutl.h"

#define MAX_LINKS 32

#ifndef AACS_PATH_MAX
#define AACS_PATH_MAX 1024
#endif

char *aacs_resolve_path(const char *path)
{
    char resolved_path[AACS_PATH_MAX];
    char tmp_path[AACS_PATH_MAX];
    char link[AACS_PATH_MAX];
    char *new_path = resolved_path;
    char *path_end = tmp_path + AACS_PATH_MAX - 1;
    int readlinks = 0;
    int n;

    if (!path || !*path || strlen(path) >= AACS_PATH_MAX - 2) {
        return NULL;
    }

    /* shadow original path with a copy */
    strcpy(tmp_path, path);
    path = tmp_path;

    /* handle relative path */
    if (*path != '/') {
        if (!getcwd(new_path, AACS_PATH_MAX - 1)) {
            return NULL;
        }
        new_path += strlen(new_path);
        if (new_path[-1] != '/') {
            *new_path++ = '/';
        }
    } else {
        *new_path++ = '/';
        path++;
    }


    while (*path) {
        /* "/" */
        if (*path == '/') {
            path++;
            continue;
        }
        /* "." */
        if (path[0] == '.' && (!path[1] || path[1] == '/')) {
            path++;
            continue;
        }
        /* ".." */
        if (path[0] == '.' && path[1] == '.' && (!path[2] || path[2] == '/')) {
            path += 2;
            /* not at root ? -> back up one level */
            if (new_path != resolved_path + 1) {
                while ((--new_path)[-1] != '/');
            }
            continue;
        }

        /* copy next component */
        while (*path && *path != '/') {
            if (path >= path_end) {
                return NULL;
            }
            *new_path++ = *path++;
        }

        /* avoid symlink loops */
        if (readlinks++ > MAX_LINKS) {
            return NULL;
        }

        /* resolve symlink */

        *new_path = 0;
        n = readlink(resolved_path, link, AACS_PATH_MAX - 1);

        if (n < 0) {
            if (errno != EINVAL) {
              return NULL;
            }
            /* file exists but isn't a symlink. */

        } else if (n >= AACS_PATH_MAX - 1) {
            return NULL;

        } else {
            link[n] = 0;
            if (*link == '/') {
                new_path = resolved_path;
            } else {
                while (*(--new_path) != '/');
            }

            if (path + n >= path_end) {
                return NULL;
            }

            /* update what's left */
            strcat(link, path);
            strcpy(tmp_path, link);
            path = tmp_path;
        }

        if (*path) {
            *new_path++ = '/';
        }
    }

    *new_path = 0;
    return str_dup(resolved_path);
}

#ifdef TEST_AACS_RESOLVE_PATH
#include <stdio.h>
void main(int argc, char *argv[]) {
    printf("%s -> %s\n", argv[0], aacs_resolve_path(argv[0]));
    printf("%s -> %s\n", argv[1], aacs_resolve_path(argv[1]));
}
#endif
