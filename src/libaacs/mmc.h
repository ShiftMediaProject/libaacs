/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010       npzacs
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

#include "util/attributes.h"

#include <stdint.h>

typedef struct mmc MMC;

#define MMC_SUCCESS             0 /* no errors */
#define MMC_ERROR              -1 /* MMC failed */
#define MMC_ERROR_CERT_REVOKED -2 /* revoked certificate */

#define MMC_READ_VID            0
#define MMC_READ_PMSN           1
#define MMC_READ_DATA_KEYS      2

AACS_PRIVATE MMC *mmc_open(const char *path);
AACS_PRIVATE void mmc_close(MMC *mmc);
AACS_PRIVATE int  mmc_read_auth(MMC *mmc, const uint8_t *host_priv_key, const uint8_t *host_cert,
                                int request, uint8_t *p1, uint8_t *p2);
AACS_PRIVATE int  mmc_read_drive_cert(MMC *mmc, uint8_t *drive_cert);
AACS_PRIVATE const uint8_t *mmc_get_drive_cert(MMC *mmc);

/* read partial MKB */
AACS_PRIVATE uint8_t *mmc_read_mkb(MMC *mmc, int address, int *size);

#endif /* MMC_H_ */
