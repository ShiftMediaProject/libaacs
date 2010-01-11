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

#define OPTS "c:d:H:o:hai"

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

typedef int TConvAction(const char *infile, FILE *outfile);


static char append_flag = 0;
static char interactive_flag = 0;
static const char *default_keydb   = "keydb.cfg";
static const char *default_devkey  = "ProcessingDeviceKeysSimple.txt";
static const char *default_hostkey = "HostKeyCertificate.txt";
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
 * small helper function, check infile and execute convert function
 */
static int checkAndExecute(TConvAction *action, const char *infile, FILE *outfile)
{
    struct stat fsCheck;

    if ( action && infile && infile[0] && outfile ) {
    
        if ( stat(infile, &fsCheck) == 0 ) {
            if ( action(infile, outfile) ) {
                fprintf(stderr, "while convert file '%s'\n", infile);
                return -1;
            }
        }
        else {
            fprintf(stderr, "file '%s' missing\n", infile);
            return -1;
        }
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
                    fprintf(stderr, "error while write record %zd\n", count + 1);
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


static int convertHostKeyCertificate(const char *hostkey, FILE *fpOutfile)
{
    FILE *fpHostKey = NULL;
    size_t index = 0;
    int    length = 0;
    size_t nwrite;
    unsigned char buffer[256];
    size_t data_length;
    RECORDHEADER header;

    if ( fpOutfile && (fpHostKey = fopen(hostkey, "r")) ) {
 
        /* seek the the last position */
        fseek(fpOutfile, 0, SEEK_END);

        /* read  */
        while (!feof(fpHostKey)) {
            if ( fgets(buffer, sizeof buffer, fpHostKey)) {

		switch(index) {
                    case 0: { /* line 1 = KF_HOST_PRIV_KEY */
                        data_length = 20;
                        unsigned char hex_data[data_length];

                        if ( strlen(buffer) >= data_length ) {
                            memset(&header, 0, sizeof header);
                            buffer[data_length] = '\0';
                            header.type = KF_HOST_PRIV_KEY;
                            header.entry_length[3] = 0x14;
                            header.count[1] = 0x01;
                            header.length[2] = header.entry_length[3] + 10;

                            /* write record header */
                            nwrite = fwrite(&header, sizeof (char), sizeof header, fpOutfile);
                            if ( nwrite != sizeof header ) {
                                /* print error message and stop process */
                                fprintf(stderr, "error while write file header\n");
                                return -1;
                            }

                            /* convert data string into hex data */
                            if ( ascii2hex(buffer, hex_data, data_length) ) {
                                fprintf(stderr, "error while convert data buffer\n");
                                return -1;
                            }

                            /* write data buffer (20 bytes) */
                            nwrite = fwrite(hex_data, sizeof (char), data_length, fpOutfile);
                            if ( nwrite != data_length ) {
                                /* print error message and stop process */
                                fprintf(stderr, "error while write record data\n");
                                return -1;
                            }
                        }
                    
                        break;
                    }
                    case 1: { /* line 2 = KF_HOST_CERT */
                        data_length = 92;
                        unsigned char hex_data[data_length];

                        if ( strlen(buffer) >= data_length ) {
                            memset(&header, 0, sizeof header);
                            buffer[data_length] = '\0';
                            header.type = KF_HOST_CERT;
                            header.entry_length[3] = 0x5C;
                            header.count[1] = 0x01;
                            header.length[2] = header.entry_length[3] + 10;

                            /* write record header */
                            nwrite = fwrite(&header, sizeof (char), sizeof header, fpOutfile);
                            if ( nwrite != sizeof header ) {
                                /* print error message and stop process */
                                fprintf(stderr, "error while write file header\n");
                                return -1;
                            }

                            /* convert data string into hex data */
                            if ( ascii2hex(buffer, hex_data, data_length) ) {
                                fprintf(stderr, "error while convert data buffer\n");
                                return -1;
                            }

                            /* write data buffer (92 bytes) */
                            nwrite = fwrite(hex_data, sizeof (char), data_length, fpOutfile);
                            if ( nwrite != data_length ) {
                                /* print error message and stop process */
                                fprintf(stderr, "error while write file header\n");
                                return -1;
                            }
                        }
                    }
                }
                index += 1;
            }
        }
    }

    /* close key db (input) file */
    if ( fpHostKey ) {
        fclose(fpHostKey);
    }

    return 0;
}


/*
 * 
 */
static int startInteractiveMode(const char *input, FILE *fpOutfile)
{
    FILE *fpInput;
    char b_loop = 0;
    long idx;
    unsigned int type;
    long record_count;
    long record_length = -1;
    char buffer[1024];
    char hex_data[512];
    char *pos;
    RECORDHEADER hdr;

    /* check interactive flag and read information */
    if ( interactive_flag && fpOutfile && input && input[0] ) {

        fpInput = fopen(input, "r");
        if ( fpInput ) {
            do {
                memset(buffer, 0, sizeof buffer);
                memset(&hdr, 0, sizeof hdr);

                /* seek the the last position */
                fseek(fpOutfile, 0, SEEK_END);

                printf("Please enter a record type: ");
                if ( !fgets(buffer, sizeof buffer, fpInput) ) {
                    fprintf(stderr, "while read record type\n");
                    return -1;
                }

                hdr.type =  atol(buffer);
                
                printf("How many entries? ");
                if ( !fgets(buffer, sizeof buffer, fpInput) ) {
                    fprintf(stderr, "while read record count\n");
                    return -1;
                }

                record_count = atol(buffer);

                /* store record count in header */
                hdr.count[0] = record_count >> 8;
                hdr.count[1] = record_count & 0xFF;

                for ( idx = 1; idx <= record_count; idx += 1)
                {
                    memset(buffer, 0, sizeof buffer);
                    printf("Enter entry #%ld in ascii-hex: ", idx);
                    if ( !fgets(buffer, sizeof buffer, fpInput) ) {
                        fprintf(stderr, "while read record data (index = %ld)\n", idx);
                        return -1;
                    }

                    if ( !(pos = strrchr(buffer, '\0'))) {
                        pos[0] = '\0';
                    }

                    if ( idx == 1 ) {
                        /* read first record */
                        record_length = strlen(buffer) / 2;
                        hdr.length[0] = record_length + 10 >> 16;
                        hdr.length[1] = record_length + 10 >> 8;
                        hdr.length[2] = record_length + 10 & 0xFF;
                        hdr.entry_length[0] = record_length >> 24;
                        hdr.entry_length[1] = record_length >> 16;
                        hdr.entry_length[2] = record_length >> 8;
                        hdr.entry_length[3] = record_length & 0xFF;

                        fwrite(&hdr, 1, sizeof hdr, fpOutfile);
                    }

                    if ( ascii2hex(buffer, hex_data, record_length) ) {
                        fprintf(stderr, "error while convert data buffer\n");
                        return -1;
                    }

                    if ( fwrite(hex_data, sizeof(char), record_length, fpOutfile) != record_length ) {
                        fprintf(stderr, "error while write record data (%ld bytes)\n", record_length);
                        return -1;
                    }
                }

                printf("Would you like to enter another entry (y/n)? ");
                if ( (fgets(buffer, sizeof buffer, fpInput))) {
                    if ( buffer[0] == 'y' || buffer[0] == 'Y' ) {
                        b_loop = 1;
                    }
                    else {
                        b_loop = 0;
                    }
                }
            } while ( b_loop );
        }
    }
    
    return 0;
}



/*
 * small usage function
 */
static int print_usage(const char *name)
{
    fprintf(stderr, "Usage: %s -a -h -i -c <in_keydb> -d <in_devkey> -H <in_hostkey> -o <out_keyfile>\n"
                    "    -a                append key information\n"
                    "    -h                print this usage\n"
                    "    -i                start interactive mode\n"
                    "    -c <in_keydb>     keydb input file (default: %s)\n"
                    "    -d <in_devkey>    device key input file (default: %s)\n"
                    "    -H <in_hostkey>   host key input file (default: %s)\n"
                    "    -o <out_keyfile>  keyfile file name (default: %s)\n", 
                    name, default_keydb, default_devkey, default_hostkey, default_outfile);
    exit(EXIT_FAILURE);
}


/*
 * main function
 */
int main(int argc, char *argv[])
{
    FILE *fpOutput;
    char keydb_file[512];
    char devkey_file[512];
    char hostkey_file[512];
    char out_keyfile[512];
    struct stat fsCheck;
    int opt;

    /* init file buffer with default values */
    strncpy(keydb_file,   default_keydb, sizeof keydb_file);
    strncpy(devkey_file,  default_devkey, sizeof devkey_file);
    strncpy(hostkey_file, default_hostkey, sizeof hostkey_file);
    strncpy(out_keyfile,  default_outfile, sizeof out_keyfile);

    /* parse all given arguments */
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

            case 'H':
                strncpy(hostkey_file, optarg, sizeof hostkey_file); 
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

            case 'i':
                interactive_flag = 1;
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

    /* check and convert KEYDB.cfg file into internal config structure */
    checkAndExecute(convertKeyDB2Config, keydb_file, fpOutput);

    /* check and convert DeviceKey file into internal config structure */
    checkAndExecute(convertDeviceKey, devkey_file, fpOutput);

    /* check and convert HostKeyCertificate file info internal config structure */
    checkAndExecute(convertHostKeyCertificate, hostkey_file, fpOutput);

    /* check interactive flag and start interactive mode */
    checkAndExecute(startInteractiveMode, "/dev/stdin", fpOutput);

    /* close current output file */
    if ( fpOutput ) {
        fclose(fpOutput);
    }

    return -1;
}

