/*************************************************************************
 *                                                                       *
 *  convert key db files into internal config file structure             *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../src/file/configfile.h"

#define OPTS "c:d:o:ha"

/* Keys are stored in a binary file in a record format
 *
 * Record format:
 *          0                   | type
 *          1-3                 | length
 *          4-5                 | num entries
 *          6-9                 | entry length
 *          10-(9+entry_length) | entry 1
 *          .
 *          .
 *          length-1            | end
 *
 */

/* config header structure */
typedef struct recordheader
{
    unsigned char type;              /* Record type in hex */
    unsigned char length[3];         /* Entire record length inc header */
    unsigned char count[2];          /* Record count */
    unsigned char entry_length[4];   /* Record length */
} RECORDHEADER;


/* config record structure */
typedef struct confrecord
{
    unsigned char key1[20];     /* first key */
    unsigned char info[10];     /* movie name / info */
    unsigned char key2[16];     /* second key */
} RECORDDATA;


static char append_flag = 0;
static const char *default_keydb   = "keydb.cfg";
static const char *default_devkey  = "ProcessingDeviceKeysSimple.txt";
static const char *default_outfile = "keyfile.db";


/*
 * convert ascii string into unsigned char array (hex)
 */
static int ascii2hex(const char *ascii, unsigned char *hex, size_t count)
{
    size_t idx = 0;
    char *pos = (char *)ascii;
    char c_byte[3];
    char *c_end;

    /* check ascii and hex array buffer */
    if ( ascii && hex ) {
        for ( ; idx < count; idx += 1 ) {
            if ( strncpy(c_byte, pos, (sizeof c_byte)-1 )) {
                hex[idx] = strtol(c_byte, &c_end, 16);
                pos += 2;
            }
        }
    }
    else {
        return -1;
    }

    return 0;
}


/* 
 * convert key db file into internal config file
 */
static int convertKeyDB2Config(const char *keydb, FILE *fpOutfile)
{
    FILE *fpKeyDB = NULL;
    int count = 0;
    size_t length = 0;
    size_t nwrite;
    const char *search_key2 = " | V | ";
    char *key2 = NULL;
    char buffer[512];
    long hdrpos;
    RECORDHEADER header;
    RECORDDATA record;

    if ( fpOutfile && (fpKeyDB = fopen(keydb, "r")) ) {
 
        memset(&header, 0, sizeof header);
        header.type = KF_VUK_ARRAY;
        header.entry_length[3] = 0x2E;
 
        /* seek the the last position */
        fseek(fpOutfile, 0, SEEK_END);

        /* store the current header position */
        hdrpos = ftell(fpOutfile);

        /* write dummy (first) header */
        nwrite = fwrite(&header, sizeof (char), sizeof header, fpOutfile);
        if ( nwrite != sizeof header ) {
            /* print error message and stop process */
            fprintf(stderr, "error while write file header\n");
            return -1;
        }

        /* read all key db entries */
        while (!feof(fpKeyDB)) {
            if ( fgets(buffer, sizeof buffer, fpKeyDB) ) {
                if ( (key2 = strstr(buffer, search_key2)) ) {
                    key2 += 7;
                    if ( ascii2hex(buffer, record.key1, 20) ) {
                        /* print error message and skip current line */
                        fprintf(stderr, "while convert key1\n");
                        continue;
                    }         /* Record type in hex */

                    if ( ascii2hex(key2, record.key2, 16) ) {
                        /* print error message and skip current line */
                        fprintf(stderr, "while convert key2\n");
                        continue;
                    }

                    if ( !strncpy((char *)record.info,  buffer + 43, sizeof record.info) ) {
                        /* print error message and skip current line */
                        fprintf(stderr, "while read info\n");
                        continue;
                    }
 
                    /* write data record into config file */
                    nwrite = fwrite(&record, sizeof (char), sizeof record, fpOutfile);
                    if ( nwrite != sizeof record ) {
                        /* print error message and stop process */
                        fprintf(stderr, "error while write record %d\n", count + 1);
                        return -1;
                    }

                    count += 1;
                }
            }
        }

        /* calculate file length from record count */
        length = count * 46 + 10;	
 
        /* store file length in header */
        header.length[0] = length >> 16;
        header.length[1] = length >> 8;
        header.length[2] = length & 0xFF;

        /* store record count in header */
        header.count[0] = count >> 8;
        header.count[1] = count & 0xFF;

        /* seek to header position (0) */
	if (fseek(fpOutfile, hdrpos, SEEK_SET) == 0 ) {
            /* write the new header with all data */
            nwrite = fwrite(&header, sizeof (char), sizeof header, fpOutfile);
            if ( nwrite != sizeof header ) {
                fprintf(stderr, "while write header (last)\n");
                return -1;
            }
        }
    }

    /* close key db (input) file */
    if ( fpKeyDB ) {
        fclose(fpKeyDB);
    }

    return 0;
}



static int convertDeviceKey(const char *devkey, FILE *fpOutfile)
{
    FILE *fpDevKey = NULL;
    size_t count = 0;
    size_t length = 0;
    size_t nwrite;
    char buffer[64];
    RECORDHEADER header;
    unsigned char record[16];
    long hdrpos;

    if ( fpOutfile && (fpDevKey = fopen(devkey, "r")) ) {
 
        memset(&header, 0, sizeof header);
        header.type = KF_PK_ARRAY;
        header.entry_length[3] = 0x10;
 
        /* seek the the last position */
        fseek(fpOutfile, 0, SEEK_END);

        /* store the current header position */
        hdrpos = ftell(fpOutfile);

        /* write dummy (first) header */
        nwrite = fwrite(&header, sizeof (char), sizeof header, fpOutfile);
        if ( nwrite != sizeof header ) {
            /* print error message and stop process */
            fprintf(stderr, "error while write file header\n");
            return -1;
        }

        /* read all key db entries */
        while (!feof(fpDevKey)) {
            if ( fgets(buffer, sizeof buffer, fpDevKey) ) {
                if ( ascii2hex(buffer, record, 16) ) {
                    /* print error message and skip current line */
                    fprintf(stderr, "while convert record data\n");
                    continue;
                }

                /* write data record into config file */
                nwrite = fwrite(&record, sizeof (char), sizeof record, fpOutfile);
                if ( nwrite != sizeof record ) {
                    /* print error message and stop process */
                    fprintf(stderr, "error while write record %d\n", count + 1);
                    return -1;
                }

                count += 1;
            }
        }

        /* calculate file length from record count */
        length = count * 16 + 10;
 
        /* store file length in header */
        header.length[0] = length >> 16;
        header.length[1] = length >> 8;
        header.length[2] = length & 0xFF;

        /* store record count in header */
        header.count[0] = count >> 8;
        header.count[1] = count & 0xFF;

	if (fseek(fpOutfile, hdrpos, SEEK_SET) == 0 ) {
            nwrite = fwrite(&header, sizeof (char), sizeof header, fpOutfile);
            if ( nwrite != sizeof header ) {
                fprintf(stderr, "while write header (last)\n");
                return -1;
            }
        }
    }

    /* close key db (input) file */
    if ( fpDevKey ) {
        fclose(fpDevKey);
    }

    return 0;
}


static int print_usage(const char *name)
{
    fprintf(stderr, "Usage: %s -a -h -c <in_keydb> -d <in_devkey> -o <out_keyfile>\n"
                    "    -a                append key information\n"
                    "    -h                print this usage\n"
                    "    -c <in_keydb>     keydb input file (default: %s)\n"
                    "    -d <in_devkey>    device key input file (default: %s)\n"
                    "    -o <out_keyfile>  keyfile file name (default: %s)\n", 
                    name, default_keydb, default_devkey, default_outfile);
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    FILE *fpOutput;
    char keydb_file[512];
    char devkey_file[512];
    char out_keyfile[512];
    struct stat fsCheck;
    int opt;

    strncpy(keydb_file,  "keydb.cfg", sizeof keydb_file);
    strncpy(devkey_file, "ProcessingDeviceKeysSimple.txt", sizeof devkey_file);
    strncpy(out_keyfile, "keyfile.db", sizeof out_keyfile);

    do {
        opt = getopt(argc, argv, OPTS);
        switch(opt) {
            case -1:
                break;

            case 'c':
                strncpy(keydb_file, optarg, sizeof keydb_file);
                break;

            case 'd':
                strncpy(devkey_file, optarg, sizeof devkey_file);
                break;

            case 'o':
                strncpy(out_keyfile, optarg, sizeof out_keyfile);
                break;

	    case 'h':
	        print_usage(argv[0]);
                break;

            case 'a':
                append_flag = 1;
                break;

            default:
              print_usage(argv[0]);
        }
    } while (opt != -1);

    if ( append_flag == 1 ) {
        fpOutput = fopen(out_keyfile, "a");
    }
    else {
        fpOutput = fopen(out_keyfile, "w");
    }

    if ( fpOutput == NULL ) {
        fprintf(stderr, "cannot open output file '%s'\n", out_keyfile);
        exit(EXIT_FAILURE);
    }

    if ( stat(keydb_file, &fsCheck) == 0 ) {
        if ( convertKeyDB2Config(keydb_file, fpOutput) ) {
            fprintf(stderr, "while convert keydb '%s' -> '%s'\n", 
                    keydb_file, out_keyfile);
        }
    }
    else {
        fprintf(stderr, "file '%s' missing\n", keydb_file);
    }

    if ( stat(devkey_file, &fsCheck) == 0 ) {
        if ( convertDeviceKey(devkey_file, fpOutput) ) {
            fprintf(stderr, "while convert device key '%s' -> '%s'\n",
                    keydb_file, out_keyfile);
        }
    }
    else {
        fprintf(stderr, "file '%s' missing\n", devkey_file);
    }

    if ( fpOutput ) {
        fclose(fpOutput);
    }

    return -1;
}
