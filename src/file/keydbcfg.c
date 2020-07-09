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

#include <stdlib.h>
#include <string.h>


#define CFG_DIR        "aacs"

#define CFG_FILE_NAME  "KEYDB.cfg"
#define CERT_FILE_NAME "HostKeyCertificate.txt"
#define PK_FILE_NAME   "ProcessingDeviceKeysSimple.txt"

#define MIN_FILE_SIZE  20
#define MAX_FILE_SIZE  65535


static char *_load_file(AACS_FILE_H *fp)
{
    char *data = NULL;
    int64_t size;

    size = file_size(fp);
    if (size < MIN_FILE_SIZE || size > MAX_FILE_SIZE) {
        BD_DEBUG(DBG_FILE, "Invalid file size\n");
        return NULL;
    }

    data = malloc(size + 1);
    if (!data) {
        return NULL;
    }

    if (file_read(fp, data, size) != size) {
        BD_DEBUG(DBG_FILE, "Error reading file\n");
        X_FREE(data);
        return NULL;
    }

    data[size] = 0;

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

static AACS_FILE_H *_open_cfg_file_user(const char *file_name, char **path, const char *mode)
{
    char *cfg_file = _config_file_user(file_name);

    if (!cfg_file) {
        return NULL;
    }

    if (*mode == 'w') {
        if (file_mkdirs(cfg_file) < 0) {
            X_FREE(cfg_file);
            return NULL;
        }
    }

    AACS_FILE_H *fp = file_open(cfg_file, mode);

    BD_DEBUG(DBG_FILE, fp ? "Opened %s for %s\n" : "%s not found\n", cfg_file, mode);

    if (fp && path) {
        *path = cfg_file;
    } else {
        X_FREE(cfg_file);
    }

    return fp;
}

static AACS_FILE_H *_open_cfg_file_system(const char *file_name, char **path)
{
    const char *dir = NULL;

    while (NULL != (dir = file_get_config_system(dir))) {

        char *cfg_file = str_printf("%s"DIR_SEP"%s"DIR_SEP"%s", dir, CFG_DIR, file_name);
        if (!cfg_file) {
            continue;
        }

        AACS_FILE_H *fp = file_open(cfg_file, "r");
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

static int _parse_pk_file(config_file *cf, AACS_FILE_H *fp)
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
                if (e) {
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

static int _parse_cert_file(config_file *cf, AACS_FILE_H *fp)
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
            if (e) {
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
    AACS_FILE_H *fp;
    int result = 0;

    fp = _open_cfg_file_user(pk_file_name, NULL, "r");
    if (fp) {
        result += _parse_pk_file(cf, fp);
        file_close(fp);
    }

    fp = _open_cfg_file_system(pk_file_name, NULL);
    if (fp) {
        result += _parse_pk_file(cf, fp);
        file_close(fp);
    }

    return result;
}

static int _load_cert_file(config_file *cf)
{
    static const char cert_file_name[] = CERT_FILE_NAME;
    AACS_FILE_H *fp;
    int result = 0;

    fp = _open_cfg_file_user(cert_file_name, NULL, "r");
    if (fp) {
        result += _parse_cert_file(cf, fp);
        file_close(fp);
    }

    fp = _open_cfg_file_system(cert_file_name, NULL);
    if (fp) {
        result += _parse_cert_file(cf, fp);
        file_close(fp);
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
    char *key_str = calloc(2, len + 1);

    if (file && key_str) {
        if (!file_mkdirs(file)) {
            AACS_FILE_H *fp = file_open(file, "w");

            if (fp) {
                hex_array_to_hexstring(key_str, key, len);

                if (file_write(fp, key_str, len*2) == len*2) {
                    BD_DEBUG(DBG_FILE, "Wrote %s to %s\n", type, file);
                    result = 1;

                } else {
                    BD_DEBUG(DBG_FILE, "Error writing to %s\n", file);
                }


                file_close(fp);
            }
        }
    }

    X_FREE(key_str);
    X_FREE(file);

    return result;
}

int keycache_find(const char *type, const uint8_t *disc_id, uint8_t *key, unsigned int len)
{
    int result = 0;
    char *file = _keycache_file(type, disc_id);

    if (file) {
        AACS_FILE_H *fp = file_open(file, "r");

        if (fp) {
            char *key_str = malloc(len*2);

            BD_DEBUG(DBG_FILE, "Reading %s\n", file);

            if (key_str && file_read(fp, key_str, len*2) == len*2) {

                result = hexstring_to_hex_array(key, len, key_str);
                if (!result) {
                    BD_DEBUG(DBG_FILE, "Error converting %s\n", file);
                }

            } else {
                BD_DEBUG(DBG_FILE, "Error reading from %s\n", file);
            }

            X_FREE(key_str);

            file_close(fp);

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
        if (!file_mkdirs(file)) {
            AACS_FILE_H *fp = file_open(file, "w");

            if (fp) {
                if (file_write(fp, &version, 4)   == 4 &&
                    file_write(fp, &len,     4)   == 4 &&
                    file_write(fp, data,     len) == len) {
                    BD_DEBUG(DBG_FILE, "Wrote %d bytes to %s\n", len + 8, file);
                    result = 1;

                } else {
                    BD_DEBUG(DBG_FILE, "Error writing to %s\n", file);
                }

                file_close(fp);
            }
        }

        X_FREE(file);
    }

    return result;
}

int cache_get(const char *name, uint32_t *version, uint32_t *len, void *buf, size_t buf_size)
{
    int result = 0;
    char *file = _cache_file(name);

    *version = 0;
    if (len) {
        *len = 0;
    }
    if (!len) {
        buf = NULL;
    }

    if (file) {
        AACS_FILE_H *fp = file_open(file, "r");

        if (fp) {
            BD_DEBUG(DBG_FILE, "Reading %s\n", file);

            if (file_read(fp, version, 4) == 4 &&
                (!len || file_read(fp, len, 4) == 4) &&
                (!len || (size_t)*len <= buf_size) &&
                (!buf || file_read(fp, buf, *len) == *len)) {

              BD_DEBUG(DBG_FILE, "Read %d bytes from %s, version %d\n", 4 + (len ? 4 : 0) + (buf ? *len : 0), file, *version);
              result = 1;

            } else {
              BD_DEBUG(DBG_FILE, "Error reading from %s\n", file);
            }

            file_close(fp);

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
    int result = !file_unlink(file);
    if (!result) {
        BD_DEBUG(DBG_FILE, "Error removing %s\n", file);
    }
    X_FREE(file);
    return result;
}

int config_save(const char *name, const void *data, uint32_t len)
{
    char *path = NULL;
    AACS_FILE_H *fp = _open_cfg_file_user(name, &path, "w");
    int result = 0;

    if (fp) {
        if (file_write(fp, &len, 4) == 4 &&
            file_write(fp, data, len) == len) {
          BD_DEBUG(DBG_FILE, "Wrote %d bytes to %s\n", len + 4, path);
          result = 1;

        } else {
            BD_DEBUG(DBG_FILE | DBG_CRIT, "Error writing to %s\n", path);
        }

        file_close(fp);
    }

    X_FREE(path);

    return result;
}

int config_get(const char *name, uint32_t *len, void *buf)
{
    char *path = NULL;
    AACS_FILE_H *fp = _open_cfg_file_user(name, &path, "r");
    int result = 0;
    uint32_t size = *len;

    *len = 0;

    if (fp) {
        BD_DEBUG(DBG_FILE, "Reading %s\n", path);

        if (file_read(fp, len, 4) == 4 && (size <= *len) &&
            (!buf || file_read(fp, buf, *len) == *len)) {

            BD_DEBUG(DBG_FILE, "Read %d bytes from %s\n", 4 + (buf ? *len : 0), path);
            result = 1;

        } else {
            BD_DEBUG(DBG_FILE | DBG_CRIT, "Error reading from %s\n", path);
        }

        file_close(fp);
    }

    X_FREE(path);

    return result;
}


static int _load_config_file(config_file *cf, int system, const uint8_t *disc_id)
{
    static const char cfg_file_name[] = CFG_FILE_NAME;

    char        *cfg_file = NULL;
    AACS_FILE_H *fp       = NULL;
    int          result   = 0;

    if (system) {
        fp = _open_cfg_file_system(cfg_file_name, &cfg_file);
    } else {
        fp = _open_cfg_file_user(cfg_file_name, &cfg_file, "r");
    }

    if (fp) {
        BD_DEBUG(DBG_FILE, "found config file: %s\n", cfg_file);
        file_close(fp);

        result = keydbcfg_parse_config(cf, cfg_file, disc_id, 0);
    }

    X_FREE(cfg_file);
    return result;
}

#include "keydb.h"

static int _parse_embedded(config_file *cf)
{
    int result = 0, jj;
    unsigned ii;
    static const uint8_t empty_key[20] = {0};

    /* reverse order to maintain key positions (items are added to list head) */
    for (jj = sizeof(internal_dk_list) / sizeof(internal_dk_list[0]) - 1; jj >= 0; --jj) {
        dk_list *e = calloc(1, sizeof(dk_list));

        if (!e)
            break;

        decrypt_key(e->key, internal_dk_list[jj], 16);
        e->node = MKINT_BE16(internal_dk_list[jj] + 16);
        e->uv   = MKINT_BE32(internal_dk_list[jj] + 16 + 2);
        e->u_mask_shift = internal_dk_list[jj][20 + 2];

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

        if (!e)
            break;

        decrypt_key(e->key, internal_pk_list[ii], 16);

        if (!memcmp(e->key, empty_key, 16) ||
            _is_duplicate_pk(cf->pkl, e->key)) {

            X_FREE(e);

        } else {
            e->next = cf->pkl;
            cf->pkl = e;
            result++;
        }
    }

    for (ii = 0; ii < sizeof(internal_hc_list) / sizeof(internal_hc_list[0]); ii++) {
        cert_list  *e = calloc(1, sizeof(cert_list));

        if (!e)
            break;

        decrypt_key(e->host_priv_key, internal_hc_list[ii],      20);
        decrypt_key(e->host_cert,     internal_hc_list[ii] + 20, 92);

        if (!memcmp(e->host_priv_key, empty_key, 20) ||
            _is_duplicate_cert(cf->host_cert_list, e)) {

            X_FREE(e);

        } else {
            e->next = cf->host_cert_list;
            cf->host_cert_list = e;
            result++;
        }
    }

    return result;
}

static void _config_summary(config_file *cf)
{
    int n;
    dk_list *dkl = cf->dkl;
    pk_list *pkl = cf->pkl;
    cert_list *hcl = cf->host_cert_list;
    title_entry_list *tel = cf->list;

    BD_DEBUG(DBG_AACS, "Config summary:\n");
    for (n = 0; dkl; dkl = dkl->next, n++) ;
    BD_DEBUG(DBG_AACS, "  %d Device keys\n", n);
    for (n = 0; pkl; pkl = pkl->next, n++) ;
    BD_DEBUG(DBG_AACS, "  %d Processing keys\n", n);
    for (n = 0; hcl; hcl = hcl->next, n++) ;
    BD_DEBUG(DBG_AACS, "  %d Host certificates\n", n);
    for (n = 0; tel; tel = tel->next, n++) ;
    BD_DEBUG(DBG_AACS, "  %d Disc entries\n", n);
}

config_file *keydbcfg_config_load(const char *configfile_path, const uint8_t *disc_id)
{
    int config_ok = 0;

    config_file *cf = keydbcfg_new_config_file();
    if (!cf) {
        return NULL;
    }

    /* try to load KEYDB.cfg */

    if (configfile_path) {
        config_ok = keydbcfg_parse_config(cf, configfile_path, disc_id, 0);

    } else {
        /* If no configfile path given, check for config files in user's home and
         * under /etc.
         */
        config_ok = _load_config_file(cf, 0, disc_id);
        config_ok = _load_config_file(cf, 1, disc_id) || config_ok;
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

    _config_summary(cf);

    return cf;
}

