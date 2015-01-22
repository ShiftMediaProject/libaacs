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

#ifndef AACS_PATH_H
#define AACS_PATH_H

#include "util/attributes.h"

#ifndef AACS_PATH_MAX
#define AACS_PATH_MAX 1024
#endif

BD_PRIVATE char *aacs_resolve_path(const char *path, char *resolved_path);

#endif /* AACS_PATH_H */
