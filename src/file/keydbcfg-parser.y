%code requires {
#include "file/keydbcfg.h"
}

%code {
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

#include "util/macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Disable some warnings triggered by generated parser */
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC visibility push(hidden)
#endif

/* Fix some warnings trigger by -Wundef which can't be ignored */
#define YYENABLE_NLS 0
#define YYLTYPE_IS_TRIVIAL 0

#define DIGIT_KEY_PAIR_LIST_FREE(X) do   \
{                                        \
  while (X)                              \
  {                                      \
    digit_key_pair_list *pnext = X->next;\
    X_FREE(X->key_pair.key);             \
    X_FREE(X);                           \
    X = pnext;                           \
  }                                      \
} while (0);

/* enum used in certain functions to add proper title entry */
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

static dk_list *new_dk_list(void);
static pk_list *new_pk_list(void);
static cert_list *new_cert_list(void);

static void add_dk_entry(config_file *cf, char *key, char *node);
static void add_pk_entry(config_file *cf, char *key);
static void add_cert_entry(config_file *cf, char *host_priv_key, char *host_cert);

static title_entry_list *new_title_entry_list(void);
static int add_entry(title_entry_list *list, int type, char *entry);
static digit_key_pair_list *new_digit_key_pair_list(void);
static digit_key_pair_list *add_digit_key_pair_entry(digit_key_pair_list *list,
                              int type, unsigned int digit, char *key);
static int add_date_entry(title_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day);
void yyerror (void *scanner, config_file *cf,
              title_entry_list *celist, digit_key_pair_list *dkplist,
              const char *msg);
extern int libaacs_yyget_lineno  (void *scanner);

/* uncomment the line below for debugging */
// int yydebug = 1;
}
/* Options set to generate a reentrant parser that is POSIX yacc compatible
 * The basic 'scanner' parameters are set. Also, another parameter is set
 * to pass in a title entry list struct used to hold all title entries.
 * Most of these options are bison specific, but some BSD's have bison
 * compatibility support for these options in byacc.
 */
%pure-parser
%yacc
%lex-param{void *scanner}
%parse-param{void *scanner}
%parse-param{config_file *cf}
%parse-param{title_entry_list *celist}
%parse-param{digit_key_pair_list *dkplist}

%union
{
  char *string;
  unsigned int digit;
}

%token <string> HEXSTRING
%token <string> DISC_TITLE
%token <digit> DIGIT

%token KEYWORD_DEVICE_KEY
%token KEYWORD_DEVICE_NODE
%token KEYWORD_HOST_PRIV_KEY
%token KEYWORD_HOST_CERT
%token KEYWORD_HOST_NONCE
%token KEYWORD_HOST_KEY_POINT

%token PUNCT_EQUALS_SIGN
%token PUNCT_VERTICAL_BAR
%token PUNCT_HYPHEN

%token ENTRY_ID_DK
%token ENTRY_ID_PK
%token ENTRY_ID_HC
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
%type <string> host_priv_key host_cert host_nonce host_key_point hexstring_list
%type <string> device_key device_node
%%
config_file
  : config_entry_list newline_list
  | config_entry_list
  | newline_list
  | /* empty string */
  ;

config_entry_list
  : config_entry_list config_entry
  | config_entry
  ;

config_entry
  : dk_entry
  | pk_entry
  | host_cert_entry
  | title_entry
  | error NEWLINE
    {
      fprintf(stderr, "bad entry at or around line %d\n",
              libaacs_yyget_lineno(scanner) - 1);
      yyerrok;
    }
  ;

dk_entry
  : newline_list ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node NEWLINE
    {
      add_dk_entry(cf, $3, $5);
    }
  | ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node NEWLINE
    {
      add_dk_entry(cf, $2, $4);
    }
  ;

device_key
  : KEYWORD_DEVICE_KEY hexstring_list
    { $$ = $2; }
  ;

device_node
  : KEYWORD_DEVICE_NODE hexstring_list
    { $$ = $2; }
  ;

pk_entry
  : newline_list ENTRY_ID_PK hexstring_list NEWLINE
    {
      add_pk_entry(cf, $3);
    }
  | ENTRY_ID_PK hexstring_list NEWLINE
    {
      add_pk_entry(cf, $2);
    }
  ;

host_cert_entry
  : newline_list ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert PUNCT_VERTICAL_BAR host_nonce PUNCT_VERTICAL_BAR host_key_point NEWLINE
    {
      /* host_nonce and host_key_point are ignored, keep this for backward compatibility */
      X_FREE($7);
      X_FREE($9);
      add_cert_entry(cf, $3, $5);
    }
  | ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert PUNCT_VERTICAL_BAR host_nonce PUNCT_VERTICAL_BAR host_key_point NEWLINE
    {
      /* host_nonce and host_key_point are ignored, keep this for backward compatibility */
      X_FREE($6);
      X_FREE($8);
      add_cert_entry(cf, $2, $4);
    }
  | newline_list ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert NEWLINE
    {
      add_cert_entry(cf, $3, $5);
    }
  | ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert NEWLINE
    {
      add_cert_entry(cf, $2, $4);
    }
  ;

host_priv_key
  : KEYWORD_HOST_PRIV_KEY hexstring_list
    { $$ = $2; }
  ;

host_cert
  : KEYWORD_HOST_CERT hexstring_list
    { $$ = $2; }
  ;

host_nonce
  : KEYWORD_HOST_NONCE hexstring_list
    { $$ = $2; }
  ;

host_key_point
  : KEYWORD_HOST_KEY_POINT hexstring_list
    { $$ = $2; }
  ;

title_entry
  : newline_list disc_info entry_list NEWLINE
    {
      if (!cf->list) {
        celist = cf->list = new_title_entry_list();
      } else {
      celist->next = new_title_entry_list();
      celist = celist->next;
      }
    }
  | disc_info entry_list NEWLINE
    {
      if (!cf->list) {
        celist = cf->list = new_title_entry_list();
      } else {
      celist->next = new_title_entry_list();
      celist = celist->next;
      }
    }
  ;

newline_list
  : newline_list NEWLINE
  | NEWLINE
  ;

disc_info
  : discid PUNCT_EQUALS_SIGN disc_title
    {
      add_entry(celist, ENTRY_TYPE_DISCID, $1);
      add_entry(celist, ENTRY_TYPE_TITLE, $3);
    }
  ;

discid
  : hexstring_list
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
      add_date_entry(celist, $2, $4, $6);
    }
  ;

mek_entry
  : ENTRY_ID_MEK hexstring_list
    {
      add_entry(celist, ENTRY_TYPE_MEK, $2);
    }
  ;

vid_entry
  : ENTRY_ID_VID hexstring_list
    {
      add_entry(celist, ENTRY_TYPE_VID, $2);
    }
  ;

bn_entry
  : ENTRY_ID_BN bn_data_list
    {
      dkplist = NULL;
    }
  ;

bn_data_list
  : bn_data_list PUNCT_VERTICAL_BAR bn_data
  | bn_data
  ;

bn_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.bn = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_BN, $1, $3);
    }
  ;

vuk_entry
  : ENTRY_ID_VUK hexstring_list
    {
      add_entry(celist, ENTRY_TYPE_VUK, $2);
    }
  ;

pak_entry
  : ENTRY_ID_PAK pak_data_list
    {
      dkplist = NULL;
    }
  ;

pak_data_list
  : pak_data_list PUNCT_VERTICAL_BAR pak_data
  | pak_data
  ;

pak_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.pak = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_PAK, $1, $3);
    }
  ;

tk_entry
  : ENTRY_ID_TK tk_data_list
    {
      dkplist = NULL;
    }
  ;

tk_data_list
  : tk_data_list PUNCT_VERTICAL_BAR tk_data
  | tk_data
  ;

tk_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.tk = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_TK, $1, $3);
    }
  ;

uk_entry
  : ENTRY_ID_UK uk_data_list
    {
      dkplist = NULL;
    }
  ;

uk_data_list
  : uk_data_list PUNCT_VERTICAL_BAR uk_data
  | uk_data
  ;

uk_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.uk = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_UK, $1, $3);
    }
  ;

hexstring_list
  : hexstring_list HEXSTRING
    {
      char *str = (char*)malloc(strlen($1) + strlen($2) + 1);
      strcpy(str, $1);
      strcat(str, $2);
      $$ = str;
      X_FREE($1);
    }
  | HEXSTRING
    {
      char *str = (char*)malloc(strlen($1) + 1);
      strcpy(str, $1);
      $$ = str;
    }
  ;
%%
/* Function to parse a config file */
int keydbcfg_parse_config(config_file *cfgfile, const char *path)
{
  if (!cfgfile || !path)
    return 0;

  FILE * fp = fopen(path, "r");
  if (!fp)
    return 0;

  void *scanner;
  libaacs_yylex_init(&scanner);
  libaacs_yyset_in(fp, scanner);
  int retval = yyparse(scanner, cfgfile, NULL, NULL);
  libaacs_yylex_destroy(scanner);

  fclose(fp);

  if (retval)
    return 0;

  return 1;
}

/* Function that returns pointer to new config file object */
config_file *keydbcfg_new_config_file(void)
{
  config_file *cfgfile = (config_file *)malloc(sizeof(*cfgfile));
  memset(cfgfile, 0, sizeof(*cfgfile));
  return cfgfile;
}

/* Function that closes and frees a config file object */
int keydbcfg_config_file_close(config_file *cfgfile)
{
  if (!cfgfile) {
    return 0;
  }

  /* free pk list */
  while (cfgfile->pkl)
  {
    pk_list *next = cfgfile->pkl->next;
    X_FREE(cfgfile->pkl->key);
    X_FREE(cfgfile->pkl);
    cfgfile->pkl = next;
  }

  /* free dk list */
  while (cfgfile->dkl)
  {
    dk_list *next = cfgfile->dkl->next;
    X_FREE(cfgfile->dkl->key);
    X_FREE(cfgfile->dkl);
    cfgfile->dkl = next;
  }

  /* free host cert list */
  while (cfgfile->host_cert_list)
  {
    cert_list *next = cfgfile->host_cert_list->next;
    X_FREE(cfgfile->host_cert_list->host_priv_key);
    X_FREE(cfgfile->host_cert_list->host_cert);
    X_FREE(cfgfile->host_cert_list);
    cfgfile->host_cert_list = next;
  }

  /* free title entries */
  while (cfgfile->list)
  {
    title_entry_list *next = cfgfile->list->next;
    X_FREE(cfgfile->list->entry.discid);
    X_FREE(cfgfile->list->entry.title);
    X_FREE(cfgfile->list->entry.mek);
    X_FREE(cfgfile->list->entry.vid);
    DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.bn);
    X_FREE(cfgfile->list->entry.vuk);
    DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.pak);
    DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.tk);
    DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.uk);
    X_FREE(cfgfile->list);
    cfgfile->list = next;
  }

  /* free the config file object */
  X_FREE(cfgfile);

  return 1;
}

/* Function to return new dk_list object */
static dk_list *new_dk_list(void)
{
  dk_list *dkl = (dk_list *)malloc(sizeof(*dkl));
  memset(dkl, 0, sizeof(*dkl));
  return dkl;
}

/* Function to add dk to config file */
static void add_dk_entry(config_file *cf, char *key, char *node)
{
  if (strlen(key) != 32) {
    fprintf(stderr, "ignoring bad DK entry %s\n", key);
    X_FREE(key);
    return;
  }

  dk_list *entry = cf->dkl;
  if (!entry) {
    entry = cf->dkl = new_dk_list();
  } else {
    for (; entry->next; entry = entry->next);
    entry->next = new_dk_list();
    entry = entry->next;
  }

  entry->key  = key;
  entry->node = strtoul(node, NULL, 16);
  X_FREE(node);
}

/* Function to return new pk_list object */
static pk_list *new_pk_list(void)
{
  pk_list *pkl = (pk_list *)malloc(sizeof(*pkl));
  memset(pkl, 0, sizeof(*pkl));
  return pkl;
}

/* Function to add pk to config file */
static void add_pk_entry(config_file *cf, char *key)
{
  if (strlen(key) != 32) {
    fprintf(stderr, "ignoring bad PK entry %s\n", key);
    X_FREE(key);
    return;
  }

  pk_list *entry = cf->pkl;
  if (!entry) {
    entry = cf->pkl = new_pk_list();
  } else {
    for (; entry->next; entry = entry->next);
    entry->next = new_pk_list();
    entry = entry->next;
  }

  entry->key  = key;
}

/* Function to create new certificate list */
static cert_list *new_cert_list(void)
{
  cert_list *list = (cert_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new certificate list!\n");
    return NULL;
  }

  memset(list, 0, sizeof(*list));

  return list;
}

/* Function to add certificate list entry into config file object */
static void add_cert_entry(config_file *cf, char *host_priv_key, char *host_cert)
{
  if (strlen(host_priv_key) != 40) {
    fprintf(stderr, "ignoring bad private key entry %s\n", host_priv_key);
    X_FREE(host_priv_key);
    X_FREE(host_cert);
    return;
  }
  if (strlen(host_cert) != 184) {
    fprintf(stderr, "ignoring bad certificate entry %s\n", host_cert);
    X_FREE(host_priv_key);
    X_FREE(host_cert);
    return;
  }

  cert_list *entry = cf->host_cert_list;
  if (!entry) {
    entry = cf->host_cert_list = new_cert_list();
  } else {
    for (; entry->next; entry = entry->next);
    entry->next = new_cert_list();
    entry = entry->next;
  }

  entry->host_priv_key = host_priv_key;
  entry->host_cert = host_cert;
}

/* Function that returns pointer to new title entry list */
title_entry_list *new_title_entry_list(void)
{
  title_entry_list *list = (title_entry_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new title entry list!\n");
    return NULL;
  }

  memset(list, 0, sizeof(*list));

  return list;
}

#define CHECK_KEY_LENGTH(name, len)                               \
  if (strlen(entry) != len) {                                     \
    fprintf(stderr, "Ignoring bad "name" entry %s\n", entry);     \
    X_FREE(entry);                                                \
    break;                                                        \
  }

/* Function to add standard string entries to a config entry */
static int add_entry(title_entry_list *list, int type, char *entry)
{
  if (!list)
  {
    printf("Error: No title list passed as parameter.\n");
    return 0;
  }

  switch (type)
  {
    case ENTRY_TYPE_DISCID:
      CHECK_KEY_LENGTH("discid", 20)
      X_FREE(list->entry.discid);
      list->entry.discid = entry;
      break;

    case ENTRY_TYPE_TITLE:
      X_FREE(list->entry.title);
      list->entry.title = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.title, entry);
      break;

    case ENTRY_TYPE_MEK:
      CHECK_KEY_LENGTH("mek", 16)
      X_FREE(list->entry.mek);
      list->entry.mek = entry;
      break;

    case ENTRY_TYPE_VID:
      CHECK_KEY_LENGTH("vid", 16)
      X_FREE(list->entry.vid);
      list->entry.vid = entry;
      break;

    case ENTRY_TYPE_VUK:
      CHECK_KEY_LENGTH("vuk", 16)
      X_FREE(list->entry.vuk);
      list->entry.vuk = entry;
      break;

    default:
      X_FREE(entry);
      printf("WARNING: entry type passed in unknown\n");
      return 0;
  }

  return 1;
}

/* Function that returns pointer to new digit key pair list */
static digit_key_pair_list *new_digit_key_pair_list(void)
{
  digit_key_pair_list *list = (digit_key_pair_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new digit key pair entry list!\n");
    return NULL;
  }

  memset(list, 0, sizeof(*list));

  return list;
}

/* Function used to add a digit/key pair to a list of digit key pair entries */
static digit_key_pair_list *add_digit_key_pair_entry(digit_key_pair_list *list,
                              int type, unsigned int digit, char *key)
{
  if (!list)
  {
    printf("Error: No digit key pair list passed as parameter.\n");
    return NULL;
  }

  list->key_pair.digit = digit;
  list->key_pair.key = key;

  list->next = new_digit_key_pair_list();

  return list->next;
}

/* Function to add a date entry */
static int add_date_entry(title_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day)
{
  if (!list)
  {
    printf("Error: No title list passed as parameter.\n");
    return 0;
  }

  list->entry.date.year = year;
  list->entry.date.month = month;
  list->entry.date.day = day;

  return 1;
}

/* Our definition of yyerror */
void yyerror (void *scanner, config_file *cf,
              title_entry_list *celist, digit_key_pair_list *dkplist,
              const char *msg)
{
  fprintf(stderr, "%s: line %d\n", msg, libaacs_yyget_lineno(scanner));
}
