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

#ifndef MMC_H_
#define MMC_H_

#include <util/attributes.h>

#include <stdint.h>

typedef struct mmc MMC;

AACS_PRIVATE MMC *mmc_open(const char *path, const uint8_t *host_priv_key,
                           const uint8_t *host_cert, const uint8_t *host_nonce,
                           const uint8_t *host_key_point);
AACS_PRIVATE void mmc_close(MMC *drive);
AACS_PRIVATE int mmc_read_vid(MMC *mmc, uint8_t *vid);

#endif /* MMC_H_ */
