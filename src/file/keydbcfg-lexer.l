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

#include "keydbcfg-parser.h"

/* Disable some warnings triggered by generated scanner */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

static char *trim_string(const char *string);
%}
/* Options to generate reentrant lexer that's POSIX lex compatible. The
 * bison-bridge option is also set since bison forces the use of a parameter
 * used to get yylval. This is handy anyway and some implementations of byacc
 * from some BSD distros support bison-bridge as well.
 */
%option posix-compat reentrant bison-bridge
%option noyywrap
%option noinput
%option nounput
%option yylineno
%option outfile="lex.yy.c"
%option prefix="libaacs_yy"

%s TITLE_STATE

WHITESPACE              ([\t ])

HEXSTRING               (0[Xx][0-9a-fA-F]+)
DISC_TITLE              ([^\n|\\]*)
DIGIT                   ([0-9]+)

KEYWORD_DEVICE_KEY      ([Dd][Ee][Vv][Ii][Cc][Ee]_[Kk][Ee][Yy])
KEYWORD_DEVICE_NODE     ([Dd][Ee][Vv][Ii][Cc][Ee]_[Nn][Oo][Dd][Ee])
KEYWORD_HOST_PRIV_KEY   ([Hh][Oo][Ss][Tt]_[Pp][Rr][Ii][Vv]_[Kk][Ee][Yy][ \t]+)
KEYWORD_HOST_CERT       ([Hh][Oo][Ss][Tt]_[Cc][Ee][Rr][Tt][ \t]+)
KEYWORD_HOST_NONCE      ([Hh][Oo][Ss][Tt]_[Nn][Oo][Nn][Cc][Ee][ \t]+)
KEYWORD_HOST_KEY_POINT  ([Hh][Oo][Ss][Tt]_[Kk][Ee][Yy]_[Pp][Oo][Ii][Nn][Tt][ \t]+)

ENTRY_ID_DK             (\|[\t ]*[Dd][Kk][\t ]*\|)
ENTRY_ID_PK             (\|[\t ]*[Pp][Kk][\t ]*\|)
ENTRY_ID_HC             (\|[\t ]*[Hh][Cc][\t ]*\|)
ENTRY_ID_DATE           (\|[\t ]*[Dd][\t ]*\|)
ENTRY_ID_MEK            (\|[\t ]*[Mm][\t ]*\|)
ENTRY_ID_VID            (\|[\t ]*[Ii][\t ]*\|)
ENTRY_ID_BN             (\|[\t ]*[Bb][\t ]*\|)
ENTRY_ID_VUK            (\|[\t ]*[Vv][\t ]*\|)
ENTRY_ID_PAK            (\|[\t ]*[Pp][\t ]*\|)
ENTRY_ID_TK             (\|[\t ]*[Tt][\t ]*\|)
ENTRY_ID_UK             (\|[\t ]*[Uu][\t ]*\|)

PUNCT_EQUALS_SIGN       ([=])
PUNCT_VERTICAL_BAR      ([|])
PUNCT_HYPHEN            ([-])

NEWLINE_ESCAPE          (\\([\n\r]|\n\r|\r\n))
NEWLINE                 ([\n\r])

COMMENT                 (;[^\n]*)
BAD_ENTRY               ([^\n])
%%
{WHITESPACE}              {}

<TITLE_STATE>{DISC_TITLE} {
                            yylval->string = trim_string(yytext);
                            BEGIN INITIAL;
                            return DISC_TITLE;
                          }
<TITLE_STATE>{NEWLINE}    {
                            printf("Bad entry at line %u!\n", yylineno);
                            BEGIN INITIAL;
                            return BAD_ENTRY;
                          }

{HEXSTRING}               {
                            /* strip the '0x' portion of hexstring */
                            yylval->string = yytext + 2;
                            return HEXSTRING;
                          }
{DIGIT}                   {
                            yylval->digit = (unsigned int)atoi(yytext);
                            return DIGIT;
                          }

{KEYWORD_DEVICE_KEY}      { return KEYWORD_DEVICE_KEY; }
{KEYWORD_DEVICE_NODE}     { return KEYWORD_DEVICE_NODE; }
{KEYWORD_HOST_PRIV_KEY}   { return KEYWORD_HOST_PRIV_KEY; }
{KEYWORD_HOST_CERT}       { return KEYWORD_HOST_CERT; }
{KEYWORD_HOST_NONCE}      { return KEYWORD_HOST_NONCE; }
{KEYWORD_HOST_KEY_POINT}  { return KEYWORD_HOST_KEY_POINT; }

{ENTRY_ID_DK}             { return ENTRY_ID_DK; }
{ENTRY_ID_PK}             { return ENTRY_ID_PK; }
{ENTRY_ID_HC}             { return ENTRY_ID_HC; }
{ENTRY_ID_DATE}           { return ENTRY_ID_DATE; }
{ENTRY_ID_MEK}            { return ENTRY_ID_MEK; }
{ENTRY_ID_VID}            { return ENTRY_ID_VID; }
{ENTRY_ID_BN}             { return ENTRY_ID_BN; }
{ENTRY_ID_VUK}            { return ENTRY_ID_VUK; }
{ENTRY_ID_PAK}            { return ENTRY_ID_PAK; }
{ENTRY_ID_TK}             { return ENTRY_ID_TK; }
{ENTRY_ID_UK}             { return ENTRY_ID_UK; }

{PUNCT_EQUALS_SIGN}       { BEGIN TITLE_STATE; return PUNCT_EQUALS_SIGN; }
{PUNCT_VERTICAL_BAR}      { return PUNCT_VERTICAL_BAR; }
{PUNCT_HYPHEN}            { return PUNCT_HYPHEN; }

{NEWLINE_ESCAPE}          {}
{NEWLINE}                 { return NEWLINE; }

{COMMENT}                 {}
{BAD_ENTRY}               { return BAD_ENTRY; }
%%
/* Function used to trim leading and trailing space from a string */
static char *trim_string(const char *string)
{
  int start = 0;
  int end = strlen(string);
  while (string[start] == ' ' || string[start] == '\t')
    start++;
  while (string[end] == '\0' || string[end] == ' ' || string[end] == '\t')
    end--;
  int size = end - start + 1;

  char *new_string = (char*)malloc(size + 1);
  strncpy(new_string, string + start, size);
  new_string[size] = '\0';
  return new_string;
}
