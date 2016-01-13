/*
 * This file is part of libaacs
 * Copyright (C) 2016  npzacs
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

#ifndef AACS_CCI_DATA_H_
#define AACS_CCI_DATA_H_

#include <stdint.h>

/* Blu-ray Disc Pre-recorded Book, 3.9.4.2 Basic CCI for AACS */
typedef struct aacs_basic_cci {
    uint8_t epn;              /* Encryption Plus Non-assertion. Valid if cci == 0. */
    uint8_t cci;              /* Copy Control Information. 0 == copy freely. */
    uint8_t image_constraint; /* High Definition Analog Output in High Definition Analog Form */
    uint8_t digital_only;     /* Output of decrypted content is allowed only for Digital Outputs */
    uint8_t apstb;            /* Analog copy protection information */

    uint16_t num_titles;
    /* title types (bit mask) */
    uint8_t title_type[1024/8]; /* Title type bit mask. 0 == basic title. 1 == enhanced title. */
} AACS_BASIC_CCI;

#endif /* AACS_CCI_DATA_H_ */
