/*
 * This file is part of libaacs
 * Copyright (C) 2011  VideoLAN
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

#ifndef AACS_DIRS_H
#define AACS_DIRS_H

#include <util/attributes.h>

#ifdef _WIN32
AACS_PRIVATE int         win32_mkdir(const char *dir);
#endif

AACS_PRIVATE const char *get_config_home(void);
AACS_PRIVATE const char *get_config_system(const char *dir);
AACS_PRIVATE const char *get_cache_home(void);

#endif
