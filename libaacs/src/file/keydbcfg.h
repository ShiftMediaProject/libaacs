/*
 * This file is part of libaacs
 * Copyright (C) 2010  gates
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

#ifndef KEYDBCFG_H
#define KEYDBCFG_H

/* struct holding a digit and key pair for <ENTRY NUMBER> - <ENTRY> entries */
typedef struct
{
  unsigned int digit;
  char *key;
} digit_key_pair;

/* list of digit_key_pair struct used in config entry */
typedef struct digit_key_pair_list_t digit_key_pair_list;
struct digit_key_pair_list_t
{
  digit_key_pair key_pair;
  digit_key_pair_list *next;
};

/* date entry struct */
typedef struct
{
  unsigned int year;
  unsigned int month;
  unsigned int day;
} date_entry;

/* pk entry */
typedef struct pk_entry pk_list;
struct pk_entry
{
  char *key;
  pk_list *next;
};

/* certificate entry */
typedef struct cert_entry cert_list;
struct cert_entry
{
  char *host_priv_key;
  char *host_cert;
  char *host_nonce;
  char *host_key_point;
  cert_list *next;
};

/* main config entry struct */
typedef struct
{
  char *discid;
  char *title;
  date_entry date;
  char *mek;
  char *vid;
  digit_key_pair_list *bn;
  char *vuk;
  digit_key_pair_list *pak;
  digit_key_pair_list *tk;
  digit_key_pair_list *uk;
} config_entry;

/* main struct for config entries, held in a list structure */
typedef struct config_entry_list_t config_entry_list;
struct config_entry_list_t
{
  config_entry entry;
  config_entry_list *next;
};

/* struct representing the contents of a config file */
typedef struct config_file_t config_file;
struct config_file_t
{
  pk_list *pkl;
  cert_list *host_cert_list;
  config_entry_list *list;
};

/* Functions used throughout the parser */
int keydbcfg_parse_config(config_file *cfgfile, const char *path);
config_file *keydbcfg_new_config_file();

#endif
