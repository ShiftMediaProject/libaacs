/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010-2015  npzacs
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

#ifndef MMC_DEVICE_H_
#define MMC_DEVICE_H_

#include "util/attributes.h"

#include <stdint.h>
#include <stddef.h>

typedef struct mmcdev MMCDEV;

BD_PRIVATE MMCDEV *device_open(const char *path);
BD_PRIVATE void    device_close(MMCDEV **mmc);

BD_PRIVATE int     device_send_cmd(MMCDEV *dev, const uint8_t *cmd, uint8_t *buf, size_t tx, size_t rx);

#endif /* MMC_DEVICE_H_ */
