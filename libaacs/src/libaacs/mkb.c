#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mkb.h"
#include "file/file.h"
#include "util/macro.h"
#include "util/logging.h"

#include <stdio.h>

uint8_t *_record(MKB *mkb, uint8_t type, size_t *rec_len)
{
    size_t pos = 0, len = 0;

    while (pos + 4 <= mkb->size) {
        len = MKINT_BE24(mkb->buf + pos + 1);

        if (rec_len) {
            *rec_len = len;
        }

        if (mkb->buf[pos] == type) {
            DEBUG(DBG_MKB, "Retrieved MKB record 0x%02x (%p)\n", type,
                  mkb->buf + pos);

            return mkb->buf + pos;
        }

        pos += len;
    }

    return NULL;
}

MKB *mkb_open(const char *path)
{
    FILE_H *fp = NULL;
    char f_name[100];
    MKB *mkb = malloc(sizeof(MKB));

    snprintf(f_name, 100, "%s/AACS/MKB_RO.inf", path);
    DEBUG(DBG_MKB, "Opening MKB %s... (%p)\n", f_name, mkb);

    if ((fp = file_open(f_name, "rb"))) {
        file_seek(fp, 0, SEEK_END);
        mkb->size = file_tell(fp);
        file_seek(fp, 0, SEEK_SET);

        mkb->buf = malloc(mkb->size);

        file_read(fp, mkb->buf, mkb->size);

        DEBUG(DBG_MKB, "MKB size: %zd (%p)\n", mkb->size, mkb);
        DEBUG(DBG_MKB, "MKB version: %d (%p)\n", mkb_version(mkb), mkb);

        file_close(fp);
        return mkb;
    }

    DEBUG(DBG_MKB, "Error opening MKB! (%p)\n", mkb);

    return NULL;
}

void mkb_close(MKB *mkb)
{
    X_FREE(mkb->buf);
    X_FREE(mkb);
}

uint8_t mkb_type(MKB *mkb)
{
    uint8_t *rec = _record(mkb, 0x10, NULL);

    return MKINT_BE32(rec + 4);
}

uint32_t mkb_version(MKB *mkb)
{
    uint8_t *rec = _record(mkb, 0x10, NULL);

    return MKINT_BE32(rec + 8);
}

uint8_t *mkb_subdiff_records(MKB *mkb, size_t *len)
{
    uint8_t *rec = _record(mkb, 0x04, len) + 4;
    *len -= 4;

    return rec;
}

uint8_t *mkb_cvalues(MKB *mkb, size_t *len)
{
    uint8_t *rec = _record(mkb, 0x05, len) + 4;
    *len -= 4;

    return rec;
}

uint8_t *mkb_mk_dv(MKB *mkb)
{
    return _record(mkb, 0x81, NULL) + 4;
}

uint8_t *mkb_signature(MKB *mkb, size_t *len)
{
    uint8_t *rec = _record(mkb, 0x02, len);
    *len -= 4;

    return rec + 4;

}
