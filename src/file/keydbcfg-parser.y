%code requires {
#include "file/keydbcfg.h"

#define MAX_KEY_SIZE 128

typedef struct {
  title_entry_list    *celist;  /* current disc entry or NULL */
  digit_key_pair_list *dkplist; /* current list */

  const uint64_t  *want_disc_id; /* parse only this disc (none if NULL) */
  int              all_discs;    /* parse entries for all discs */

  size_t hexkey_size;
  union { /* make sure we're properly aligned */
    char     b[MAX_KEY_SIZE];
    uint64_t u64[5];
  } hexkey;
} parser_state;

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
#include "util/strutl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

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
    X_FREE(X);                           \
    X = pnext;                           \
  }                                      \
} while (0);

/* enum used in certain functions to add proper title entry */
enum
{
  ENTRY_TYPE_DISCID,
  /*ENTRY_TYPE_TITLE,*/
  /*ENTRY_TYPE_DATE,*/
  ENTRY_TYPE_MEK,
  ENTRY_TYPE_VID,
  /*ENTRY_TYPE_BN,*/
  ENTRY_TYPE_VUK,
  /*ENTRY_TYPE_PAK,*/
  /*ENTRY_TYPE_TK,*/
  ENTRY_TYPE_UK
};

static dk_list *new_dk_list(void);
static pk_list *new_pk_list(void);
static cert_list *new_cert_list(void);

static void add_dk_entry(config_file *cf, char *key, char *node, char *uv, char *u_mask_shift);
static void add_pk_entry(config_file *cf, char *key);
static void add_cert_entry(config_file *cf, char *host_priv_key, char *host_cert);

static title_entry_list *new_title_entry_list(void);
static int add_entry(title_entry_list *list, int type, const char *entry);
static digit_key_pair_list *new_digit_key_pair_entry(int type, unsigned int digit, const char *key);
/*
static int add_date_entry(title_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day);
*/
void yyerror (void *scanner, config_file *cf, parser_state *ps, const char *msg);
extern int libaacs_yyget_lineno  (void *scanner);

static inline int _discid_cmp(const uint64_t *want_disc_id, const uint64_t *disc_id)
{
  unsigned i;

  /* We know input strings are valid hex strings, len 40:
   * want_disc_id was created from binary data, disc_id was checked by lexer and parser.
   * -> we just need to make sure all letters are lower case (= bit 0x20 set)
   */
  for (i = 0; i < 40/sizeof(uint64_t); i++)
    if (want_disc_id[i] != (disc_id[i] | UINT64_C(0x2020202020202020)))
      return 0;
  return 1;
}

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
%parse-param{parser_state *ps}

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
%token KEYWORD_KEY_UV
%token KEYWORD_KEY_U_MASK_SHIFT
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
%type <string> host_priv_key host_cert host_nonce host_key_point hexstring_list hexkey
%type <string> device_key device_node key_uv key_u_mask_shift
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
  : newline_list ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node PUNCT_VERTICAL_BAR key_uv PUNCT_VERTICAL_BAR key_u_mask_shift NEWLINE
    {
      add_dk_entry(cf, $3, $5, $7, $9);
    }
  | newline_list ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node NEWLINE
    {
      add_dk_entry(cf, $3, $5, NULL, NULL);
    }
  | ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node PUNCT_VERTICAL_BAR key_uv PUNCT_VERTICAL_BAR key_u_mask_shift NEWLINE
    {
      add_dk_entry(cf, $2, $4, $6, $8);
    }
  | ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node NEWLINE
    {
      add_dk_entry(cf, $2, $4, NULL, NULL);
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

key_uv
  : KEYWORD_KEY_UV hexstring_list
    { $$ = $2; }
  ;

key_u_mask_shift
  : KEYWORD_KEY_U_MASK_SHIFT hexstring_list
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
  | disc_info entry_list NEWLINE
  ;

newline_list
  : newline_list NEWLINE
  | NEWLINE
  ;

disc_info
  : discid PUNCT_EQUALS_SIGN disc_title
    {
      if (ps->hexkey_size != 40) {
        fprintf(stderr, "Ignoring invalid disc id: %s (len = %zu)\n", $1, ps->hexkey_size);
        ps->celist = NULL;
      } else if (!ps->all_discs && (!ps->want_disc_id || !_discid_cmp(ps->want_disc_id, ps->hexkey.u64))) {
        ps->celist = NULL; /* ignore this disc */
      } else {
        ps->celist = new_title_entry_list();
        if (ps->celist) {
          ps->celist->next = cf->list;
          cf->list = ps->celist;
          hexstring_to_hex_array(ps->celist->entry.discid, 20, ps->hexkey.b);
        }
      }
    }
  ;

discid
  : hexkey
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
    /*
    {
      add_date_entry(celist, $2, $4, $6);
    }
    */
  ;

mek_entry
  : ENTRY_ID_MEK hexkey
    {
      add_entry(ps->celist, ENTRY_TYPE_MEK, $2);
    }
  ;

vid_entry
  : ENTRY_ID_VID hexkey
    {
      add_entry(ps->celist, ENTRY_TYPE_VID, $2);
    }
  ;

bn_entry
  : ENTRY_ID_BN bn_data_list
    {
      ps->dkplist = NULL;
    }
  ;

bn_data_list
  : bn_data_list PUNCT_VERTICAL_BAR bn_data
  | bn_data
  ;

bn_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    /*
    {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.bn = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_BN, $1, $3);
    }
    */
  ;

vuk_entry
  : ENTRY_ID_VUK hexkey
    {
      add_entry(ps->celist, ENTRY_TYPE_VUK, $2);
    }
  ;

pak_entry
  : ENTRY_ID_PAK pak_data_list
    {
      ps->dkplist = NULL;
    }
  ;

pak_data_list
  : pak_data_list PUNCT_VERTICAL_BAR pak_data
  | pak_data
  ;

pak_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    /*
    {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.pak = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_PAK, $1, $3);
    }
    */
  ;

tk_entry
  : ENTRY_ID_TK tk_data_list
    {
      ps->dkplist = NULL;
    }
  ;

tk_data_list
  : tk_data_list PUNCT_VERTICAL_BAR tk_data
  | tk_data
  ;

tk_data
  : DIGIT PUNCT_HYPHEN hexstring_list
    /*
  {
      if (!dkplist)
      {
        dkplist = new_digit_key_pair_list();
        celist->entry.tk = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_TK, $1, $3);
    }
    */
  ;

uk_entry
  : ENTRY_ID_UK uk_data_list
    {
      ps->dkplist = NULL;
    }
  ;

uk_data_list
  : uk_data_list PUNCT_VERTICAL_BAR uk_data
  | uk_data
  ;

uk_data
  : DIGIT PUNCT_HYPHEN hexkey
    {
      if (ps->celist) {
      if (!ps->dkplist)
      {
        ps->dkplist = new_digit_key_pair_entry(ENTRY_TYPE_UK, $1, $3);
        ps->celist->entry.uk = ps->dkplist;
      } else {
        ps->dkplist->next = new_digit_key_pair_entry(ENTRY_TYPE_UK, $1, $3);
        if (ps->dkplist->next)
          ps->dkplist = ps->dkplist->next;
      }
      }
    }
  ;

hexkey
  : hexkey HEXSTRING
    {
      size_t len = strlen($2);
      if (ps->hexkey_size + len >= sizeof(ps->hexkey.b)) {
        fprintf(stderr, "too long key: %s %s\n", ps->hexkey.b, $2);
      } else {
        memcpy(ps->hexkey.b + ps->hexkey_size, $2, len + 1);
        ps->hexkey_size += len;
      }
      $$ = ps->hexkey.b;
    }
  | HEXSTRING
    {
      size_t len = strlen($1);
      if (len >= sizeof(ps->hexkey.b)) {
        fprintf(stderr, "too long key: %s\n", $1);
        ps->hexkey.b[0] = 0;
        ps->hexkey_size = 0;
      } else {
        memcpy(ps->hexkey.b, $1, len + 1);
        ps->hexkey_size = len;
      }
      $$ = ps->hexkey.b;
    }
hexstring_list
  : hexstring_list HEXSTRING
    {
      char *str = (char*)malloc(strlen($1) + strlen($2) + 1);
      if (str) {
        strcpy(str, $1);
        strcat(str, $2);
      }
      $$ = str;
      X_FREE($1);
    }
  | HEXSTRING
    {
      char *str = (char*)malloc(strlen($1) + 1);
      if (str) {
        strcpy(str, $1);
      }
      $$ = str;
    }
  ;
%%
/* Function to parse a config file */
int keydbcfg_parse_config(config_file *cfgfile, const char *path, const uint8_t *disc_id, int all_discs)
{
  union { /* make sure we're properly aligned */
    uint64_t u64[5];
    char     b[41];
  } want_disc_id;

  parser_state ps = {
    .celist       = NULL,
    .dkplist      = NULL,
    .want_disc_id = NULL,
    .all_discs    = all_discs,
    .hexkey_size  = 0,
    .hexkey.b     = "",
  };

  if (!cfgfile || !path)
    return 0;

#ifdef _WIN32
  wchar_t wfilename[MAX_PATH];
  if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wfilename, MAX_PATH)) {
    return 0;
  }
  FILE *fp = _wfopen(wfilename, L"r");
#else
  FILE *fp = fopen(path, "r");
#endif
  if (!fp)
    return 0;

  if (disc_id) {
    hex_array_to_hexstring(want_disc_id.b, disc_id, 20);
    ps.want_disc_id = want_disc_id.u64;
  }
  void *scanner;
  libaacs_yylex_init(&scanner);
  libaacs_yyset_in(fp, scanner);
  int retval = yyparse(scanner, cfgfile, &ps);
  libaacs_yylex_destroy(scanner);

  fclose(fp);

  if (retval)
    return 0;

  return 1;
}

/* Function that returns pointer to new config file object */
config_file *keydbcfg_new_config_file(void)
{
  config_file *cfgfile = (config_file *)calloc(1, sizeof(*cfgfile));
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
    X_FREE(cfgfile->pkl);
    cfgfile->pkl = next;
  }

  /* free dk list */
  while (cfgfile->dkl)
  {
    dk_list *next = cfgfile->dkl->next;
    X_FREE(cfgfile->dkl);
    cfgfile->dkl = next;
  }

  /* free host cert list */
  while (cfgfile->host_cert_list)
  {
    cert_list *next = cfgfile->host_cert_list->next;
    X_FREE(cfgfile->host_cert_list);
    cfgfile->host_cert_list = next;
  }

  /* free title entries */
  while (cfgfile->list)
  {
    title_entry_list *next = cfgfile->list->next;
    /*X_FREE(cfgfile->list->entry.title);*/
    /*DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.bn);*/
    /*DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.pak);*/
    /*DIGIT_KEY_PAIR_LIST_FREE(cfgfile->list->entry.tk);*/
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
  dk_list *dkl = (dk_list *)calloc(1, sizeof(*dkl));
  if (!dkl) {
    fprintf(stderr, "Error allocating memory for new certificate list!\n");
  }
  return dkl;
}

/* Function to add dk to config file */
static void add_dk_entry(config_file *cf, char *key, char *node, char *uv, char *u_mask_shift)
{
  if (!key || !node || strlen(key) != 32) {
    fprintf(stderr, "ignoring bad DK entry %s\n", key);
    goto out;
  }

  dk_list *entry = cf->dkl;
  if (!entry) {
    entry = cf->dkl = new_dk_list();
  } else {
    for (; entry->next; entry = entry->next);
    entry->next = new_dk_list();
    entry = entry->next;
  }
  if (!entry) {
    goto out;
  }

  hexstring_to_hex_array(entry->key, 16, key);
  entry->node = strtoul(node, NULL, 16);

  if (uv) {
    entry->uv = strtoul(uv, NULL, 16);
  }
  if (u_mask_shift) {
    entry->u_mask_shift = strtoul(u_mask_shift, NULL, 16);
  }

out:
  X_FREE(key);
  X_FREE(node);
  X_FREE(uv);
  X_FREE(u_mask_shift);
}

/* Function to return new pk_list object */
static pk_list *new_pk_list(void)
{
  pk_list *pkl = (pk_list *)calloc(1, sizeof(*pkl));
  if (!pkl) {
    fprintf(stderr, "Error allocating memory for new pk list!\n");
  }
  return pkl;
}

/* Function to add pk to config file */
static void add_pk_entry(config_file *cf, char *key)
{
  if (!key || strlen(key) != 32) {
    fprintf(stderr, "ignoring bad PK entry %s\n", key);
    goto out;
  }

  pk_list *entry = cf->pkl;
  if (!entry) {
    entry = cf->pkl = new_pk_list();
  } else {
    for (; entry->next; entry = entry->next);
    entry->next = new_pk_list();
    entry = entry->next;
  }

  if (entry) {
    hexstring_to_hex_array(entry->key, 16, key);
  }

out:
  X_FREE(key);
}

/* Function to create new certificate list */
static cert_list *new_cert_list(void)
{
  cert_list *list = (cert_list *)calloc(1, sizeof(*list));
  if (!list) {
    fprintf(stderr, "Error allocating memory for new certificate list!\n");
  }
  return list;
}

/* Function to add certificate list entry into config file object */
static void add_cert_entry(config_file *cf, char *host_priv_key, char *host_cert)
{
  if (!host_priv_key || strlen(host_priv_key) != 40) {
    fprintf(stderr, "ignoring bad private key entry %s\n", host_priv_key);
    goto out;
  }
  if (!host_cert || strlen(host_cert) != 184) {
    fprintf(stderr, "ignoring bad certificate entry %s\n", host_cert);
    goto out;
  }

  cert_list *entry = cf->host_cert_list;
  if (!entry) {
    entry = cf->host_cert_list = new_cert_list();
  } else {
    for (; entry->next; entry = entry->next);
    entry->next = new_cert_list();
    entry = entry->next;
  }

  if (entry) {
    hexstring_to_hex_array(entry->host_priv_key, 20, host_priv_key);
    hexstring_to_hex_array(entry->host_cert, 92, host_cert);
  }

out:
  X_FREE(host_priv_key);
  X_FREE(host_cert);
}

/* Function that returns pointer to new title entry list */
title_entry_list *new_title_entry_list(void)
{
  title_entry_list *list = (title_entry_list *)calloc(1, sizeof(*list));
  if (!list) {
    fprintf(stderr, "Error allocating memory for new title entry list!\n");
  }
  return list;
}

#define CHECK_KEY_LENGTH(name, len)                               \
  if (!entry || strlen(entry) != len) {                           \
    fprintf(stderr, "Ignoring bad "name" entry %s\n", entry);     \
    break;                                                        \
  }

/* Function to add standard string entries to a config entry */
static int add_entry(title_entry_list *list, int type, const char *entry)
{
  if (!list)
  {
    return 0;
  }

  switch (type)
  {
    case ENTRY_TYPE_DISCID:
      CHECK_KEY_LENGTH("discid", 40)
      hexstring_to_hex_array(list->entry.discid, 20, entry);
      break;

#if 0
    case ENTRY_TYPE_TITLE:
      X_FREE(list->entry.title);
      list->entry.title = (char*)malloc(strlen(entry) + 1);
      strcpy(list->entry.title, entry);
      break;
#endif
    case ENTRY_TYPE_MEK:
      CHECK_KEY_LENGTH("mek", 32)
      hexstring_to_hex_array(list->entry.mk, 16, entry);
      break;

    case ENTRY_TYPE_VID:
      CHECK_KEY_LENGTH("vid", 32)
      hexstring_to_hex_array(list->entry.vid, 16, entry);
      break;

    case ENTRY_TYPE_VUK:
      CHECK_KEY_LENGTH("vuk", 32)
      hexstring_to_hex_array(list->entry.vuk, 16, entry);
      break;

    default:
      fprintf(stderr, "WARNING: entry type passed in unknown\n");
      return 0;
  }

  return 1;
}

/* Function used to add a digit/key pair to a list of digit key pair entries */
static digit_key_pair_list *new_digit_key_pair_entry(int type, unsigned int digit, const char *key)
{
  digit_key_pair_list *list;

  if (!key || strlen(key) != 32) {
    fprintf(stderr, "Ignoring bad UK entry %s\n", key ? key : "<null>");
    return NULL;
  }

  list = (digit_key_pair_list *)calloc(1, sizeof(*list));
  if (!list) {
    fprintf(stderr, "Error allocating memory for new digit key pair entry list!\n");
    return NULL;
  }

  list->key_pair.digit = digit;
  hexstring_to_hex_array(list->key_pair.key, 16, key);
  return list;
}

/* Function to add a date entry */
#if 0
static int add_date_entry(title_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day)
{
  if (!list)
  {
    fprintf(stderr, "Error: No title list passed as parameter.\n");
    return 0;
  }

  list->entry.date.year = year;
  list->entry.date.month = month;
  list->entry.date.day = day;

  return 1;
}
#endif

/* Our definition of yyerror */
void yyerror (void *scanner, config_file *cf, parser_state *ps, const char *msg)
{
  fprintf(stderr, "%s: line %d\n", msg, libaacs_yyget_lineno(scanner));
}
