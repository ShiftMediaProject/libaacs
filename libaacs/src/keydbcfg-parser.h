
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
     PUNCT_EQUALS_SIGN = 261,
     PUNCT_VERTICAL_BAR = 262,
     PUNCT_HYPHEN = 263,
     ENTRY_ID_DATE = 264,
     ENTRY_ID_MEK = 265,
     ENTRY_ID_VID = 266,
     ENTRY_ID_BN = 267,
     ENTRY_ID_VUK = 268,
     ENTRY_ID_PAK = 269,
     ENTRY_ID_TK = 270,
     ENTRY_ID_UK = 271,
     NEWLINE = 272,
     BAD_ENTRY = 273
   };
#endif
/* Tokens.  */
#define HEXSTRING 258
#define DISC_TITLE 259
#define DIGIT 260
#define PUNCT_EQUALS_SIGN 261
#define PUNCT_VERTICAL_BAR 262
#define PUNCT_HYPHEN 263
#define ENTRY_ID_DATE 264
#define ENTRY_ID_MEK 265
#define ENTRY_ID_VID 266
#define ENTRY_ID_BN 267
#define ENTRY_ID_VUK 268
#define ENTRY_ID_PAK 269
#define ENTRY_ID_TK 270
#define ENTRY_ID_UK 271
#define NEWLINE 272
#define BAD_ENTRY 273




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 85 "keydbcfg-parser.y"

  char *string;
  unsigned int digit;



/* Line 1676 of yacc.c  */
#line 95 "keydbcfg-parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




