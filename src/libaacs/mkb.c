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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mkb.h"

#include "util/macro.h"
#include "util/logging.h"
#include "util/strutl.h"

#include <stdio.h>

struct mkb {
    size_t size;    // file size
    uint8_t *buf;   // file contents
};

static const uint8_t *_record(MKB *mkb, uint8_t type, size_t *rec_len)
{
    size_t pos = 0, len = 0;

    while (pos + 4 <= mkb->size) {
        len = MKINT_BE24(mkb->buf + pos + 1);

        if (rec_len) {
            *rec_len = len;
        }

        if (mkb->buf[pos] == type) {
            BD_DEBUG(DBG_MKB, "Retrieved MKB record 0x%02x (%p)\n", type,
                  (void*)(mkb->buf + pos));

            if (len > mkb->size - pos) {
                BD_DEBUG(DBG_MKB | DBG_CRIT, "Ignoring truncated MKB record 0x%02x @ %zu, size %zu (%p)\n", type, pos, len,
                         (void*)(mkb->buf + pos));
                return NULL;
            }

            return mkb->buf + pos;
        }

        if (len == 0) {
            BD_DEBUG(DBG_MKB, "Couldn't retrieve MKB record 0x%02x - len=0 (%p)\n", type,
                  (void*)(mkb->buf + pos));
            break;
        }

        pos += len;
    }

    return NULL;
}

MKB *mkb_init(uint8_t *data, size_t len)
{
    MKB *mkb = malloc(sizeof(MKB));

    if (!mkb) {
        BD_DEBUG(DBG_MKB | DBG_CRIT, "out of memory\n");
        return NULL;
    }

    mkb->size = len;
    mkb->buf  = data;

    BD_DEBUG(DBG_MKB, "MKB size: %u\n", (unsigned)mkb->size);
    BD_DEBUG(DBG_MKB, "MKB type: 0x%x\n", mkb_type(mkb));
    BD_DEBUG(DBG_MKB, "MKB version: %d\n", mkb_version(mkb));

    return mkb;
}

void mkb_close(MKB *mkb)
{
    if (mkb) {
        X_FREE(mkb->buf);
        X_FREE(mkb);
    }
}

const uint8_t *mkb_data(MKB *mkb)
{
    return mkb->buf;
}

size_t mkb_data_size(MKB *mkb)
{
    size_t pos = 0;

    while (pos + 4 <= mkb->size) {
        if (!mkb->buf[pos]) {
            break;
        }
        pos += MKINT_BE24(mkb->buf + pos + 1);
    }

    if (pos > mkb->size) {
        BD_DEBUG(DBG_MKB | DBG_CRIT, "mkb_data_size(): invalid or truncated MKB\n");
        return mkb->size;
    }

    BD_DEBUG(DBG_MKB, "MKB data size %zu bytes\n", pos);
    return pos;
}

static const uint8_t *_simple_record(MKB *mkb, uint8_t type, size_t *len)
{
    const uint8_t *rec = _record(mkb, type, len);

    if (*len < 4) {
        return NULL;
    }
    if (rec) {
        rec += 4;
        *len -= 4;
    }

    return rec;
}


uint32_t mkb_type(MKB *mkb)
{
    const uint8_t *rec = mkb_type_and_version_record(mkb);

    if (!rec) {
        return 0;
    }

    return MKINT_BE32(rec + 4);
}

uint32_t mkb_version(MKB *mkb)
{
    const uint8_t *rec = mkb_type_and_version_record(mkb);

    if (!rec) {
        return 0;
    }

    return MKINT_BE32(rec + 8);
}

const uint8_t *mkb_type_and_version_record(MKB *mkb)
{
    size_t len = 0;
    const uint8_t *rec = _record(mkb, 0x10, &len);

    if (len < 12) {
        return NULL;
    }

    return rec;
}

const uint8_t *mkb_host_revokation_entries(MKB *mkb, size_t *len)
{
    return _simple_record(mkb, 0x21, len);
}

const uint8_t *mkb_drive_revokation_entries(MKB *mkb, size_t *len)
{
    return _simple_record(mkb, 0x20, len);
}

const uint8_t *mkb_subdiff_records(MKB *mkb, size_t *len)
{
    return _simple_record(mkb, 0x04, len);
}

const uint8_t *mkb_cvalues(MKB *mkb, size_t *len)
{
    return _simple_record(mkb, 0x05, len);
}

const uint8_t *mkb_mk_dv(MKB *mkb)
{
    const uint8_t *rec;
    uint8_t dv_record;
    size_t len;

    switch (mkb_type(mkb)) {
        case MKB_20_CATEGORY_C:
        case MKB_21_CATEGORY_C:
            dv_record = 0x86;
            break;
        case MKB_TYPE_3:
        case MKB_TYPE_4:
        case MKB_TYPE_10_CLASS_II:
        default:
            dv_record = 0x81;
            break;
    }

    rec = _simple_record(mkb, dv_record, &len);

    if (len < 16) {
        return NULL;
    }

    return rec;
}

const uint8_t *mkb_signature(MKB *mkb, size_t *len)
{
    return _simple_record(mkb, 0x02, len);
}

static int _cert_is_revoked(const uint8_t *rl, size_t rl_size, const uint8_t *cert_id_bin)
{
    if (rl && rl_size > 8) {
        uint64_t cert_id = MKINT_BE48(cert_id_bin);
        /*int total = MKINT_BE32(rl);*/
        uint32_t entries = MKINT_BE32(rl + 4);
        unsigned ii;

        if (entries >= (0xffffffff - 8 - 40) / 8) {
            BD_DEBUG(DBG_MKB, "invalid revocation list\n");
            return 0;
        }

        size_t rec_len = 4 + 4 + 8 * entries + 40;
        if (rec_len > rl_size) {
            BD_DEBUG(DBG_MKB, "revocation list size mismatch\n");
            return -1;
        }

        rl += 8;
        for (ii=0; ii < entries; ii++) {
            uint16_t len = MKINT_BE16(rl);
            uint64_t id  = MKINT_BE48(rl + 2);
            if (cert_id >= id && cert_id <= id + len) {
                char str[16];
                str_print_hex(str, rl + 2, 6);
                BD_DEBUG(DBG_MKB, "Certificate %s has been revoked\n", str);
                return 1;
            }
            rl += 8;
        }
    }

    return 0;
}

int mkb_host_cert_is_revoked(MKB *mkb, const uint8_t *cert_id)
{
    if (mkb) {
        size_t rl_size = 0;
        const uint8_t *rl = mkb_host_revokation_entries(mkb, &rl_size);
        if (rl) {
            return _cert_is_revoked(rl, rl_size, cert_id);
        }
    }

    return -1;
}

int mkb_drive_cert_is_revoked(MKB *mkb, const uint8_t *cert_id)
{
    if (mkb) {
        size_t rl_size = 0;
        const uint8_t *rl = mkb_drive_revokation_entries(mkb, &rl_size);
        if (rl) {
            return _cert_is_revoked(rl, rl_size, cert_id);
        }
    }

    return -1;
}
