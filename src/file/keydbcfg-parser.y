%{
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

#include "keydbcfg.h"
#include "util/macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Disable some warnings triggered by generated parser */
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

/* Fix some warnings trigger by -Wundef which can't be ignored */
#define YYENABLE_NLS 0
#define YYLTYPE_IS_TRIVIAL 0

#define DIGIT_KEY_PAIR_LIST_FREE(X) do   \
{                                        \
  while (X)                              \
  {                                      \
    digit_key_pair_list *next = X->next; \
    X_FREE(X->key_pair.key);             \
    X_FREE(X);                           \
    X = next;                            \
  }                                      \
} while (0);

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

static dk_list *new_dk_list();
static dk_list *add_dk_list_entry(dk_list *list, const char *key,
                                  const char *node);
static pk_list *new_pk_list();
static pk_list *add_pk_list_entry(pk_list *list, const char *key);
static cert_list *new_cert_list();
static cert_list *add_cert_list(cert_list *list, const char *host_priv_key,
                         const char *host_cert, const char *host_nonce,
                         const char *host_key_point);
static config_entry_list *new_config_entry_list();
static int add_entry(config_entry_list *list, int type, const char *entry);
static digit_key_pair_list *new_digit_key_pair_list();
static digit_key_pair_list *add_digit_key_pair_entry(digit_key_pair_list *list,
                              int type, unsigned int digit, const char *key);
static int add_date_entry(config_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day);
void yyerror (void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist,
              config_entry_list *celist, digit_key_pair_list *dkplist,
              const char *msg);
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
%parse-param{dk_list *dklist}
%parse-param{pk_list *pklist}
%parse-param{cert_list *clist}
%parse-param{config_entry_list *celist}
%parse-param{digit_key_pair_list *dkplist}

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
%token KEYWORD_DK_LIST
%token KEYWORD_DK_ENTRY
%token KEYWORD_DEVICE_KEY
%token KEYWORD_DEVICE_NODE
%token KEYWORD_PK_LIST
%token KEYWORD_HOST_CERT_LIST
%token KEYWORD_HOST_CERT_ENTRY
%token KEYWORD_HOST_PRIV_KEY
%token KEYWORD_HOST_CERT
%token KEYWORD_HOST_NONCE
%token KEYWORD_HOST_KEY_POINT

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
%type <string> host_priv_key host_cert host_nonce host_key_point hexstring_list
%type <string> device_key device_node
%%
config_file
  : dk_list_block pk_block host_cert_list_block config_entries
  ;

dk_list_block
  : dk_list_start dk_entries dk_list_end
  ;

dk_list_start
  : newline_list KEYWORD_BEGIN KEYWORD_DK_LIST NEWLINE
  | KEYWORD_BEGIN KEYWORD_DK_LIST NEWLINE
  ;

dk_list_end
  : newline_list KEYWORD_END KEYWORD_DK_LIST NEWLINE
  | KEYWORD_END KEYWORD_DK_LIST NEWLINE
  ;

dk_entries
  : dk_entries dk_entry_block
  | dk_entry_block
  ;

dk_entry_block
  : dk_entry_start dk_entry dk_entry_end
  ;

dk_entry_start
  : newline_list KEYWORD_BEGIN KEYWORD_DK_ENTRY NEWLINE
  | KEYWORD_BEGIN KEYWORD_DK_ENTRY NEWLINE
  ;

dk_entry_end
  : newline_list KEYWORD_END KEYWORD_DK_ENTRY NEWLINE
  | KEYWORD_END KEYWORD_DK_ENTRY NEWLINE
  ;

dk_entry
  : device_key device_node
    {
      dklist = add_dk_list_entry(dklist, $1, $2);
    }
  ;

device_key
  : newline_list KEYWORD_DEVICE_KEY hexstring_list NEWLINE
    { $$ = $3; }
  | KEYWORD_DEVICE_KEY hexstring_list NEWLINE
    { $$ = $2; }
  ;

device_node
  : newline_list KEYWORD_DEVICE_NODE hexstring_list NEWLINE
    { $$ = $3; }
  | KEYWORD_DEVICE_NODE hexstring_list NEWLINE
    { $$ = $2; }
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
  : newline_list hexstring_list
    {
      pklist = add_pk_list_entry(pklist, $2);
    }
  | hexstring_list
    {
      pklist = add_pk_list_entry(pklist, $1);
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
      clist = add_cert_list(clist, $1, $2, $3, $4);
    }
  ;

host_priv_key
  : newline_list KEYWORD_HOST_PRIV_KEY hexstring_list NEWLINE
    { $$ = $3; }
  | KEYWORD_HOST_PRIV_KEY hexstring_list NEWLINE
    { $$ = $2; }
  ;

host_cert
  : newline_list KEYWORD_HOST_CERT hexstring_list NEWLINE
    { $$ = $3; }
  | KEYWORD_HOST_CERT hexstring_list NEWLINE
    { $$ = $2; }
  ;

host_nonce
  : newline_list KEYWORD_HOST_NONCE hexstring_list NEWLINE
    { $$ = $3; }
  | KEYWORD_HOST_NONCE hexstring_list NEWLINE
    { $$ = $2; }
  ;

host_key_point
  : newline_list KEYWORD_HOST_KEY_POINT hexstring_list NEWLINE
    { $$ = $3; }
  | KEYWORD_HOST_KEY_POINT hexstring_list NEWLINE
    { $$ = $2; }
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
      celist->next = new_config_entry_list();
      celist = celist->next;
    }
  | config_entry NEWLINE
    {
      celist->next = new_config_entry_list();
      celist = celist->next;
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
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
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
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
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
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
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
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
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
  FILE * fp = fopen(path, "r");
  if (!fp)
    return 0;

  dk_list *head_dklist = new_dk_list();
  pk_list *head_pklist = new_pk_list();
  cert_list *head_clist = new_cert_list();
  config_entry_list *head_celist = new_config_entry_list();
  digit_key_pair_list *dkplist = NULL;

  void *scanner;
  yylex_init(&scanner);
  yyset_in(fp, scanner);
  int retval = yyparse(scanner, head_dklist, head_pklist, head_clist,
                       head_celist, dkplist);
  yylex_destroy(scanner);

  cfgfile->dkl = head_dklist;
  cfgfile->pkl = head_pklist;
  cfgfile->host_cert_list = head_clist;
  cfgfile->list = head_celist;

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

/* Function that closes and frees a config file object */
int keydbcfg_config_file_close(config_file *cfgfile)
{
  /* free pk list */
  while (cfgfile->pkl)
  {
    pk_list *next = cfgfile->pkl->next;
    X_FREE(cfgfile->pkl->key);
    X_FREE(cfgfile->pkl);
    cfgfile->pkl = next;
  }

  /* free host cert list */
  while (cfgfile->host_cert_list)
  {
    cert_list *next = cfgfile->host_cert_list->next;
    X_FREE(cfgfile->host_cert_list->host_priv_key);
    X_FREE(cfgfile->host_cert_list->host_cert);
    X_FREE(cfgfile->host_cert_list->host_nonce);
    X_FREE(cfgfile->host_cert_list->host_key_point);
    X_FREE(cfgfile->host_cert_list);
    cfgfile->host_cert_list = next;
  }

  /* free title entries */
  while (cfgfile->list)
  {
    config_entry_list *next = cfgfile->list->next;
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
static dk_list *new_dk_list()
{
  dk_list *dkl = (dk_list *)malloc(sizeof(*dkl));
  dkl->key = NULL;
  dkl->next = NULL;
  return dkl;
}

/* Function to add dk to config file */
static dk_list *add_dk_list_entry(dk_list *list, const char *key,
                                  const char *node)
{
  if (!list)
  {
    printf("Error: No dk list passed as parameter.\n");
    return NULL;
  }

  list->key = (char*)malloc(strlen(key) + 1);
  strcpy(list->key, key);
  list->node = strtoul(node, NULL, 16);

  list->next = new_dk_list();

  return list->next;
}

/* Function to return new pk_list object */
static pk_list *new_pk_list()
{
  pk_list *pkl = (pk_list *)malloc(sizeof(*pkl));
  pkl->key = NULL;
  pkl->next = NULL;
  return pkl;
}

/* Function to add pk to config file */
static pk_list *add_pk_list_entry(pk_list *list, const char *key)
{
  if (!list)
  {
    printf("Error: No pk list passed as parameter.\n");
    return NULL;
  }

  list->key = (char*)malloc(strlen(key) + 1);
  strcpy(list->key, key);

  list->next = new_pk_list();

  return list->next;
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
static cert_list *add_cert_list(cert_list *list, const char *host_priv_key,
                         const char *host_cert, const char *host_nonce,
                         const char *host_key_point)
{
  if (!list)
  {
    printf("Error: no certificate list object passed as parameter.\n");
    return NULL;
  }

  list->host_priv_key = (char*)malloc(strlen(host_priv_key) + 1);
  strcpy(list->host_priv_key, host_priv_key);
  list->host_cert = (char*)malloc(strlen(host_cert) + 1);
  strcpy(list->host_cert, host_cert);
  list->host_nonce = (char*)malloc(strlen(host_nonce) + 1);
  strcpy(list->host_nonce, host_nonce);
  list->host_key_point = (char*)malloc(strlen(host_key_point) + 1);
  strcpy(list->host_key_point, host_key_point);

  list->next = new_cert_list();

  return list->next;
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

  switch (type)
  {
    case ENTRY_TYPE_DISCID:
      list->entry.discid = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.discid, entry);
      break;

    case ENTRY_TYPE_TITLE:
      list->entry.title = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.title, entry);
      break;

    case ENTRY_TYPE_MEK:
      list->entry.mek = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.mek, entry);
      break;

    case ENTRY_TYPE_VID:
      list->entry.vid = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.vid, entry);
      break;

    case ENTRY_TYPE_VUK:
      list->entry.vuk = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.vuk, entry);
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
static digit_key_pair_list *add_digit_key_pair_entry(digit_key_pair_list *list,
                              int type, unsigned int digit, const char *key)
{
  if (!list)
  {
    printf("Error: No digit key pair list passed as parameter.\n");
    return NULL;
  }

  list->key_pair.digit = digit;
  list->key_pair.key = (char*)malloc(strlen(key) + 1);
  strcpy(list->key_pair.key, key);

  list->next = new_digit_key_pair_list();

  return list->next;
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

  list->entry.date.year = year;
  list->entry.date.month = month;
  list->entry.date.day = day;

  return 1;
}

/* Our definition of yyerror */
void yyerror (void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist,
              config_entry_list *celist, digit_key_pair_list *dkplist,
              const char *msg)
{
  fprintf(stderr, "%s: line %d\n", msg, yyget_lineno(scanner));
}
