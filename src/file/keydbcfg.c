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

#ifndef _WIN32
# include "xdg.h"
# define get_config_home xdg_get_config_home
# define get_cache_home xdg_get_cache_home
# define get_config_system xdg_get_config_system
#else
# include "win32.h"
# define get_config_home win32_get_config_home
# define get_cache_home win32_get_config_home
# define get_config_system win32_get_config_system
#endif

#include "util/strutl.h"
#include "util/logging.h"
#include "util/macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
# define mkdir(p,m) win32_mkdir(p)
#endif


#define CFG_DIR        "aacs"

#define CFG_FILE_NAME  "KEYDB.cfg"
#define CERT_FILE_NAME "HostKeyCertificate.txt"
#define PK_FILE_NAME   "ProcessingDeviceKeysSimple.txt"

#define MIN_FILE_SIZE  20
#define MAX_FILE_SIZE  65535


static char *_load_file(FILE *fp)
{
    char *data = NULL;
    long file_size, read_size;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < MIN_FILE_SIZE || file_size > MAX_FILE_SIZE) {
        DEBUG(DBG_FILE, "Invalid file size\n");
        return NULL;
    }

    data      = malloc(file_size + 1);
    read_size = fread(data, 1, file_size, fp);

    if (read_size != file_size) {
        DEBUG(DBG_FILE, "Error reading file\n");
        X_FREE(data);
        return NULL;
    }

    data[file_size] = 0;

    return data;
}

static FILE *_open_cfg_file_user(const char *file_name, char **path)
{
    const char *cfg_dir = get_config_home();

    if (!cfg_dir) {
        return NULL;
    }

    char *cfg_file = str_printf("%s/%s/%s", cfg_dir, CFG_DIR, file_name);
    FILE *fp       = fopen(cfg_file, "r");

    DEBUG(DBG_FILE, fp ? "Reading %s\n" : "%s not found\n", cfg_file);

    if (fp && path) {
        *path = cfg_file;
    } else {
        X_FREE(cfg_file);
    }

    return fp;
}

static FILE *_open_cfg_file_system(const char *file_name, char **path)
{
    const char *dir = NULL;

    while (NULL != (dir = get_config_system(dir))) {

        char *cfg_file = str_printf("%s/%s/%s", dir, CFG_DIR, file_name);

        FILE *fp = fopen(cfg_file, "r");
        if (fp) {
            DEBUG(DBG_FILE, "Reading %s\n", cfg_file);

            if (path) {
                *path = cfg_file;
            } else {
                X_FREE(cfg_file);
            }

            return fp;
        }

        DEBUG(DBG_FILE, "%s not found\n", cfg_file);
        X_FREE(cfg_file);
    }

    return NULL;
}

static int _is_duplicate_pk(pk_list *list, const char *e)
{
    while (list) {
        if (list->key) {
            if (!memcmp(list->key, e, 2*16)) {
                return 1;
            }
        }
        list = list->next;
    }

    return 0;
}

static int _parse_pk_file(config_file *cf, FILE *fp)
{
    char *data   = _load_file(fp);
    int   result = 0;

    if (data) {
        const char *p = data;

        while (*p) {
            char *str = str_get_hex_string(p, 2*16);

            if (str && _is_duplicate_pk(cf->pkl, str)) {
                DEBUG(DBG_FILE, "Skipping duplicate processing key %s\n", str);
                X_FREE(str);

            } else if (str) {
                DEBUG(DBG_FILE, "Found processing key %s\n", str);

                pk_list *e = calloc(1, sizeof(pk_list));

                e->key  = str;
                e->next = cf->pkl;

                cf->pkl = e;

                result++;
            }

            p = str_next_line(p);
        }

        X_FREE(data);
    }

    return result;
}

static int _is_duplicate_cert(cert_list *list, cert_list *e)
{
    while (list) {
        if (list->host_priv_key && list->host_cert) {

            if (!memcmp(list->host_priv_key, e->host_priv_key, 2*20) &&
                !memcmp(list->host_cert,     e->host_cert,     2*92)) {

                return 1;
            }
        }
        list = list->next;
    }

  return 0;
}

static void _free_cert_entry(cert_list *e)
{
    X_FREE(e->host_priv_key);
    X_FREE(e->host_cert);
    X_FREE(e);
}

static int _parse_cert_file(config_file *cf, FILE *fp)
{
    char *data   = _load_file(fp);
    int   result = 0;

    if (data) {
        const char *p = data;
        cert_list  *e = calloc(1, sizeof(cert_list));

        e->host_priv_key = str_get_hex_string(p, 2*20);
        p = str_next_line(p);
        e->host_cert = str_get_hex_string(p, 2*92);

        X_FREE(data);

        if (!e->host_priv_key || !e->host_cert) {
            DEBUG(DBG_FILE, "Invalid file\n");
            _free_cert_entry(e);

        } else if (_is_duplicate_cert(cf->host_cert_list, e)) {
            DEBUG(DBG_FILE, "Skipping duplicate certificate entry %s %s\n", e->host_priv_key, e->host_cert);
            _free_cert_entry(e);

        } else {
            DEBUG(DBG_FILE, "Found certificate: %s %s\n", e->host_priv_key, e->host_cert);
            e->next = cf->host_cert_list;
            cf->host_cert_list = e;
            result = 1;
        }
    }

    return result;
}

int keydbcfg_load_pk_file(config_file *cf)
{
    static const char pk_file_name[] = PK_FILE_NAME;
    FILE *fp;
    int result = 0;

    fp = _open_cfg_file_user(pk_file_name, NULL);
    if (fp) {
        result += _parse_pk_file(cf, fp);
        fclose(fp);
    }

    fp = _open_cfg_file_system(pk_file_name, NULL);
    if (fp) {
        result += _parse_pk_file(cf, fp);
        fclose(fp);
    }

    return result;
}

int keydbcfg_load_cert_file(config_file *cf)
{
    static const char cert_file_name[] = CERT_FILE_NAME;
    FILE *fp;
    int result = 0;

    fp = _open_cfg_file_user(cert_file_name, NULL);
    if (fp) {
        result += _parse_cert_file(cf, fp);
        fclose(fp);
    }

    fp = _open_cfg_file_system(cert_file_name, NULL);
    if (fp) {
        result += _parse_cert_file(cf, fp);
        fclose(fp);
    }

    return result;
}

static int _mkpath(const char *path)
{
    struct stat s;
    int result = 1;
    char *dir = str_printf("%s", path);
    char *end = dir;

    while (*end == '/')
        end++;

    while ((end = strchr(end, '/'))) {
        *end = 0;

        if (stat(dir, &s) != 0 || !S_ISDIR(s.st_mode)) {
            DEBUG(DBG_FILE, "Creating directory %s\n", dir);

            if (mkdir(dir, S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
                DEBUG(DBG_FILE, "Error creating directory %s\n", dir);
                result = 0;
                break;
            }
        }

        *end++ = '/';
    }

    X_FREE(dir);

    return result;
}

char *_keycache_file(const char *type, const uint8_t *disc_id)
{
    const char *cache_dir = get_cache_home();
    char disc_id_str[41];

    if (!cache_dir) {
        return NULL;
    }

    hex_array_to_hexstring(disc_id_str, disc_id, 20);

    return str_printf("%s/%s/%s/%s", cache_dir, CFG_DIR, type, disc_id_str);
}

int keycache_save(const char *type, const uint8_t *disc_id, const uint8_t *key, unsigned int len)
{
    int result = 0;
    char *file = _keycache_file(type, disc_id);

    if (file) {
        if (_mkpath(file)) {
            FILE *fp = fopen(file, "w");

            if (fp) {
                char *key_str = calloc(1, len*2 + 1);
                hex_array_to_hexstring(key_str, key, len);

                if (fwrite(key_str, 1, len*2, fp) == len*2) {
                    DEBUG(DBG_FILE, "Wrote %s to %s\n", type, file);
                    result = 1;

                } else {
                    DEBUG(DBG_FILE, "Error writing to %s\n", file);
                }

                free(key_str);

                fclose(fp);
            }
        }

        X_FREE(file);
    }

    return result;
}

int keycache_find(const char *type, const uint8_t *disc_id, uint8_t *key, unsigned int len)
{
    int result = 0;
    char *file = _keycache_file(type, disc_id);

    if (file) {
        FILE *fp = fopen(file, "r");

        if (fp) {
            char *key_str = malloc(len*2);

            DEBUG(DBG_FILE, "Reading %s\n", file);

            if (fread(key_str, 1, len*2, fp) == len*2) {

                result = hexstring_to_hex_array(key, len, key_str);
                if (!result) {
                    DEBUG(DBG_FILE, "Error converting %s\n", file);
                }

            } else {
              DEBUG(DBG_FILE, "Error reading from %s\n", file);
            }

            free(key_str);

            fclose(fp);

        } else {
            DEBUG(DBG_FILE, "%s not found\n", file);
        }

        X_FREE(file);
    }

    return result;
}

char *keydbcfg_find_config_file(void)
{
    static const char cfg_file_name[] = CFG_FILE_NAME;

    char       *cfg_file = NULL;
    FILE       *fp       = NULL;

    fp = _open_cfg_file_user(cfg_file_name, &cfg_file);
    if (!fp) {
        fp = _open_cfg_file_system(cfg_file_name, &cfg_file);
    }

    if (fp) {
        DEBUG(DBG_FILE, "found config file: %s\n", cfg_file);
        fclose(fp);
    }

    return cfg_file;
}

