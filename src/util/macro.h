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

#ifndef MACRO_H_
#define MACRO_H_

#include <stdlib.h>  /* free() */

#define MKINT_BE16(X) ( (X)[0] << 8 | (X)[1] )
#define MKINT_BE24(X) ( (X)[0] << 16 | (X)[1] << 8 | (X)[2] )
#define MKINT_BE32(X) ( (X)[0] << 24 | (X)[1] << 16 |  (X)[2] << 8 | (X)[3] )
#define MKINT_BE48(X) ( (uint64_t)((X)[0]) << 40 | (uint64_t)((X)[1]) << 32 | (uint64_t)((X)[2]) << 24 | \
                        (uint64_t)((X)[3]) << 16 | (uint64_t)((X)[4]) << 8  | (uint64_t)((X)[5]) )

#define X_FREE(X)     ( free(X), X = NULL )

#endif /* MACRO_H_ */
