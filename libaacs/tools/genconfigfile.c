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
 *
 * In addition, as a special exception, the copyright holders of libaacs
 * gives permission to link the code of its release of libaacs with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked library.  You must obey the GNU Lesser General Public
 * License in all respects for all of the code used other than "OpenSSL".
 * If you modify this file, you may extend this exception to your
 * version of the file, but you are not obligated to do so.  If you do
 * not wish to do so, delete this exception statement from your version.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// THIS IS BAAAAAD CODING AND CRASHES TOWARDS THE END BUT IT WRITES THE FILE!

int main(int argc, char *argv[])
{
    char s[1024];
    unsigned char header[10], s2[3], val;
    unsigned int type, entries;
    int i, add_entry = 1;

    printf("Generate libaacs config file builder tool\n\n");

    if (argc == 2) {
        FILE *fp = fopen(argv[1], "w");

        while (add_entry) {

            printf("Record type in hex (e.g. 0x01): ");
            scanf("%x", &type);

            printf("Number of entries in dec: ");
            scanf("%d", &entries);

            printf("Record data (all entries together in ascii): ");
            memset(s, 0, sizeof(s));
            scanf("%s", s);
            header[0] = type;
            header[1] = (strlen(s)/2 + 10) >> 16;
            header[2] = (strlen(s)/2 + 10) >> 8;
            header[3] = (strlen(s)/2 + 10) & 0xff;
            header[4] = entries >> 8;
            header[5] = entries & 0xff;
            header[6] = strlen(s)/2/entries >> 24;
            header[7] = strlen(s)/2/entries >> 16;
            header[8] = strlen(s)/2/entries >> 8;
            header[9] = strlen(s)/2/entries & 0xff;

            fwrite(header, 10, 1, fp);

            for (i = 0; i < strlen(s); i += 2) {
                s2[0] = s[i];
                s2[1] = s[i + 1];
                s2[2] = 0;
                val = strtol((char*)s2, NULL, 16);
                fputc(val, fp);
            }

            printf("Add another record [y/n]? ");
            scanf("%s", s);
            if ((s[0] != 'Y') && (s[0] != 'y'))
                add_entry = 0;
        }

        fclose(fp);
    } else {
        printf("Usage: %s /path/to/file.bin\n", argv[0]);
    }

    return 0;
}
