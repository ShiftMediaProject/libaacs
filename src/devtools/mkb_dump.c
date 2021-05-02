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
#include "libaacs/mkb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "read_file.h"

static const struct {
    uint8_t     type;
    const char *name;
} rec_names[] = {
    { 0x02, "End of media key block" },
    { 0x04, "Explicit Subset-Difference" },
    { 0x05, "Media Key Data / CVALUE" },
    { 0x07, "Subset-Difference Index" },
    { 0x0c, "Media Key Variant Data" },
    { 0x10, "Type and version" },
    { 0x20, "Drive revocation list (AACS1)" },
    { 0x21, "Host revocation list (AACS1)" },
    { 0x30, "Drive revocation list (AACS2)" },
    { 0x31, "Host revocation list (AACS2)" },
    { 0x81, "Verify media key / DVALUE" },
    { 0x86, "Verify media key / DVALUE (AACS2)" },
};

static const struct {
    uint32_t    type;
    const char *name;
} mkb_types[] = {
    { MKB_TYPE_3, "Type 3" },
    { MKB_TYPE_4, "Type 4" },
    { MKB_TYPE_10_CLASS_II, "Type 10, class II" },
    { MKB_20_CATEGORY_C, "Type 2.0 Category C" },
    { MKB_21_CATEGORY_C, "Type 2.1 Category C" },
};

static const char *rec_name(uint8_t type) {
    size_t i;
    for (i = 0; i < sizeof(rec_names)/sizeof(rec_names[0]); i++)
        if (rec_names[i].type == type)
            return rec_names[i].name;
    return "(UNKNOWN)";
}

static const char *type_name(uint32_t type) {
    size_t i;
    for (i = 0; i < sizeof(mkb_types)/sizeof(mkb_types[0]); i++)
        if (mkb_types[i].type == type)
            return mkb_types[i].name;
    return "(UNKNOWN)";
}

static void _dump_signature(const uint8_t *sig, size_t len)
{
    size_t i;

    printf("  Signature (%zu bytes): ", len);
    for (i = 0; i < len; i++) {
        printf("%02x", sig[i]);
    }
    printf("\n");
}

static void _dump_aacs1_rl(const uint8_t *rl, size_t rl_size)
{
    if (rl_size < 4)
        return;

    uint32_t total_entries = MKINT_BE32(rl);
    rl += 4; rl_size -= 4;

    if ((size_t)total_entries > rl_size / 8) {
        printf("  revocation list size mismatch: total_entries=%u\n", (unsigned)total_entries);
        return;
    }

    while (total_entries > 0 && rl_size >= 4) {
        uint32_t entries = MKINT_BE32(rl);
        rl += 4; rl_size -= 4;
        if (entries >= (0xffffffff - 8 - 40) / 8) {
            printf("  invalid revocation list\n");
            break;
        }

        size_t rec_len = 8 * entries + 40;
        if (rec_len > rl_size) {
            printf("  revocation list size mismatch\n");
            break;
        }

        unsigned ii;
        for (ii = 0; ii < entries; ii++) {
            uint16_t len = MKINT_BE16(rl);
            uint64_t id  = MKINT_BE48(rl + 2);
            printf("    %3d: id %12" PRIx64, ii, id);
            if (len) {
                printf("-%12" PRIx64, id + len);
            }
            printf("\n");
            rl += 8; rl_size -= 8;
        }
        _dump_signature(rl, 40);
        rl += 40; rl_size -= 40;
        if (total_entries <= entries) {
            total_entries -= entries;
        } else {
            printf("  revocation list size mismatch\n");
            total_entries = 0;
        }
    }
}

static void _dump_type_and_version(const uint8_t *p, size_t size)
{
    if (size >= 4)
      printf("  MKB type:      0x%08x  (%s)\n", MKINT_BE32(p), type_name(MKINT_BE32(p)));
    if (size >= 8)
        printf("  MKB version:     %8d\n",  MKINT_BE32(p+4));
}

static void _dump_record(MKB *mkb, int record)
{
    size_t size = mkb_data_size(mkb);
    const uint8_t *data = mkb_data(mkb);
    size_t pos, len;

    printf("\nRecord 0x%02x (%s):\n", record, rec_name(record));

    for (pos = 0; pos + 4 <= size; pos += len) {
        uint8_t type = data[pos];
        len  = MKINT_BE24(data + pos + 1);
        if (len > size - pos) {
            printf("  invalid record 0x%02x size: %zu\n", type, len);
            break;
        }
        if (type == record) {
            switch (record) {
                case 0x02: if (len > 4 && len <= 64) _dump_signature(data + pos + 4, len - 4); break;
                case 0x10: if (len > 4)              _dump_type_and_version(data + pos + 4, len - 4); break;
                case 0x20:
                case 0x21: if (len > 4)              _dump_aacs1_rl(data + pos + 4, len - 4); break;
            }
            printf("  Raw data (%zu bytes):\n", len - 4);
            const uint8_t *p = data + pos + 4;
            const uint8_t *e = data + pos + len;
            while (p < e) {
                size_t i;
                printf("      ");
                for (i = 0; i < 8 && p < e; i++, p++)
                    printf("%02x ", *p);
                printf("  ");
                for (i = 0; i < 8 && p < e; i++, p++)
                    printf("%02x ", *p);
                printf("\n");
            }
            break;
        }
    }
}

static void _list_missing_records(const uint8_t *seen_map)
{
    int type;

    for (type = 0; type < 256; type++) {
        if (!seen_map[type]) {
            size_t i;
            for (i = 0; i < sizeof(rec_names)/sizeof(rec_names[0]); i++) {
                if (rec_names[i].type == type) {
                    printf("    %s\n", rec_names[i].name);
                }
            }
        }
    }
}

static void _list_records(MKB *mkb, uint8_t *seen_map)
{
    const uint8_t *data = mkb_data(mkb);
    size_t size = mkb_data_size(mkb);
    size_t pos, len;

    for (pos = 0; pos + 4 <= size; pos += len) {
        uint8_t type = data[pos];
        len  = MKINT_BE24(data + pos + 1);
        printf("    record 0x%02x: %10zu bytes  %s\n", type, len, rec_name(type));
        seen_map[type & 0xff] = 1;
        if (len == 0) {
            printf("    UNKNOWN:     %10zu bytes\n", size - pos);
            break;
        }
    }
}

int main (int argc, char **argv)
{
    MKB     *mkb;
    uint8_t  seen_map[256] = {0};
    uint8_t *data;
    size_t   size;
    int      arg;

    if (argc < 2) {
        fprintf(stderr, "usage: mkb_dump <mkb_file> [record [record ...]]\n");
        exit(-1);
    }

    size = _read_file(argv[1], 16, 1024*1024*1024, &data);
    if (!size) {
        exit(-1);
    }

    printf("MKB file size: %10zu bytes\n", size);

    mkb = mkb_init(data, size);
    if (!mkb) {
        fprintf(stderr, "MKB init failed: %s (%d)\n", strerror(errno), errno);
        free(data);
        exit(-1);
    }

    printf("MKB size:      %10zu bytes\n\n", mkb_data_size(mkb));

    printf("MKB type:    %s (0x%08x)\n", type_name(mkb_type(mkb)), mkb_type(mkb));
    printf("MKB version: %d\n", mkb_version(mkb));

    printf("MKB records:\n");
    _list_records(mkb, seen_map);
    printf("    PADDING:     %10zu bytes\n", size - mkb_data_size(mkb));

    printf("\nNot present records:\n");
    _list_missing_records(seen_map);

    for (arg = 2; arg < argc; arg++) {
        int record = strtol(argv[arg], NULL, 16);
        _dump_record(mkb, record);
    }

    /* check padding */
    for (size_t i = mkb_data_size(mkb); i < size; i++) {
        if (data[i]) {
            printf("\nData found from padding !\n");
            break;
        }
    }

    mkb_close(mkb);

    return 0;
}
