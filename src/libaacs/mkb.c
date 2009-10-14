
#include <malloc.h>

#include "mkb.h"
#include "../util/macro.h"

uint8_t *_record(MKB *mkb, uint8_t type, size_t *rec_len);  // returns ptr to requests MKB record

uint8_t *_record(MKB *mkb, uint8_t type, size_t *rec_len)
{
    size_t pos = 0, len = 0;
    
    while (pos + 4 <= len) {
        len = MKINT_BE24(mkb->buf + pos + 1);
        
        if (rec_len) {
            *rec_len = len;
        }
        
        if (mkb->buf[pos] == type)
            return mkb->buf + pos;
        
        pos += len;
    }
    
    return NULL;
}

struct mkb *mkb_open(const char *path)
{
    FILE *fp = NULL;
    char f_name[100];
    MKB *mkb = malloc(sizeof(MKB));

    snprintf(f_name, 100, "%s/AACS/MKB_RO.inf", path);
    
    fp = fopen(f_name, "rb");
    
    fseek(fp, 0, SEEK_END);
    mkb->size = ftell(fp);
    rewind(fp);
    
    mkb->buf = malloc(mkb->size);
    
    fread(mkb->buf, 1, mkb->size, fp);
    
    fclose(fp);

    X_FREE(fp);
    
    return mkb; 
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

