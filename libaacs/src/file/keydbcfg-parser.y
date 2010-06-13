%{
/*
 * This file is part of libaacs
 * Copyright (C) 2010  gates
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders of libaacs
 * gives permission to link the code of its release of libaacs with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked executables.  You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL".  If you
 * modify this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to do
 * so, delete this exception statement from your version.
 */

#include "keydbcfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Disable some warnings triggered by generated parser */
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

/* enum used in certain functions to add proper entry */
enum
{
  ENTRY_TYPE_DISCID,
  ENTRY_TYPE_TITLE,
  ENTRY_TYPE_DATE,
  ENTRY_TYPE_MEK,
  ENTRY_TYPE_VID,
  ENTRY_TYPE_BN,
  ENTRY_TYPE_VUK,
  ENTRY_TYPE_PAK,
  ENTRY_TYPE_TK,
  ENTRY_TYPE_UK
};

static pk_list *new_pk_list();
static int add_pk_list(pk_list *list, const char *key);
static int add_pk_list_entry(config_file *cfgfile, const char *entry);
static cert_list *new_cert_list();
static int add_cert_list(cert_list *list, const char *host_priv_key,
                         const char *host_cert, const char *host_nonce,
                         const char *host_key_point);
static config_entry_list *new_config_entry_list();
static int add_entry(config_entry_list *list, int type, const char *entry);
static digit_key_pair_list *new_digit_key_pair_list();
static int add_digit_key_pair(digit_key_pair_list *list, int type,
                              unsigned int digit, const char *key);
static int add_digit_key_pair_entry(config_entry_list *list, int type,
                                    unsigned int digit, const char *entry);
static int add_date_entry(config_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day);
void yyerror (void *scanner, config_file *cfgfile, const char *msg);
extern int yyget_lineno  (void *scanner);

/* uncomment the line below for debugging */
// int yydebug = 1;
%}
/* Options set to generate a reentrant parser that is POSIX yacc compatible
 * The basic 'scanner' parameters are set. Also, another parameter is set
 * to pass in a config entry list struct used to hold all config entries.
 * Most of these options are bison specific, but some BSD's have bison
 * compatibility support for these options in byacc.
 */
%pure-parser
%yacc
%lex-param{void *scanner}
%parse-param{void *scanner}
%parse-param{config_file *cfgfile}

%union
{
  char *string;
  unsigned int digit;
}

%token <string> HEXSTRING
%token <string> DISC_TITLE
%token <digit> DIGIT

%token KEYWORD_BEGIN
%token KEYWORD_END
%token KEYWORD_PK_LIST
%token KEYWORD_HOST_CERT_LIST
%token KEYWORD_HOST_CERT_ENTRY

%token PUNCT_EQUALS_SIGN
%token PUNCT_VERTICAL_BAR
%token PUNCT_HYPHEN

%token ENTRY_ID_DATE
%token ENTRY_ID_MEK
%token ENTRY_ID_VID
%token ENTRY_ID_BN
%token ENTRY_ID_VUK
%token ENTRY_ID_PAK
%token ENTRY_ID_TK
%token ENTRY_ID_UK

%token NEWLINE
%token BAD_ENTRY

%type <string> discid disc_title
%type <string> host_priv_key host_cert host_nonce host_key_point
%%
config_file
  : pk_block host_cert_list_block config_entries
  ;

pk_block
  : pk_list_start pk_list pk_list_end
  ;

pk_list_start
  : newline_list KEYWORD_BEGIN KEYWORD_PK_LIST NEWLINE
  | KEYWORD_BEGIN KEYWORD_PK_LIST NEWLINE
  ;

pk_list_end
  : newline_list KEYWORD_END KEYWORD_PK_LIST NEWLINE
  | KEYWORD_END KEYWORD_PK_LIST NEWLINE
  ;

pk_list
  : pk_list pk_entry NEWLINE
  | pk_entry NEWLINE
  ;

pk_entry
  : newline_list HEXSTRING
    {
      add_pk_list_entry(cfgfile, $2);
    }
  | HEXSTRING
    {
      add_pk_list_entry(cfgfile, $1);
    }
  ;

host_cert_list_block
  : host_cert_list_start host_cert_entries host_cert_list_end
  ;

host_cert_list_start
  : newline_list KEYWORD_BEGIN KEYWORD_HOST_CERT_LIST NEWLINE
  | KEYWORD_BEGIN KEYWORD_HOST_CERT_LIST NEWLINE
  ;

host_cert_list_end
  : newline_list KEYWORD_END KEYWORD_HOST_CERT_LIST NEWLINE
  | KEYWORD_END KEYWORD_HOST_CERT_LIST NEWLINE
  ;

host_cert_entries
  : host_cert_entries host_cert_entry_block
  | host_cert_entry_block
  ;

host_cert_entry_block
  : host_cert_entry_start host_cert_entry host_cert_entry_end
  ;

host_cert_entry_start
  : newline_list KEYWORD_BEGIN KEYWORD_HOST_CERT_ENTRY NEWLINE
  | KEYWORD_BEGIN KEYWORD_HOST_CERT_ENTRY NEWLINE
  ;

host_cert_entry_end
  : newline_list KEYWORD_END KEYWORD_HOST_CERT_ENTRY NEWLINE
  | KEYWORD_END KEYWORD_HOST_CERT_ENTRY NEWLINE
  ;

host_cert_entry
  : host_priv_key host_cert host_nonce host_key_point
    {
      add_cert_list(cfgfile->host_cert_list, $1, $2, $3, $4);
    }
  ;

host_priv_key
  : newline_list HEXSTRING NEWLINE
    { $$ = $2; }
  | HEXSTRING NEWLINE
    { $$ = $1; }
  ;

host_cert
  : newline_list HEXSTRING NEWLINE
    { $$ = $2; }
  | HEXSTRING NEWLINE
    { $$ = $1; }
  ;

host_nonce
  : newline_list HEXSTRING NEWLINE
    { $$ = $2; }
  | HEXSTRING NEWLINE
    { $$ = $1; }
  ;

host_key_point
  : newline_list HEXSTRING NEWLINE
    { $$ = $2; }
  | HEXSTRING NEWLINE
    { $$ = $1; }
  ;

config_entries
  : config_entry_list newline_list
  | config_entry_list
  | config_entry_list error
    {
      if (yychar == YYEOF)
        fprintf(stderr, "warning: last entry ended without newline\n");
    }
  ;

config_entry_list
  : config_entry_list config_entry NEWLINE
    {
      config_entry_list *cursor = cfgfile->list;
      while (cursor->next)
        cursor = cursor->next;
      cursor->next = new_config_entry_list();
    }
  | config_entry NEWLINE
    {
      config_entry_list *cursor = cfgfile->list;
      while (cursor->next)
        cursor = cursor->next;
      cursor->next = new_config_entry_list();
    }
  | config_entry_list error NEWLINE
    {
      fprintf(stderr, "bad entry at line %d\n", yyget_lineno(scanner) - 1);
      yyerrok;
    }
  | error NEWLINE
    {
      fprintf(stderr, "bad entry at line %d\n", yyget_lineno(scanner) - 1);
      yyerrok;
    }
  ;

config_entry
  : newline_list disc_info entry_list
  | disc_info entry_list
  ;

newline_list
  : newline_list NEWLINE
  | NEWLINE
  ;

disc_info
  : discid PUNCT_EQUALS_SIGN disc_title
    {
      add_entry(cfgfile->list, ENTRY_TYPE_DISCID, $1);
      add_entry(cfgfile->list, ENTRY_TYPE_TITLE, $3);
    }
  ;

discid
  : HEXSTRING
  ;

disc_title
  : DISC_TITLE
  ;

entry_list
  : entry_list entry
  | entry
  ;

entry
  : date_entry
  | mek_entry
  | vid_entry
  | bn_entry
  | vuk_entry
  | pak_entry
  | tk_entry
  | uk_entry
  ;

date_entry
  : ENTRY_ID_DATE DIGIT PUNCT_HYPHEN DIGIT PUNCT_HYPHEN DIGIT
    {
      add_date_entry(cfgfile->list, $2, $4, $6);
    }
  ;

mek_entry
  : ENTRY_ID_MEK HEXSTRING
    {
      add_entry(cfgfile->list, ENTRY_TYPE_MEK, $2);
    }
  ;

vid_entry
  : ENTRY_ID_VID HEXSTRING
    {
      add_entry(cfgfile->list, ENTRY_TYPE_VID, $2);
    }
  ;

bn_entry
  : ENTRY_ID_BN bn_data_list
  ;

bn_data_list
  : bn_data_list PUNCT_VERTICAL_BAR bn_data
  | bn_data
  ;

bn_data
  : DIGIT PUNCT_HYPHEN HEXSTRING
    {
      add_digit_key_pair_entry(cfgfile->list, ENTRY_TYPE_BN, $1, $3);
    }
  ;

vuk_entry
  : ENTRY_ID_VUK HEXSTRING
    {
      add_entry(cfgfile->list, ENTRY_TYPE_VUK, $2);
    }
  ;

pak_entry
  : ENTRY_ID_PAK pak_data_list
  ;

pak_data_list
  : pak_data_list PUNCT_VERTICAL_BAR pak_data
  | pak_data
  ;

pak_data
  : DIGIT PUNCT_HYPHEN HEXSTRING
    {
      add_digit_key_pair_entry(cfgfile->list, ENTRY_TYPE_PAK, $1, $3);
    }
  ;

tk_entry
  : ENTRY_ID_TK tk_data_list
  ;

tk_data_list
  : tk_data_list PUNCT_VERTICAL_BAR tk_data
  | tk_data
  ;

tk_data
  : DIGIT PUNCT_HYPHEN HEXSTRING
    {
      add_digit_key_pair_entry(cfgfile->list, ENTRY_TYPE_TK, $1, $3);
    }
  ;

uk_entry
  : ENTRY_ID_UK uk_data_list
  ;

uk_data_list
  : uk_data_list PUNCT_VERTICAL_BAR uk_data
  | uk_data
  ;

uk_data
  : DIGIT PUNCT_HYPHEN HEXSTRING
    {
      add_digit_key_pair_entry(cfgfile->list, ENTRY_TYPE_UK, $1, $3);
    }
  ;
%%
/* Function to parse a config file */
int keydbcfg_parse_config(config_file *cfgfile, const char *path)
{
  FILE * fp = fopen(path, "r");
  if (!fp)
    return 0;

  cfgfile->pkl = new_pk_list();
  cfgfile->host_cert_list = new_cert_list();
  cfgfile->list = new_config_entry_list();

  void *scanner;
  yylex_init(&scanner);
  yyset_in(fp, scanner);
  int retval = yyparse(scanner, cfgfile);
  yylex_destroy(scanner);

  if (retval)
    return 0;

  return 1;
}

/* Function that returns pointer to new config file object */
config_file *keydbcfg_new_config_file()
{
  config_file *cfgfile = (config_file *)malloc(sizeof(*cfgfile));
  cfgfile->pkl = NULL;
  cfgfile->list = NULL;
  return cfgfile;
}

/* Function to return new pk_list object */
static pk_list *new_pk_list()
{
  pk_list *pkl = (pk_list *)malloc(sizeof(*pkl));
  pkl->key = NULL;
  pkl->next = NULL;
  return pkl;
}

/* Function to add pk to config entry list */
static int add_pk_list(pk_list *list, const char *key)
{
  if (!list)
  {
    printf("Error: No pk list passed as parameter.\n");
    return 0;
  }

  pk_list *cursor = list;
  while (cursor->next)
    cursor = cursor->next;

  cursor->key = (char*)malloc(strlen(key) + 1);
  strcpy(cursor->key, key);

  cursor->next = new_pk_list();

  return 1;
}

static int add_pk_list_entry(config_file *cfgfile, const char *entry)
{
  if (!cfgfile)
  {
    printf("Error: No config file object passed as parameter.\n");
    return 0;
  }

  add_pk_list(cfgfile->pkl, entry);

  return 1;
}

/* Function to create new certificate list */
static cert_list *new_cert_list()
{
  cert_list *list = (cert_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new certificate list!\n");
    return NULL;
  }

  list->host_priv_key = NULL;
  list->host_cert = NULL;
  list->host_nonce = NULL;
  list->host_key_point = NULL;
  list->next = NULL;

  return list;
}

/* Function to add certificate list entry into config file object */
static int add_cert_list(cert_list *list, const char *host_priv_key,
                         const char *host_cert, const char *host_nonce,
                         const char *host_key_point)
{
  if (!list)
  {
    printf("Error: no certificate list object passed as parameter.\n");
    return 0;
  }

  cert_list *cursor = list;
  while (cursor->next)
    cursor = cursor->next;

  cursor->host_priv_key = (char*)malloc(strlen(host_priv_key) + 1);
  strcpy(cursor->host_priv_key, host_priv_key);
  cursor->host_cert = (char*)malloc(strlen(host_cert) + 1);
  strcpy(cursor->host_cert, host_cert);
  cursor->host_nonce = (char*)malloc(strlen(host_nonce) + 1);
  strcpy(cursor->host_nonce, host_nonce);
  cursor->host_key_point = (char*)malloc(strlen(host_key_point) + 1);
  strcpy(cursor->host_key_point, host_key_point);

  cursor->next = new_cert_list();

  return 1;
}

/* Function that returns pointer to new config entry list */
config_entry_list *new_config_entry_list()
{
  config_entry_list *list = (config_entry_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new config entry list!\n");
    return NULL;
  }

  list->entry.discid = NULL;
  list->entry.title = NULL;
  list->entry.date.year = 0;
  list->entry.date.month = 0;
  list->entry.date.day = 0;
  list->entry.mek = NULL;
  list->entry.vid = NULL;
  list->entry.bn = NULL;
  list->entry.vuk = NULL;
  list->entry.pak = NULL;
  list->entry.tk = NULL;
  list->entry.uk = NULL;
  list->next = NULL;

  return list;
}

/* Function to add standard string entries to a config entry */
static int add_entry(config_entry_list *list, int type, const char *entry)
{
  if (!list)
  {
    printf("Error: No config list passed as parameter.\n");
    return 0;
  }

  config_entry_list *cursor = list;
  while (cursor->next)
    cursor = cursor->next;

  switch (type)
  {
    case ENTRY_TYPE_DISCID:
      cursor->entry.discid = (char*)malloc(strlen(entry) + 1);
      strcpy(cursor->entry.discid, entry);
      break;

    case ENTRY_TYPE_TITLE:
      cursor->entry.title = (char*)malloc(strlen(entry) + 1);
      strcpy(cursor->entry.title, entry);
      break;

    case ENTRY_TYPE_MEK:
      cursor->entry.mek = (char*)malloc(strlen(entry) + 1);
      strcpy(cursor->entry.mek, entry);
      break;

    case ENTRY_TYPE_VID:
      cursor->entry.vid = (char*)malloc(strlen(entry) + 1);
      strcpy(cursor->entry.vid, entry);
      break;

    case ENTRY_TYPE_VUK:
      cursor->entry.vuk = (char*)malloc(strlen(entry) + 1);
      strcpy(cursor->entry.vuk, entry);
      break;

    default:
      printf("WARNING: entry type passed in unknown\n");
      return 0;
  }

  return 1;
}

/* Function that returns pointer to new digit key pair list */
static digit_key_pair_list *new_digit_key_pair_list()
{
  digit_key_pair_list *list = (digit_key_pair_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new digit key pair entry list!\n");
    return NULL;
  }

  list->key_pair.digit = 0;
  list->key_pair.key = NULL;
  list->next = NULL;

  return list;
}

/* Function used to add a digit/key pair to a list of digit key pair entries */
static int add_digit_key_pair(digit_key_pair_list *list, int type,
                              unsigned int digit, const char *key)
{
  if (!list)
  {
    printf("Error: No digit key pair list passed as parameter.\n");
    return 0;
  }

  digit_key_pair_list *cursor = list;
  unsigned int count = 0;
  if (type == ENTRY_ID_TK || type == ENTRY_ID_UK)
    count = 1;
  while (cursor->next)
  {
    cursor = cursor->next;
    count++;
  }

  if (count != digit)
    printf("Warning: Digit list for entry may be out of order.\n");

  cursor->key_pair.digit = digit;
  cursor->key_pair.key = (char*)malloc(strlen(key) + 1);
  strcpy(cursor->key_pair.key, key);

  cursor->next = new_digit_key_pair_list();

  return 1;
}

/* Function to add a digit/key pair to a config entry */
static int add_digit_key_pair_entry(config_entry_list *list, int type,
                                    unsigned int digit, const char *entry)
{
  if (!list)
  {
    printf("Error: No config list passed as parameter.\n");
    return 0;
  }

  config_entry_list *cursor = list;
  while (cursor->next)
    cursor = cursor->next;

  switch (type)
  {
    case ENTRY_TYPE_BN:
      if (!cursor->entry.bn)
        cursor->entry.bn = new_digit_key_pair_list();
      add_digit_key_pair(cursor->entry.bn, ENTRY_ID_BN, digit, entry);
      break;

    case ENTRY_TYPE_PAK:
      if (!cursor->entry.pak)
        cursor->entry.pak = new_digit_key_pair_list();
      add_digit_key_pair(cursor->entry.pak, ENTRY_ID_PAK, digit, entry);
      break;

    case ENTRY_TYPE_TK:
      if (!cursor->entry.tk)
        cursor->entry.tk = new_digit_key_pair_list();
      add_digit_key_pair(cursor->entry.tk, ENTRY_ID_TK, digit, entry);
      break;

    case ENTRY_TYPE_UK:
      if (!cursor->entry.uk)
        cursor->entry.uk = new_digit_key_pair_list();
      add_digit_key_pair(cursor->entry.uk, ENTRY_ID_UK, digit, entry);
      break;

    default:
      printf("WARNING: entry type passed in unknown\n");
      return 0;
  }

  return 1;
}

/* Function to add a date entry */
static int add_date_entry(config_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day)
{
  if (!list)
  {
    printf("Error: No config list passed as parameter.\n");
    return 0;
  }

  config_entry_list *cursor = list;
  while (cursor->next)
    cursor = cursor->next;

  cursor->entry.date.year = year;
  cursor->entry.date.month = month;
  cursor->entry.date.day = day;

  return 1;
}

/* Our definition of yyerror */
void yyerror (void *scanner, config_file *cfgfile, const char *msg)
{
  fprintf(stderr, "%s: line %d\n", msg, yyget_lineno(scanner));
}
