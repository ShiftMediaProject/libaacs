
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     HEXSTRING = 258,
     DISC_TITLE = 259,
     DIGIT = 260,
     KEYWORD_BEGIN = 261,
     KEYWORD_END = 262,
     KEYWORD_DK_LIST = 263,
     KEYWORD_DK_ENTRY = 264,
     KEYWORD_DEVICE_KEY = 265,
     KEYWORD_DEVICE_NODE = 266,
     KEYWORD_PK_LIST = 267,
     KEYWORD_HOST_CERT_LIST = 268,
     KEYWORD_HOST_CERT_ENTRY = 269,
     KEYWORD_HOST_PRIV_KEY = 270,
     KEYWORD_HOST_CERT = 271,
     KEYWORD_HOST_NONCE = 272,
     KEYWORD_HOST_KEY_POINT = 273,
     PUNCT_EQUALS_SIGN = 274,
     PUNCT_VERTICAL_BAR = 275,
     PUNCT_HYPHEN = 276,
     ENTRY_ID_DATE = 277,
     ENTRY_ID_MEK = 278,
     ENTRY_ID_VID = 279,
     ENTRY_ID_BN = 280,
     ENTRY_ID_VUK = 281,
     ENTRY_ID_PAK = 282,
     ENTRY_ID_TK = 283,
     ENTRY_ID_UK = 284,
     NEWLINE = 285,
     BAD_ENTRY = 286
   };
#endif
/* Tokens.  */
#define HEXSTRING 258
#define DISC_TITLE 259
#define DIGIT 260
#define KEYWORD_BEGIN 261
#define KEYWORD_END 262
#define KEYWORD_DK_LIST 263
#define KEYWORD_DK_ENTRY 264
#define KEYWORD_DEVICE_KEY 265
#define KEYWORD_DEVICE_NODE 266
#define KEYWORD_PK_LIST 267
#define KEYWORD_HOST_CERT_LIST 268
#define KEYWORD_HOST_CERT_ENTRY 269
#define KEYWORD_HOST_PRIV_KEY 270
#define KEYWORD_HOST_CERT 271
#define KEYWORD_HOST_NONCE 272
#define KEYWORD_HOST_KEY_POINT 273
#define PUNCT_EQUALS_SIGN 274
#define PUNCT_VERTICAL_BAR 275
#define PUNCT_HYPHEN 276
#define ENTRY_ID_DATE 277
#define ENTRY_ID_MEK 278
#define ENTRY_ID_VID 279
#define ENTRY_ID_BN 280
#define ENTRY_ID_VUK 281
#define ENTRY_ID_PAK 282
#define ENTRY_ID_TK 283
#define ENTRY_ID_UK 284
#define NEWLINE 285
#define BAD_ENTRY 286




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 106 "keydbcfg-parser.y"

  char *string;
  unsigned int digit;



/* Line 1676 of yacc.c  */
#line 121 "keydbcfg-parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




