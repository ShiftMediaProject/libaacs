/*
 * This file is part of libaacs
 * Copyright (C) 2011 libaacs
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

#ifndef AACS_VERSION_H_
#define AACS_VERSION_H_

#define AACS_VERSION_CODE(major, minor, micro) \
    (((major) * 10000) +                         \
     ((minor) *   100) +                         \
     ((micro) *     1))

#define AACS_VERSION_MAJOR 0
#define AACS_VERSION_MINOR 11
#define AACS_VERSION_MICRO 1

#define AACS_VERSION_STRING "0.11.1"

#define AACS_VERSION \
    AACS_VERSION_CODE(AACS_VERSION_MAJOR, AACS_VERSION_MINOR, AACS_VERSION_MICRO)

#endif /* AACS_VERSION_H_ */
