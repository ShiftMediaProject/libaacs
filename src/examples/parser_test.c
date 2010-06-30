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

#include "file/keydbcfg.h"

#include <stdio.h>
#include <stdlib.h>

static int print_digit_key_pair_enties(digit_key_pair_list *list);
static int print_config_entries(config_entry_list *list);

/* Function to print the entres in a digit key pair list */
static int print_digit_key_pair_enties(digit_key_pair_list *list)
{
  if (!list)
  {
    printf("Error: No digit key pair list passed as parameter.\n");
    return 0;
  }

  digit_key_pair_list *cursor = list;
  while (cursor)
  {
    if (!cursor->key_pair.key)
      break;

    printf("    %u - %s\n", cursor->key_pair.digit, cursor->key_pair.key);

    cursor = cursor->next;
  }

  return 1;
}

/* Function that prints all entries parsed from a config file */
static int print_config_entries(config_entry_list *list)
{
  if (!list)
  {
    printf("Error: No config list passed as parameter.\n");
    return 0;
  }

  config_entry_list *cursor = list;
  while (cursor)
  {
    if (!cursor->entry.discid)
      break;

    printf("DISCID: %s\n", cursor->entry.discid);
    printf("  Title: %s\n", cursor->entry.title);
    printf("  Date: %u-%u-%u\n", cursor->entry.date.year,
      cursor->entry.date.month, cursor->entry.date.day);
    if (cursor->entry.mek)
      printf("  MEK: %s\n", cursor->entry.mek);
    if (cursor->entry.vid)
      printf("  VID: %s\n", cursor->entry.vid);
    if (cursor->entry.bn)
    {
      printf("  BN:\n");
      print_digit_key_pair_enties(cursor->entry.bn);
    }
    if (cursor->entry.vuk)
      printf("  VUK: %s\n", cursor->entry.vuk);
    if (cursor->entry.pak)
    {
      printf("  PAK:\n");
      print_digit_key_pair_enties(cursor->entry.pak);
    }
    if (cursor->entry.tk)
    {
      printf("  TK:\n");
      print_digit_key_pair_enties(cursor->entry.tk);
    }
    if (cursor->entry.uk)
    {
      printf("  UK:\n");
      print_digit_key_pair_enties(cursor->entry.uk);
    }

    cursor = cursor->next;
  }

  return 1;
}

/* Function to print certificate list from config file */
static int print_cert_list(cert_list *list)
{
  if (!list)
  {
    printf("Error: no certificate list object passed in as parameter\n");
    return 0;
  }

  printf("Available certificates:\n");
  cert_list *cursor = list;
  while (cursor)
  {
    if (!cursor->host_priv_key)
      break;

    printf("  Host private key: %s\n", cursor->host_priv_key);
    printf("  Host certificate: %s\n", cursor->host_cert);
    printf("  Host nonce: %s\n", cursor->host_nonce);
    printf("  Host key point: %s\n", cursor->host_key_point);
    printf("\n");

    cursor = cursor->next;
  }

  printf("\n");

  return 1;
}

/* Function to print config file */
static int print_config_file(config_file *cfgfile)
{
  printf("Available processing keys:\n");

  pk_list *cursor = cfgfile->pkl;
  while (cursor)
  {
    if (!cursor->key)
      break;

    printf("  %s\n", cursor->key);

    cursor = cursor->next;
  }

  printf("\n");

  int status1 = print_cert_list(cfgfile->host_cert_list);
  int status2 = print_config_entries(cfgfile->list);

  return status1 & status2;
}

/* main */
int main (int argc, char **argv)
{
  /* suppress unused parameter warning */
  if (argc) {}

  config_file *cfgfile = keydbcfg_new_config_file();
  int retval = keydbcfg_parse_config(cfgfile, argv[1]);
  retval &= print_config_file(cfgfile);
  keydbcfg_config_file_close(cfgfile);

  if (!retval)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
