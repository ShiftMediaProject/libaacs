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

#include "keydbcfg.h"

#include "util/strutl.h"
#include "util/logging.h"
#include "util/macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define USER_CFG_DIR   "/.libaacs/"
#define SYSTEM_CFG_DIR "/etc/libaacs/"

#define CFG_FILE_NAME  "KEYDB.cfg"


char *keydbcfg_find_config_file(void)
{
    static const char cfg_file_user[]   = USER_CFG_DIR   CFG_FILE_NAME;
    static const char cfg_file_system[] = SYSTEM_CFG_DIR CFG_FILE_NAME;

    const char *userhome = getenv("HOME");

    char *cfg_file = str_printf("%s%s", userhome, cfg_file_user);

    FILE *fp = fopen(cfg_file, "r");
    if (!fp) {

        cfg_file = (char*)realloc(cfg_file, sizeof(cfg_file_system));
        strcpy(cfg_file, cfg_file_system);

        fp = fopen(cfg_file, "r");
        if (!fp) {
            DEBUG(DBG_FILE, "No config file found!\n");
            X_FREE(cfg_file);
            return NULL;
        }
    }

    fclose(fp);

    DEBUG(DBG_FILE, "found config file: %s\n", cfg_file);
    return cfg_file;
}

