/*
 * This file is part of libaacs
 * Copyright (C) 2012   Konstantin Pavlov
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

#ifndef AACS_DARWIN_H
#define AACS_DARWIN_H

#include <util/attributes.h>

AACS_PRIVATE const char *darwin_get_config_home(void);
AACS_PRIVATE const char *darwin_get_config_system(const char *dir);
AACS_PRIVATE const char *darwin_get_cache_home(void);

#endif
