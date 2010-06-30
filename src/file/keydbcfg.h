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
 */

#ifndef KEYDBCFG_H
#define KEYDBCFG_H

#include <util/attributes.h>

/* struct holding a digit and key pair for <ENTRY NUMBER> - <ENTRY> entries */
typedef struct digit_key_pair_t digit_key_pair;
struct digit_key_pair_t
{
  unsigned int digit;
  char *key;
};

/* list of digit_key_pair struct used in config entry */
typedef struct digit_key_pair_list_t digit_key_pair_list;
struct digit_key_pair_list_t
{
  digit_key_pair key_pair;
  digit_key_pair_list *next;
};

/* date entry struct */
typedef struct date_entry_t date_entry;
struct date_entry_t
{
  unsigned int year;
  unsigned int month;
  unsigned int day;
};

/* dk entry */
typedef struct dk_entry dk_list;
struct dk_entry
{
  char *key;
  unsigned long node;
  dk_list *next;
};

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
typedef struct config_entry_t config_entry;
struct config_entry_t
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
};

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
  dk_list *dkl;
  pk_list *pkl;
  cert_list *host_cert_list;
  config_entry_list *list;
};

/* Functions used throughout the parser */
int keydbcfg_parse_config(config_file *cfgfile, const char *path) AACS_PRIVATE;
config_file *keydbcfg_new_config_file() AACS_PRIVATE;
int keydbcfg_config_file_close(config_file *cfgfile) AACS_PRIVATE;

#endif
