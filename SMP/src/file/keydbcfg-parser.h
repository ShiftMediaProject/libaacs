/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_LIBAACS_YY_SMP_SRC_FILE_KEYDBCFG_PARSER_H_INCLUDED
# define YY_LIBAACS_YY_SMP_SRC_FILE_KEYDBCFG_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int libaacs_yydebug;
#endif
/* "%code requires" blocks.  */


#include "file/keydbcfg.h"



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    HEXSTRING = 258,
    DISC_TITLE = 259,
    DIGIT = 260,
    KEYWORD_DEVICE_KEY = 261,
    KEYWORD_DEVICE_NODE = 262,
    KEYWORD_KEY_UV = 263,
    KEYWORD_KEY_U_MASK_SHIFT = 264,
    KEYWORD_HOST_PRIV_KEY = 265,
    KEYWORD_HOST_CERT = 266,
    KEYWORD_HOST_NONCE = 267,
    KEYWORD_HOST_KEY_POINT = 268,
    PUNCT_EQUALS_SIGN = 269,
    PUNCT_VERTICAL_BAR = 270,
    PUNCT_HYPHEN = 271,
    ENTRY_ID_DK = 272,
    ENTRY_ID_PK = 273,
    ENTRY_ID_HC = 274,
    ENTRY_ID_DATE = 275,
    ENTRY_ID_MEK = 276,
    ENTRY_ID_VID = 277,
    ENTRY_ID_BN = 278,
    ENTRY_ID_VUK = 279,
    ENTRY_ID_PAK = 280,
    ENTRY_ID_TK = 281,
    ENTRY_ID_UK = 282,
    NEWLINE = 283,
    BAD_ENTRY = 284
  };
#endif
/* Tokens.  */
#define HEXSTRING 258
#define DISC_TITLE 259
#define DIGIT 260
#define KEYWORD_DEVICE_KEY 261
#define KEYWORD_DEVICE_NODE 262
#define KEYWORD_KEY_UV 263
#define KEYWORD_KEY_U_MASK_SHIFT 264
#define KEYWORD_HOST_PRIV_KEY 265
#define KEYWORD_HOST_CERT 266
#define KEYWORD_HOST_NONCE 267
#define KEYWORD_HOST_KEY_POINT 268
#define PUNCT_EQUALS_SIGN 269
#define PUNCT_VERTICAL_BAR 270
#define PUNCT_HYPHEN 271
#define ENTRY_ID_DK 272
#define ENTRY_ID_PK 273
#define ENTRY_ID_HC 274
#define ENTRY_ID_DATE 275
#define ENTRY_ID_MEK 276
#define ENTRY_ID_VID 277
#define ENTRY_ID_BN 278
#define ENTRY_ID_VUK 279
#define ENTRY_ID_PAK 280
#define ENTRY_ID_TK 281
#define ENTRY_ID_UK 282
#define NEWLINE 283
#define BAD_ENTRY 284

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


  char *string;
  unsigned int digit;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int libaacs_yyparse (void *scanner, config_file *cf, title_entry_list *celist, digit_key_pair_list *dkplist);

#endif /* !YY_LIBAACS_YY_SMP_SRC_FILE_KEYDBCFG_PARSER_H_INCLUDED  */
