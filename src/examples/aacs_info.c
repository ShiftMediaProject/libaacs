/*
 * This file is part of libaacs
 * Copyright (C) 2010  npzacs
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

#include "../libaacs/aacs.h"
#include "../util/macro.h"  /* MKINT_BE48 */

#include <stdio.h>
#include <stdlib.h>

static const char *_hex2str(const uint8_t *s, unsigned n)
{
    static const char hex[] = "0123456789ABCDEF";
    static char *str = NULL;

    unsigned ii;

    str = realloc(str, n*2 + 1);
    for (ii = 0; ii < n; ii++) {
        str[2*ii]     = hex[ s[ii] >> 4];
        str[2*ii + 1] = hex[ s[ii] & 0x0f];
    }
    str[2*ii] = 0;

    return str;
}

static const char *_error_str(int error_code)
{
  switch (error_code) {
      case AACS_SUCCESS:
        return "No errors";
      case AACS_ERROR_CORRUPTED_DISC:
        return "Corrupted BluRay disc";
      case AACS_ERROR_NO_CONFIG:
        return "Missing configuration file(s)";
      case AACS_ERROR_NO_PK:
        return "No matching processing key in configuration file(s)";
      case AACS_ERROR_NO_CERT:
        return "No valid certificates in configuration file(s)";
      case AACS_ERROR_CERT_REVOKED:
        return "All available certificates have been revoked";
      case AACS_ERROR_MMC_OPEN:
        return "MMC open failed";
      case AACS_ERROR_MMC_FAILURE:
        return "MMC authentication failed";
  }
  return "Unknown error";
}

static void _dump_rl(const char *type, AACS_RL_ENTRY *rl, int num_entries, int mkb_version)
{
    int ii;

    printf("%s Revocation List  (MKB version %d):\n", type, mkb_version);

    if (num_entries < 1 || !rl) {
        printf("  (empty)\n");
        return;
    }

    for (ii = 0; ii < num_entries; ii++) {
        unsigned long long id = MKINT_BE48(rl[ii].id);

        if (rl[ii].range) {
            printf("  %012llx - %012llx\n", id, id + rl[ii].range);
        } else {
            printf("  %012llx\n", id);
        }
    }
}

int main (int argc, char **argv)
{
    int major, minor, micro, error_code = AACS_SUCCESS;

    if (argc < 2) {
        fprintf(stderr, "Usage: aacs_info <path-to-disc-root> [<path-to-config-file>]\n");
	exit(EXIT_FAILURE);
    }

    aacs_get_version(&major, &minor, &micro);
    printf("Opening %s using libaacs %d.%d.%d ...\n", argv[1], major, minor, micro);

    AACS *aacs = aacs_open2(argv[1], argc > 2 ? argv[2] : NULL, &error_code);
    if (error_code) {
        fprintf(stderr, "libaacs open failed: %s\n", _error_str(error_code));
    } else {
        printf("libaacs open succeed.\n");
    }

    if (!aacs) {
	exit(EXIT_FAILURE);
    }

    const uint8_t *vid = aacs_get_vid(aacs);
    const uint8_t *id  = aacs_get_disc_id(aacs);
    const uint8_t *pmsn = aacs_get_pmsn(aacs);
    const int      bec  = aacs_get_bus_encryption(aacs);
    printf("Disc ID: %s\n", id  ? _hex2str(id,  20) : "???");
    printf("VID    : %s\n", vid ? _hex2str(vid, 16) : "???");
    printf("MKBv   : %d\n", aacs_get_mkb_version(aacs));
    printf("PMSN   : %s\n", pmsn ? _hex2str(pmsn, 16) : "???");
    printf("Bus encryption:\n");
    printf("  Device support:   %s\n", (bec & AACS_BUS_ENCRYPTION_CAPABLE) ? "yes" : "no");
    printf("  Enabled in media: %s\n", (bec & AACS_BUS_ENCRYPTION_ENABLED) ? "yes" : "no");

    aacs_close(aacs);

    /* dump revocation lists */

    AACS_RL_ENTRY *rl;
    int num_entries, mkb_version;

    rl = aacs_get_hrl(&num_entries, &mkb_version);
    _dump_rl("Host", rl, num_entries, mkb_version);
    X_FREE(rl);

    rl = aacs_get_drl(&num_entries, &mkb_version);
    _dump_rl("Drive", rl, num_entries, mkb_version);
    X_FREE(rl);

    return EXIT_SUCCESS;
}
