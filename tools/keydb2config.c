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

#define OPTS "c:d:"

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
typedef struct confheader
{
    unsigned char type;              /* Record type in hex */
    unsigned char length[3];         /* Entire record length inc header */
    unsigned char count[2];          /* Record count */
    unsigned char entry_length[4];   /* Record length */
} CONFHEADER;


/* config record structure */
typedef struct confrecord
{
    unsigned char key1[20];     /* first key */
    unsigned char info[10];     /* movie name / info */
    unsigned char key2[16];     /* second key */
} CONFRECORD;


/* config header structure */
typedef struct devkeyheader
{
    unsigned char byte0;             /* Record type in hex */
    unsigned char length[3];         /* Number of entries in dec */
    unsigned char count[2];          /* Record counts */
    unsigned char entry_length[4];   /* Record length */
} DEVKEYHEADER;



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
static int convertKeyDB2Config(const char *keydb, const char *conffile)
{
    FILE *fpKeyDB = NULL;
    FILE *fpConfig = NULL;
    int count = 0;
    size_t length = 0;
    size_t nwrite;
    const char *search_key2 = " | V | ";
    char *key2 = NULL;
    char buffer[512];
    CONFHEADER header;
    CONFRECORD record;

    if ( (fpKeyDB = fopen(keydb, "r")) && 
         (fpConfig = fopen(conffile, "w")) ) {
 
        memset(&header, 0, sizeof header);
        header.type = 0x07;
        header.entry_length[3] = 0x2E;
 
        nwrite = fwrite(&header, sizeof (char), sizeof header, fpConfig);
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
                    nwrite = fwrite(&record, sizeof (char), sizeof record, fpConfig);
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
	if (fseek(fpConfig, 0, SEEK_SET) == 0 ) {
            /* write the new header with all data */
            nwrite = fwrite(&header, sizeof (char), sizeof header, fpConfig);
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

    /* close config (output) file */
    if ( fpConfig ) {
        fclose(fpConfig);
    }

    return 0;
}



static int convertDeviceKey(const char *devkey, const char *outfile)
{
    FILE *fpDevKey = NULL;
    FILE *fpOutfile = NULL;
    size_t count = 0;
    size_t length = 0;
    size_t nwrite;
    char buffer[64];
    DEVKEYHEADER header;
    unsigned char record[16];

    if ( (fpDevKey = fopen(devkey, "r")) && 
         (fpOutfile = fopen(outfile, "w")) ) {
 
        memset(&header, 0, sizeof header);
        header.byte0 = 2;
        header.entry_length[3] = 0x10;
 
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

	if (fseek(fpOutfile, 0, SEEK_SET) == 0 ) {
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

    /* close config (output) file */
    if ( fpOutfile ) {
        fclose(fpOutfile);
    }

    return 0;
}


static int print_usage(const char *name)
{
    fprintf(stderr, "Usage: %s -c <keydb> -d <devkey>\n"
                    "         <keydb>       export file name for keydb\n"
                    "         <devkey>      export file name for device key\n", name);
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    const char *keydb_file = "keydb.cfg";
    char keydb_outfile[512];
    const char *devkey_file = "ProcessingDeviceKeysSimple.txt";
    char devkey_outfile[512];
    struct stat fsCheck;
    int opt;


    strncpy(keydb_outfile,  "_gen_config.db", sizeof keydb_outfile);
    strncpy(devkey_outfile, "_gen_devkey.db", sizeof devkey_outfile);

    do {
        opt = getopt(argc, argv, OPTS);
        switch(opt) {
            case -1:
               break;

            case 'c':
               strncpy(keydb_outfile, optarg, sizeof keydb_outfile);
               break;

            case 'd':
               strncpy(devkey_outfile, optarg, sizeof devkey_outfile);
               break;

            default:
              print_usage(argv[0]);
        }
    } while (opt != -1);

    if ( stat(keydb_file, &fsCheck) == 0 ) {
        if ( convertKeyDB2Config(keydb_file, keydb_outfile) ) {
            fprintf(stderr, "while convert keydb '%s' -> '%s'\n", 
                    keydb_file, keydb_outfile);
        }
    }
    else {
        fprintf(stderr, "file '%s' missing\n", keydb_file);
    }

    if ( stat(devkey_file, &fsCheck) == 0 ) {
        if ( convertDeviceKey(devkey_file, devkey_outfile) ) {
            fprintf(stderr, "while convert device key '%s' -> '%s'\n",
                    keydb_file, keydb_outfile);
        }
    }
    else {
        fprintf(stderr, "file '%s' missing\n", devkey_file);
    }

    return -1;
}
