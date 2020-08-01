/*
 * This file is part of libaacs
 * Copyright (C) 2020  VideoLAN
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

#include "util/strutl.h"
#include "util/macro.h"
#include "libaacs/unit_key.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read_file.h"

static void _uk_dump(AACS_UK *uk)
{
    unsigned i, j;

    printf("Num BDMV dir:             %d\n", uk->num_bdmv_dir);
    printf("Application type:         %d\n", uk->app_type);
    printf("Use_SKB_Unified_MKB_Flag: %d\n", uk->use_skb_mkb);

    printf("Encrypted unit keys: %u\n", uk->num_uk);
    for (i = 0; i < uk->num_uk; i++) {
        printf("  %u: ", i + 1);
        for (j = 0; j < 16; j++) {
            printf("%02x", uk->enc_uk[i].key[j]);
        }
        printf("\n");
    }

    printf("Title CPS units (%d titles):\n", uk->num_titles);
    if (uk->num_titles > 0)
        printf("  FP: %u\n", uk->title_cps_unit[0]);
    if (uk->num_titles > 1)
        printf("  TM: %u\n", uk->title_cps_unit[1]);
    for (i = 0; i < uk->num_titles; i++)
        printf("  %2u: %u\n", i + 1, uk->title_cps_unit[i + 2]);
}

int main (int argc, char **argv)
{
    AACS_UK *uk;
    uint8_t *data;
    size_t   size;
    int      aacs2 = argc > 2;
    size_t   l, b;

    if (argc < 2) {
        fprintf(stderr, "usage: uk_dump <Unit_Key_RO.inf> [AACS2]\n");
        exit(-1);
    }

    size = _read_file(argv[1], 16, 1024*1024, &data);
    if (!size) {
        exit(-1);
    }

    printf("Unit key file size: %8zu bytes\n", size);
    uk = uk_parse(data, size, aacs2);

    if (!uk) {
        fprintf(stderr, "Parsing failed\n");
        exit(-1);
    }

    _uk_dump(uk);

    printf("Raw data:\n");
    uint32_t mask = 0;
    for (l = 0; l < size; l += 16) {
      mask <<= 8;
      for (b = 0; b < 32 && l+b < size; b++)
        mask |= data[l + b];
      if (mask) {
        printf("%04lx: ", (long)l);

        for (b = 0; b < 8 && l+b < size; b++)
          printf("%02x ", data[l + b]);
        printf(" ");
        for (b = 8; b < 16 && l+b < size; b++)
          printf("%02x ", data[l + b]);
        printf("        ");
        for (b = 0; b < 8 && l+b < size; b++)
          if (data[l+b]) printf("%02x ", data[l + b]); else printf("   ");
        printf(" ");
        for (b = 8; b < 16 && l+b < size; b++)
          if (data[l+b]) printf("%02x ", data[l + b]); else printf("   ");

        printf("\n");
      }
    }

    uk_free(&uk);
    free(data);

    return 0;
}
