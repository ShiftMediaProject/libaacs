
#ifndef MKB_H_
#define MKB_H_

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

typedef struct mkb MKB;
struct mkb {
    size_t size;    // file size
    uint8_t *buf;   // file contents
};

MKB *mkb_open(const char *path);    // init MKB
void mkb_close(MKB *mkb);           // free MKB

uint8_t mkb_type(MKB *mkb);                                      // returns type
uint32_t mkb_version(MKB *mkb);                                  // returns version
uint8_t *mkb_host_revokation_entries(MKB *mkb, size_t *len);   // returns list of revoked host entries
uint8_t *mkb_drive_revokation_entries(MKB *mkb, size_t *len);  // returns list of revoked drive entries
uint8_t *mkb_subdiff_records(MKB *mkb, size_t *len);           // returns subset-diff records
uint8_t *mkb_subdiff_offsets(MKB *mkb, size_t *len);           // returns list of subset-diff offsets
uint8_t *mkb_cvalues(MKB *mkb, size_t *len);                   // returns list of cvalues
uint8_t *mkb_mk_dv(MKB *mkb);                                    // media key verification data return in param (always 16 bytes)
uint8_t *mkb_signature(MKB *mkb, size_t *len);                 // returns MKB signature

/* The Media Key block is of variable size but must be a multiple of 4
 * MKB Structure:
 *      Type and Version record (12 bytes)
 *          0   | type = 0x10
 *          1-3 | length = 0x00000c
 *          4-7 | MKB type = 0x000?1003
 *          8-11| MKB version
 *          
 *      Host Revokation List record (variable - multiple of 4 bytes)
 *          0                        | type = 0x21
 *          1-3                      | length
 *          4-7                      | total entries
 *          8-11                     | entires in signature block 1 (N1)
 *          12-19                    | host revokation list entry 0
 *          .
 *          .
 *          (12+(N1-1)*8)-(12+N1*8)-1| entry n
 *          (12+ N1*8)-(52+ N1*8)-1  | block 1 signature
 *          (52+ N1*8)-(length-1)    | more signature blocks
 *          
 *          Host Rekovation List entry (8 bytes)
 *              0-1| range
 *              2-7| host id
 *              
 *      Drive Revokation List record (variable - multiple of 4 bytes)
 *          0                        | type = 0x20
 *          1-3                      | length
 *          4-7                      | total entries
 *          8-11                     | entires in signature block 1 (N1)
 *          12-19                    | drive revokation list entry 0
 *          .
 *          .
 *          (12+(N1-1)*8)-(12+N1*8)-1| entry n
 *          (12+ N1*8)-(52+ N1*8)-1  | block 1 signature
 *          (52+ N1*8)-(length-1)    | more signature blocks
 *          
 *          Drive Rekovation List entry (8 bytes)
 *              0-1| range
 *              2-7| host id
 *              
 *      Verify Media Key record
 *          0   | type = 0x81
 *          1-3 | length = 0x14
 *          4-19| verification data (Dv)
 *          
 *      Explicit Subset-Difference record (variable - multiple of 4 bytes)
 *          0       | type = 0x04
 *          1-3     | length
 *          4       | uv mask 0
 *          5-8     | uv number 0
 *          .
 *          .
 *          length-1| end of uv number n
 *      
 *      Subset-Difference Index record (variable, padded - multiple of 4 bytes)
 *          0       | type = 0x07
 *          1-3     | length
 *          4-7     | span (number of devices)
 *          8-10    | offset 0
 *          .
 *          .
 *          length-1| end of offset n
 *          
 *      Media Key Data / cvalue record (variable - muliple of 4 bytes)
 *          0       | type = 0x05
 *          1-3     | length
 *          4-19    | media key data 0
 *          .
 *          .
 *          length-1| end of media key data n
 *          
 *      End of Media Key Block record (variable - multiple of 4 bytes)
 *          0         | type = 0x02
 *          1-3       | length
 *          4-length-1| signature
*/

#endif /* MKB_H_ */
