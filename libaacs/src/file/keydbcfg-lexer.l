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

%s TITLE_STATE

WHITESPACE              ([\t ])

HEXSTRING               ([0-9a-fA-F]{32,})
DISC_TITLE              ([^\n|]*)
DIGIT                   ([0-9]+)

KEYWORD_BEGIN           ([Bb][Ee][Gg][Ii][Nn][ \t]+)
KEYWORD_END             ([Ee][Nn][Dd][ \t]+)
KEYWORD_PK_LIST         ([Pp][Kk]_[Ll][Ii][Ss][Tt])
KEYWORD_HOST_CERT_LIST  ([Hh][Oo][Ss][Tt]_[Cc][Ee][Rr][Tt]_[Ll][Ii][Ss][Tt])
KEYWORD_HOST_CERT_ENTRY ([Hh][Oo][Ss][Tt]_[Cc][Ee][Rr][Tt]_[Ee][Nn][Tt][Rr][Yy])

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
                            yylval->string = trim_string(yytext);
                            return HEXSTRING;
                          }
{DIGIT}                   {
                            yylval->digit = (unsigned int)atoi(yytext);
                            return DIGIT;
                          }

{KEYWORD_BEGIN}           { return KEYWORD_BEGIN; }
{KEYWORD_END}             { return KEYWORD_END; }
{KEYWORD_PK_LIST}         { return KEYWORD_PK_LIST; }
{KEYWORD_HOST_CERT_LIST}  { return KEYWORD_HOST_CERT_LIST; }
{KEYWORD_HOST_CERT_ENTRY} { return KEYWORD_HOST_CERT_ENTRY; }

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
  return new_string;
}
