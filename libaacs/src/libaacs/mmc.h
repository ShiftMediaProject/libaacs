/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  cRTrn13
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders of libaacs
 * gives permission to link the code of its release of libaacs with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked executables.  You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL".  If you
 * modify this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to do
 * so, delete this exception statement from your version.
 */

#ifndef MMC_H_
#define MMC_H_

#include <stdint.h>

typedef struct mmc MMC;
struct mmc {
    int fd;
    uint8_t sk, asc, ascq;
    uint8_t host_priv_key[20], host_cert[92], host_nonce[20];
    uint8_t host_key_point[40];
};

MMC *mmc_open(const char *path, const uint8_t *host_priv_key,
              const uint8_t *host_cert, const uint8_t *host_nonce,
              const uint8_t *host_key_point);
void mmc_close(MMC *drive);
int mmc_read_vid(MMC *mmc, uint8_t *vid);

#endif /* MMC_H_ */
