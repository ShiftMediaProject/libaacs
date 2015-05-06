/*
 * This file is part of libaacs
 * Copyright (C) 2010-2013  npzacs
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

#include "dirs.h"
#include "file.h"

#include "util/strutl.h"
#include "util/logging.h"
#include "util/macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#define CFG_DIR        "aacs"

#define CFG_FILE_NAME  "KEYDB.cfg"
#define CERT_FILE_NAME "HostKeyCertificate.txt"
#define PK_FILE_NAME   "ProcessingDeviceKeysSimple.txt"

#define MIN_FILE_SIZE  20
#define MAX_FILE_SIZE  65535

static int _mkpath(const char *path)
{
    struct stat s;
    int result = 1;
    char *dir = str_dup(path);
    char *end = dir;

    if (!dir) {
        return -1;
    }

#ifdef _WIN32
    end += 2; /* skip drive */
#endif
    while (*end == DIR_SEP_CHAR)
        end++;

    while ((end = strchr(end, DIR_SEP_CHAR))) {
        *end = 0;

        if (stat(dir, &s) != 0 || !S_ISDIR(s.st_mode)) {
            BD_DEBUG(DBG_FILE, "Creating directory %s\n", dir);

            if (file_mkdir(dir) == -1) {
                BD_DEBUG(DBG_FILE | DBG_CRIT, "Error creating directory %s\n", dir);
                result = 0;
                break;
            }
        }

        *end++ = DIR_SEP_CHAR;
    }

    X_FREE(dir);

    return result;
}

static char *_load_file(FILE *fp)
{
    char *data = NULL;
    long file_size, read_size;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < MIN_FILE_SIZE || file_size > MAX_FILE_SIZE) {
        BD_DEBUG(DBG_FILE, "Invalid file size\n");
        return NULL;
    }

    data      = malloc(file_size + 1);
    read_size = fread(data, 1, file_size, fp);

    if (read_size != file_size) {
        BD_DEBUG(DBG_FILE, "Error reading file\n");
        X_FREE(data);
        return NULL;
    }

    data[file_size] = 0;

    return data;
}

static char *_config_file_user(const char *file_name)
{
    char *cfg_dir = file_get_config_home();
    char *result;

    if (!cfg_dir) {
        return NULL;
    }

    result = str_printf("%s"DIR_SEP"%s"DIR_SEP"%s", cfg_dir, CFG_DIR, file_name);
    X_FREE(cfg_dir);
    return result;
}

static FILE *_open_cfg_file_user(const char *file_name, char **path, const char *mode)
{
    char *cfg_file = _config_file_user(file_name);

    if (!cfg_file) {
        return NULL;
    }

    if (*mode == 'w') {
        if (!_mkpath(cfg_file)) {
            X_FREE(cfg_file);
            return NULL;
        }
    }

    FILE *fp = fopen(cfg_file, mode);

    BD_DEBUG(DBG_FILE, fp ? "Opened %s for %s\n" : "%s not found\n", cfg_file, mode);

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

    while (NULL != (dir = file_get_config_system(dir))) {

        char *cfg_file = str_printf("%s"DIR_SEP"%s"DIR_SEP"%s", dir, CFG_DIR, file_name);
        if (!cfg_file) {
            continue;
        }

        FILE *fp = fopen(cfg_file, "r");
        if (fp) {
            BD_DEBUG(DBG_FILE, "Reading %s\n", cfg_file);

            if (path) {
                *path = cfg_file;
            } else {
                X_FREE(cfg_file);
            }

            return fp;
        }

        BD_DEBUG(DBG_FILE, "%s not found\n", cfg_file);
        X_FREE(cfg_file);
    }

    return NULL;
}

static int _is_duplicate_pk(pk_list *list, const uint8_t *e)
{
    while (list) {
        if (!memcmp(list->key, e, 16)) {
            return 1;
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

            if (str) {
                BD_DEBUG(DBG_FILE, "Found processing key %s\n", str);

                pk_list *e = calloc(1, sizeof(pk_list));

                hexstring_to_hex_array(e->key, 16, str);

                if (_is_duplicate_pk(cf->pkl, e->key)) {
                    BD_DEBUG(DBG_FILE, "Skipping duplicate processing key %s\n", str);
                    X_FREE(e);
                } else {
                    e->next = cf->pkl;
                    cf->pkl = e;
                }

                result++;
            }
            X_FREE(str);

            p = str_next_line(p);
        }

        X_FREE(data);
    }

    return result;
}

static int _is_duplicate_cert(cert_list *list, cert_list *e)
{
    while (list) {
        if (!memcmp(list->host_priv_key, e->host_priv_key, 20) &&
            !memcmp(list->host_cert,     e->host_cert,     92)) {

            return 1;
        }
        list = list->next;
    }

  return 0;
}

static int _parse_cert_file(config_file *cf, FILE *fp)
{
    char *data   = _load_file(fp);
    int   result = 0;

    if (data) {
        const char *p = data;

        char *host_cert, *host_priv_key;
        host_priv_key = str_get_hex_string(p, 2*20);
        p = str_next_line(p);
        host_cert = str_get_hex_string(p, 2*92);

        X_FREE(data);

        if (!host_priv_key || !host_cert) {
            BD_DEBUG(DBG_FILE, "Invalid file\n");

        } else {
            BD_DEBUG(DBG_FILE, "Found certificate: %s %s\n", host_priv_key, host_cert);

            cert_list  *e = calloc(1, sizeof(cert_list));
            hexstring_to_hex_array(e->host_priv_key, 20, host_priv_key);
            hexstring_to_hex_array(e->host_cert, 92, host_cert);

            if (_is_duplicate_cert(cf->host_cert_list, e)) {
                BD_DEBUG(DBG_FILE, "Skipping duplicate certificate entry %s %s\n", host_priv_key, host_cert);
                X_FREE(e);
            } else {
                e->next = cf->host_cert_list;
                cf->host_cert_list = e;
                result = 1;
            }
        }
        X_FREE(host_priv_key);
        X_FREE(host_cert);
    }

    return result;
}

static int _is_duplicate_dk(dk_list *list, dk_list *e)
{
    while (list) {
      if (!memcmp(list, e, sizeof(*e))) {
            return 1;
        }
        list = list->next;
    }

    return 0;
}

static int _load_pk_file(config_file *cf)
{
    static const char pk_file_name[] = PK_FILE_NAME;
    FILE *fp;
    int result = 0;

    fp = _open_cfg_file_user(pk_file_name, NULL, "r");
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

static int _load_cert_file(config_file *cf)
{
    static const char cert_file_name[] = CERT_FILE_NAME;
    FILE *fp;
    int result = 0;

    fp = _open_cfg_file_user(cert_file_name, NULL, "r");
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

static char *_keycache_file(const char *type, const uint8_t *disc_id)
{
    char *cache_dir = file_get_cache_home();
    char *result;
    char disc_id_str[41];

    if (!cache_dir) {
        return NULL;
    }

    hex_array_to_hexstring(disc_id_str, disc_id, 20);

    result = str_printf("%s"DIR_SEP"%s"DIR_SEP"%s"DIR_SEP"%s", cache_dir, CFG_DIR, type, disc_id_str);
    X_FREE(cache_dir);
    return result;
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
                    BD_DEBUG(DBG_FILE, "Wrote %s to %s\n", type, file);
                    result = 1;

                } else {
                    BD_DEBUG(DBG_FILE, "Error writing to %s\n", file);
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

            BD_DEBUG(DBG_FILE, "Reading %s\n", file);

            if (fread(key_str, 1, len*2, fp) == len*2) {

                result = hexstring_to_hex_array(key, len, key_str);
                if (!result) {
                    BD_DEBUG(DBG_FILE, "Error converting %s\n", file);
                }

            } else {
              BD_DEBUG(DBG_FILE, "Error reading from %s\n", file);
            }

            X_FREE(key_str);

            fclose(fp);

        } else {
            BD_DEBUG(DBG_FILE, "%s not found\n", file);
        }

        X_FREE(file);
    }

    return result;
}

static char *_cache_file(const char *name)
{
    char *cache_dir = file_get_cache_home();
    char *result;

    if (!cache_dir) {
        return NULL;
    }

    result = str_printf("%s"DIR_SEP"%s"DIR_SEP"%s", cache_dir, CFG_DIR, name);
    X_FREE(cache_dir);
    return result;
}

int cache_save(const char *name, uint32_t version, const void *data, uint32_t len)
{
    int result = 0;
    char *file = _cache_file(name);

    if (file) {
        if (_mkpath(file)) {
            FILE *fp = fopen(file, "w");

            if (fp) {
                if (fwrite(&version, 1, 4, fp) == 4 &&
                    fwrite(&len, 1, 4, fp) == 4 &&
                    fwrite(data, 1, len, fp) == len) {
                    BD_DEBUG(DBG_FILE, "Wrote %d bytes to %s\n", len + 8, file);
                    result = 1;

                } else {
                    BD_DEBUG(DBG_FILE, "Error writing to %s\n", file);
                }

                fclose(fp);
            }
        }

        X_FREE(file);
    }

    return result;
}

int cache_get(const char *name, uint32_t *version, uint32_t *len, void *buf)
{
    int result = 0;
    char *file = _cache_file(name);

    *version = 0;
    if (len) {
        *len = 0;
    }

    if (file) {
        FILE *fp = fopen(file, "r");

        if (fp) {
            BD_DEBUG(DBG_FILE, "Reading %s\n", file);

            if (fread(version, 1, 4, fp) == 4 &&
                (!len || fread(len, 1, 4, fp) == 4) &&
                (!buf || fread(buf, 1, *len, fp) == *len)) {

              BD_DEBUG(DBG_FILE, "Read %d bytes from %s, version %d\n", 4 + (len ? 4 : 0) + (buf ? *len : 0), file, *version);
              result = 1;

            } else {
              BD_DEBUG(DBG_FILE, "Error reading from %s\n", file);
            }

            fclose(fp);

        } else {
            BD_DEBUG(DBG_FILE, "%s not found\n", file);
        }

        X_FREE(file);
    }

    return result;
}

int cache_remove(const char *name)
{
    char *file = _cache_file(name);
    if (!file) {
        return 0;
    }
    int result = !remove(file);
    if (!result) {
        BD_DEBUG(DBG_FILE, "Error removing %s\n", file);
    }
    X_FREE(file);
    return result;
}

int config_save(const char *name, const void *data, uint32_t len)
{
    char *path = NULL;
    FILE *fp = _open_cfg_file_user(name, &path, "w");
    int result = 0;

    if (fp) {
        if (fwrite(&len, 1, 4, fp) == 4 &&
            fwrite(data, 1, len, fp) == len) {
          BD_DEBUG(DBG_FILE, "Wrote %d bytes to %s\n", len + 4, path);
          result = 1;

        } else {
            BD_DEBUG(DBG_FILE | DBG_CRIT, "Error writing to %s\n", path);
        }

        fclose(fp);
    }

    X_FREE(path);

    return result;
}

int config_get(const char *name, uint32_t *len, void *buf)
{
    char *path = NULL;
    FILE *fp = _open_cfg_file_user(name, &path, "r");
    int result = 0;
    uint32_t size = *len;

    *len = 0;

    if (fp) {
        BD_DEBUG(DBG_FILE, "Reading %s\n", path);

        if (fread(len, 1, 4, fp) == 4 && (size <= *len) &&
            (!buf || fread(buf, 1, *len, fp) == *len)) {

            BD_DEBUG(DBG_FILE, "Read %d bytes from %s\n", 4 + (buf ? *len : 0), path);
            result = 1;

        } else {
            BD_DEBUG(DBG_FILE | DBG_CRIT, "Error reading from %s\n", path);
        }

        fclose(fp);
    }

    X_FREE(path);

    return result;
}


static char *_find_config_file(void)
{
    static const char cfg_file_name[] = CFG_FILE_NAME;

    char       *cfg_file = NULL;
    FILE       *fp       = NULL;

    fp = _open_cfg_file_user(cfg_file_name, &cfg_file, "r");
    if (!fp) {
        fp = _open_cfg_file_system(cfg_file_name, &cfg_file);
    }

    if (fp) {
        BD_DEBUG(DBG_FILE, "found config file: %s\n", cfg_file);
        fclose(fp);
    }

    return cfg_file;
}

#include "keydb.h"

static int _parse_embedded(config_file *cf)
{
    int result = 0, jj;
    unsigned ii;

    /* reverse order to maintain key positions (items are added to list head) */
    for (jj = sizeof(internal_dk_list) / sizeof(internal_dk_list[0]) - 1; jj >= 0; --jj) {
        dk_list *e = calloc(1, sizeof(dk_list));

        decrypt_key(e->key, internal_dk_list[jj], 16);
        e->node = internal_device_number;
        e->uv   = MKINT_BE32(internal_dk_list[jj] + 16);
        e->u_mask_shift = internal_dk_list[jj][20];

        if (!e->uv || _is_duplicate_dk(cf->dkl, e)) {
            X_FREE(e);

        } else {
            e->next = cf->dkl;
            cf->dkl = e;
            result++;
        }
    }

    for (ii = 0; ii < sizeof(internal_pk_list) / sizeof(internal_pk_list[0]); ii++) {
        pk_list *e = calloc(1, sizeof(pk_list));

        decrypt_key(e->key, internal_pk_list[ii], 16);

        if (_is_duplicate_pk(cf->pkl, e->key)) {
            X_FREE(e);

        } else {
            e->next = cf->pkl;
            cf->pkl = e;
            result++;
        }
    }

    for (ii = 0; ii < sizeof(internal_hc_list) / sizeof(internal_hc_list[0]); ii++) {
        cert_list  *e = calloc(1, sizeof(cert_list));

        decrypt_key(e->host_priv_key, internal_hc_list[ii],      20);
        decrypt_key(e->host_cert,     internal_hc_list[ii] + 20, 92);

        if (_is_duplicate_cert(cf->host_cert_list, e)) {
            X_FREE(e);

        } else {
            e->next = cf->host_cert_list;
            cf->host_cert_list = e;
            result++;
        }
    }

    return result;
}

config_file *keydbcfg_config_load(const char *configfile_path)
{
    int config_ok = 0;

    config_file *cf = keydbcfg_new_config_file();

    /* try to load KEYDB.cfg */

    if (configfile_path) {
        config_ok = keydbcfg_parse_config(cf, configfile_path);

    } else {
        /* If no configfile path given, check for config files in user's home or
         * under /etc.
         */
        char *cfgfile = _find_config_file();
        config_ok = keydbcfg_parse_config(cf, cfgfile);
        X_FREE(cfgfile);
    }

    /* Try to load simple (aacskeys) config files */

    config_ok = _load_pk_file(cf)   || config_ok;
    config_ok = _load_cert_file(cf) || config_ok;

    /* embedded keys */
    config_ok = _parse_embedded(cf) || config_ok;

    if (!config_ok) {
        BD_DEBUG(DBG_AACS | DBG_CRIT, "No valid AACS configuration files found\n");
        keydbcfg_config_file_close(cf);
        return NULL;
    }

    return cf;
}

