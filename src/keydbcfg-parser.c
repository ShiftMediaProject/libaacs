
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "keydbcfg-parser.y"

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
static title_entry_list *new_title_entry_list();
static int add_entry(title_entry_list *list, int type, const char *entry);
static digit_key_pair_list *new_digit_key_pair_list();
static digit_key_pair_list *add_digit_key_pair_entry(digit_key_pair_list *list,
                              int type, unsigned int digit, const char *key);
static int add_date_entry(title_entry_list *list, unsigned int year,
                          unsigned int month, unsigned int day);
void yyerror (void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist,
              title_entry_list *celist, digit_key_pair_list *dkplist,
              const char *msg);
extern int yyget_lineno  (void *scanner);

/* uncomment the line below for debugging */
// int yydebug = 1;


/* Line 189 of yacc.c  */
#line 163 "keydbcfg-parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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
     KEYWORD_PROCESSING_KEY = 267,
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
#define KEYWORD_PROCESSING_KEY 267
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

/* Line 214 of yacc.c  */
#line 106 "keydbcfg-parser.y"

  char *string;
  unsigned int digit;



/* Line 214 of yacc.c  */
#line 268 "keydbcfg-parser.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 280 "keydbcfg-parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   231

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  89
/* YYNRULES -- Number of states.  */
#define YYNSTATES  205

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     8,    11,    13,    15,    17,    19,
      21,    25,    30,    34,    39,    43,    46,    48,    52,    57,
      61,    66,    70,    73,    78,    82,    87,    91,    96,   100,
     104,   109,   113,   118,   122,   125,   127,   131,   136,   140,
     145,   149,   154,   159,   163,   168,   172,   177,   181,   186,
     190,   195,   199,   202,   205,   207,   211,   213,   215,   218,
     220,   222,   224,   226,   228,   230,   232,   234,   236,   243,
     246,   249,   252,   256,   258,   262,   265,   268,   272,   274,
     278,   281,   285,   287,   291,   294,   298,   300,   304,   307
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      33,     0,    -1,    34,    60,    -1,    34,    -1,    34,    35,
      -1,    35,    -1,    36,    -1,    46,    -1,    47,    -1,    59,
      -1,    37,    39,    38,    -1,    60,     6,     8,    30,    -1,
       6,     8,    30,    -1,    60,     7,     8,    30,    -1,     7,
       8,    30,    -1,    39,    40,    -1,    40,    -1,    41,    43,
      42,    -1,    60,     6,     9,    30,    -1,     6,     9,    30,
      -1,    60,     7,     9,    30,    -1,     7,     9,    30,    -1,
      44,    45,    -1,    60,    10,    82,    30,    -1,    10,    82,
      30,    -1,    60,    11,    82,    30,    -1,    11,    82,    30,
      -1,    60,    12,    82,    30,    -1,    12,    82,    30,    -1,
      48,    50,    49,    -1,    60,     6,    13,    30,    -1,     6,
      13,    30,    -1,    60,     7,    13,    30,    -1,     7,    13,
      30,    -1,    50,    51,    -1,    51,    -1,    52,    54,    53,
      -1,    60,     6,    14,    30,    -1,     6,    14,    30,    -1,
      60,     7,    14,    30,    -1,     7,    14,    30,    -1,    55,
      56,    57,    58,    -1,    60,    15,    82,    30,    -1,    15,
      82,    30,    -1,    60,    16,    82,    30,    -1,    16,    82,
      30,    -1,    60,    17,    82,    30,    -1,    17,    82,    30,
      -1,    60,    18,    82,    30,    -1,    18,    82,    30,    -1,
      60,    61,    64,    30,    -1,    61,    64,    30,    -1,     1,
      30,    -1,    60,    30,    -1,    30,    -1,    62,    19,    63,
      -1,    82,    -1,     4,    -1,    64,    65,    -1,    65,    -1,
      66,    -1,    67,    -1,    68,    -1,    69,    -1,    72,    -1,
      73,    -1,    76,    -1,    79,    -1,    22,     5,    21,     5,
      21,     5,    -1,    23,    82,    -1,    24,    82,    -1,    25,
      70,    -1,    70,    20,    71,    -1,    71,    -1,     5,    21,
      82,    -1,    26,    82,    -1,    27,    74,    -1,    74,    20,
      75,    -1,    75,    -1,     5,    21,    82,    -1,    28,    77,
      -1,    77,    20,    78,    -1,    78,    -1,     5,    21,    82,
      -1,    29,    80,    -1,    80,    20,    81,    -1,    81,    -1,
       5,    21,    82,    -1,    82,     3,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   150,   150,   151,   155,   156,   160,   161,   162,   163,
     167,   171,   172,   176,   177,   181,   182,   186,   190,   191,
     195,   196,   200,   207,   209,   214,   216,   221,   225,   232,
     236,   237,   241,   242,   246,   247,   251,   255,   256,   260,
     261,   265,   272,   274,   279,   281,   286,   288,   293,   295,
     300,   305,   310,   318,   319,   323,   331,   335,   339,   340,
     344,   345,   346,   347,   348,   349,   350,   351,   355,   362,
     369,   376,   383,   384,   388,   400,   407,   414,   415,   419,
     431,   438,   439,   443,   455,   462,   463,   467,   479,   487
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "HEXSTRING", "DISC_TITLE", "DIGIT",
  "KEYWORD_BEGIN", "KEYWORD_END", "KEYWORD_DK_LIST", "KEYWORD_DK_ENTRY",
  "KEYWORD_DEVICE_KEY", "KEYWORD_DEVICE_NODE", "KEYWORD_PROCESSING_KEY",
  "KEYWORD_HOST_CERT_LIST", "KEYWORD_HOST_CERT_ENTRY",
  "KEYWORD_HOST_PRIV_KEY", "KEYWORD_HOST_CERT", "KEYWORD_HOST_NONCE",
  "KEYWORD_HOST_KEY_POINT", "PUNCT_EQUALS_SIGN", "PUNCT_VERTICAL_BAR",
  "PUNCT_HYPHEN", "ENTRY_ID_DATE", "ENTRY_ID_MEK", "ENTRY_ID_VID",
  "ENTRY_ID_BN", "ENTRY_ID_VUK", "ENTRY_ID_PAK", "ENTRY_ID_TK",
  "ENTRY_ID_UK", "NEWLINE", "BAD_ENTRY", "$accept", "config_file",
  "config_entry_list", "config_entry", "dk_list_block", "dk_list_start",
  "dk_list_end", "dk_entries", "dk_entry_block", "dk_entry_start",
  "dk_entry_end", "dk_entry", "device_key", "device_node", "pk_entry",
  "host_cert_list_block", "host_cert_list_start", "host_cert_list_end",
  "host_cert_entries", "host_cert_entry_block", "host_cert_entry_start",
  "host_cert_entry_end", "host_cert_entry", "host_priv_key", "host_cert",
  "host_nonce", "host_key_point", "title_entry", "newline_list",
  "disc_info", "discid", "disc_title", "entry_list", "entry", "date_entry",
  "mek_entry", "vid_entry", "bn_entry", "bn_data_list", "bn_data",
  "vuk_entry", "pak_entry", "pak_data_list", "pak_data", "tk_entry",
  "tk_data_list", "tk_data", "uk_entry", "uk_data_list", "uk_data",
  "hexstring_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    32,    33,    33,    34,    34,    35,    35,    35,    35,
      36,    37,    37,    38,    38,    39,    39,    40,    41,    41,
      42,    42,    43,    44,    44,    45,    45,    46,    46,    47,
      48,    48,    49,    49,    50,    50,    51,    52,    52,    53,
      53,    54,    55,    55,    56,    56,    57,    57,    58,    58,
      59,    59,    59,    60,    60,    61,    62,    63,    64,    64,
      65,    65,    65,    65,    65,    65,    65,    65,    66,    67,
      68,    69,    70,    70,    71,    72,    73,    74,    74,    75,
      76,    77,    77,    78,    79,    80,    80,    81,    82,    82
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     2,     1,     1,     1,     1,     1,
       3,     4,     3,     4,     3,     2,     1,     3,     4,     3,
       4,     3,     2,     4,     3,     4,     3,     4,     3,     3,
       4,     3,     4,     3,     2,     1,     3,     4,     3,     4,
       3,     4,     4,     3,     4,     3,     4,     3,     4,     3,
       4,     3,     2,     2,     1,     3,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     6,     2,
       2,     2,     3,     1,     3,     2,     2,     3,     1,     3,
       2,     3,     1,     3,     2,     3,     1,     3,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    89,     0,     0,    54,     0,     0,     5,     6,
       0,     7,     8,     0,     9,     0,     0,     0,    56,    52,
       0,     0,     0,     1,     4,     2,     0,     0,    16,     0,
       0,     0,     0,    35,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    88,
      12,    31,    28,     0,     0,    10,    15,     0,     0,     0,
       0,     0,     0,     0,     0,    29,    34,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    69,    70,
       0,    71,    73,    75,     0,    76,    78,     0,    80,    82,
       0,    84,    86,    51,    58,    57,    55,    19,     0,     0,
       0,     0,    17,     0,     0,    22,     0,     0,     0,    38,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,    11,    30,    27,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    24,     0,     0,     0,
       0,     0,    18,    33,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,    37,     0,    74,    72,    79,    77,
      83,    81,    87,    85,    13,    21,     0,    26,     0,    23,
      32,    40,     0,    45,     0,     0,    41,     0,     0,     0,
      42,     0,    20,    25,    39,    47,     0,     0,     0,    44,
      68,    49,     0,    46,    48
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,     7,     8,     9,    10,    65,    27,    28,    29,
     112,    69,    70,   115,    11,    12,    13,    75,    32,    33,
      34,   124,    79,    80,   127,   160,   186,    14,    15,    16,
      17,   106,    48,    49,    50,    51,    52,    53,    91,    92,
      54,    55,    95,    96,    56,    98,    99,    57,   101,   102,
      18
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -48
static const yytype_int16 yypact[] =
{
      15,   -18,   -48,    -2,    10,   -48,    54,     2,   -48,   -48,
      43,   -48,   -48,    57,   -48,    16,   125,    41,    95,   -48,
      69,    70,     6,   -48,   -48,    16,    92,    49,   -48,    82,
      59,    88,    52,   -48,    75,    63,   169,    10,   -48,   125,
     115,    10,    10,   116,    10,   124,   136,   138,   137,   -48,
     -48,   -48,   -48,   -48,   -48,   -48,   -48,   -48,   126,   -48,
     -48,   -48,   -48,   114,   134,   -48,   -48,    65,    10,    74,
      85,    84,   147,   148,   132,   -48,   -48,    77,    10,    78,
      87,    76,   165,   150,   153,    12,   146,   164,    95,    95,
     166,   168,   -48,    95,   170,   172,   -48,   173,   175,   -48,
     176,   178,   -48,   -48,   -48,   -48,   -48,   -48,   156,   181,
      14,   187,   -48,    79,    10,   -48,    86,    10,   160,   -48,
     171,   186,    17,   188,   -48,    81,    10,   101,   103,    10,
     174,   -48,   -48,   -48,   -48,   195,    10,   116,    10,   124,
      10,   136,    10,   138,   -48,   177,   -48,   179,   194,    20,
      10,    22,   -48,   -48,   180,   -48,   182,   191,    27,    10,
     105,   109,    10,    31,   -48,   185,    95,   -48,    95,   -48,
      95,   -48,    95,   -48,   -48,   -48,   183,   -48,    32,   -48,
     -48,   -48,   184,   -48,    38,    10,   -48,   110,    10,    40,
     -48,   203,   -48,   -48,   -48,   -48,    45,    10,    48,   -48,
     -48,   -48,    50,   -48,   -48
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -48,   -48,   -48,   204,   -48,   -48,   -48,   -48,   189,   -48,
     -48,   -48,   -48,   -48,   -48,   -48,   -48,   -48,   -48,   190,
     -48,   -48,   -48,   -48,   -48,   -48,   -48,   -48,    -3,   112,
     -48,   -48,   192,   -47,   -48,   -48,   -48,   -48,   -48,    80,
     -48,   -48,   -48,    89,   -48,   -48,    83,   -48,   -48,    72,
      -4
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -4
static const yytype_int16 yytable[] =
{
      22,   104,    -3,     1,    25,     2,    20,    30,     3,    59,
      35,    21,    19,     2,     4,    59,     1,    59,     2,     2,
      59,     3,    36,    59,    67,    59,    71,     4,    37,    77,
      59,    81,     5,    85,    59,    59,    62,    88,    89,   104,
      93,    59,   133,    59,   146,     5,    38,   155,    59,    26,
     177,    59,   179,    59,    23,    26,    64,   183,    31,    74,
      58,   190,   193,    31,   110,    72,   113,   116,   195,    82,
     199,    72,   109,     5,   122,   201,   125,   128,   203,     5,
     204,   111,     5,    82,   121,   123,   148,     5,   157,    38,
      78,   129,    68,    38,   117,    38,   114,   150,    59,    60,
      61,    63,    73,   126,     5,     5,    38,    38,     5,    38,
     149,    38,     5,   151,    38,     5,    38,     5,   159,   162,
      87,    90,   158,   185,   161,   163,   188,    39,   197,    94,
     105,     5,   166,    38,   168,     5,   170,    39,   172,    38,
      38,    97,   108,   100,   107,   120,   178,    40,    41,    42,
      43,    44,    45,    46,    47,   184,   118,   187,   189,    40,
      41,    42,    43,    44,    45,    46,    47,   103,    40,    41,
      42,    43,    44,    45,    46,    47,   134,    83,   119,   130,
     131,   196,    84,   132,   198,   135,   144,   136,   137,   145,
     152,   138,   139,   202,   140,   141,   147,   142,   143,   154,
     165,   153,   156,   176,   164,   182,   191,   174,   200,   175,
     180,    24,   181,   192,   194,   173,    66,   167,     0,     0,
       0,     0,    76,     0,   171,     0,     0,     0,   169,     0,
       0,    86
};

static const yytype_int16 yycheck[] =
{
       4,    48,     0,     1,     7,     3,     8,    10,     6,     3,
      13,    13,    30,     3,    12,     3,     1,     3,     3,     3,
       3,     6,     6,     3,    27,     3,    29,    12,    12,    32,
       3,    34,    30,    37,     3,     3,    30,    41,    42,    86,
      44,     3,    30,     3,    30,    30,    30,    30,     3,     6,
      30,     3,    30,     3,     0,     6,     7,    30,     6,     7,
      19,    30,    30,     6,    68,     6,    69,    70,    30,     6,
      30,     6,     7,    30,    78,    30,    79,    80,    30,    30,
      30,     7,    30,     6,     7,     7,     7,    30,     7,    30,
      15,    15,    10,    30,    10,    30,    11,    11,     3,    30,
      30,     9,    14,    16,    30,    30,    30,    30,    30,    30,
     114,    30,    30,   117,    30,    30,    30,    30,    17,    16,
       5,     5,   126,    18,   127,   129,    17,    15,    18,     5,
       4,    30,   136,    30,   138,    30,   140,    25,   142,    30,
      30,     5,     8,     5,    30,    13,   150,    22,    23,    24,
      25,    26,    27,    28,    29,   159,     9,   160,   162,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     8,    30,    14,
      30,   185,    13,    30,   188,    21,    30,    21,    20,     8,
      30,    21,    20,   197,    21,    20,     9,    21,    20,    13,
       5,    30,    14,     9,    30,    14,    21,    30,     5,    30,
      30,     7,    30,    30,    30,   143,    27,   137,    -1,    -1,
      -1,    -1,    32,    -1,   141,    -1,    -1,    -1,   139,    -1,
      -1,    39
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     6,    12,    30,    33,    34,    35,    36,
      37,    46,    47,    48,    59,    60,    61,    62,    82,    30,
       8,    13,    82,     0,    35,    60,     6,    39,    40,    41,
      60,     6,    50,    51,    52,    60,     6,    12,    30,    61,
      22,    23,    24,    25,    26,    27,    28,    29,    64,    65,
      66,    67,    68,    69,    72,    73,    76,    79,    19,     3,
      30,    30,    30,     9,     7,    38,    40,    60,    10,    43,
      44,    60,     6,    14,     7,    49,    51,    60,    15,    54,
      55,    60,     6,     8,    13,    82,    64,     5,    82,    82,
       5,    70,    71,    82,     5,    74,    75,     5,    77,    78,
       5,    80,    81,    30,    65,     4,    63,    30,     8,     7,
      82,     7,    42,    60,    11,    45,    60,    10,     9,    30,
      13,     7,    82,     7,    53,    60,    16,    56,    60,    15,
      14,    30,    30,    30,    30,    21,    21,    20,    21,    20,
      21,    20,    21,    20,    30,     8,    30,     9,     7,    82,
      11,    82,    30,    30,    13,    30,    14,     7,    82,    17,
      57,    60,    16,    82,    30,     5,    82,    71,    82,    75,
      82,    78,    82,    81,    30,    30,     9,    30,    82,    30,
      30,    30,    14,    30,    82,    18,    58,    60,    17,    82,
      30,    21,    30,    30,    30,    30,    82,    18,    82,    30,
       5,    30,    82,    30,    30
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (scanner, dklist, pklist, clist, celist, dkplist, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, scanner)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, scanner, dklist, pklist, clist, celist, dkplist); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist, title_entry_list *celist, digit_key_pair_list *dkplist)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner, dklist, pklist, clist, celist, dkplist)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void *scanner;
    dk_list *dklist;
    pk_list *pklist;
    cert_list *clist;
    title_entry_list *celist;
    digit_key_pair_list *dkplist;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (scanner);
  YYUSE (dklist);
  YYUSE (pklist);
  YYUSE (clist);
  YYUSE (celist);
  YYUSE (dkplist);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist, title_entry_list *celist, digit_key_pair_list *dkplist)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, scanner, dklist, pklist, clist, celist, dkplist)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void *scanner;
    dk_list *dklist;
    pk_list *pklist;
    cert_list *clist;
    title_entry_list *celist;
    digit_key_pair_list *dkplist;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner, dklist, pklist, clist, celist, dkplist);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist, title_entry_list *celist, digit_key_pair_list *dkplist)
#else
static void
yy_reduce_print (yyvsp, yyrule, scanner, dklist, pklist, clist, celist, dkplist)
    YYSTYPE *yyvsp;
    int yyrule;
    void *scanner;
    dk_list *dklist;
    pk_list *pklist;
    cert_list *clist;
    title_entry_list *celist;
    digit_key_pair_list *dkplist;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , scanner, dklist, pklist, clist, celist, dkplist);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, scanner, dklist, pklist, clist, celist, dkplist); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist, title_entry_list *celist, digit_key_pair_list *dkplist)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, scanner, dklist, pklist, clist, celist, dkplist)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    void *scanner;
    dk_list *dklist;
    pk_list *pklist;
    cert_list *clist;
    title_entry_list *celist;
    digit_key_pair_list *dkplist;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  YYUSE (dklist);
  YYUSE (pklist);
  YYUSE (clist);
  YYUSE (celist);
  YYUSE (dkplist);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist, title_entry_list *celist, digit_key_pair_list *dkplist);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist, title_entry_list *celist, digit_key_pair_list *dkplist)
#else
int
yyparse (scanner, dklist, pklist, clist, celist, dkplist)
    void *scanner;
    dk_list *dklist;
    pk_list *pklist;
    cert_list *clist;
    title_entry_list *celist;
    digit_key_pair_list *dkplist;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 22:

/* Line 1455 of yacc.c  */
#line 201 "keydbcfg-parser.y"
    {
      dklist = add_dk_list_entry(dklist, (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string));
    }
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 208 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(3) - (4)].string); }
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 210 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); }
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 215 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(3) - (4)].string); }
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 217 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); }
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 222 "keydbcfg-parser.y"
    {
      pklist = add_pk_list_entry(pklist, (yyvsp[(3) - (4)].string));
    }
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 226 "keydbcfg-parser.y"
    {
      pklist = add_pk_list_entry(pklist, (yyvsp[(2) - (3)].string));
    }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 266 "keydbcfg-parser.y"
    {
      clist = add_cert_list(clist, (yyvsp[(1) - (4)].string), (yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string), (yyvsp[(4) - (4)].string));
    }
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 273 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(3) - (4)].string); }
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 275 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); }
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 280 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(3) - (4)].string); }
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 282 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); }
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 287 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(3) - (4)].string); }
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 289 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); }
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 294 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(3) - (4)].string); }
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 296 "keydbcfg-parser.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); }
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 301 "keydbcfg-parser.y"
    {
      celist->next = new_title_entry_list();
      celist = celist->next;
    }
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 306 "keydbcfg-parser.y"
    {
      celist->next = new_title_entry_list();
      celist = celist->next;
    }
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 311 "keydbcfg-parser.y"
    {
      fprintf(stderr, "bad entry at line %d\n", yyget_lineno(scanner) - 1);
      yyerrok;
    }
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 324 "keydbcfg-parser.y"
    {
      add_entry(celist, ENTRY_TYPE_DISCID, (yyvsp[(1) - (3)].string));
      add_entry(celist, ENTRY_TYPE_TITLE, (yyvsp[(3) - (3)].string));
    }
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 356 "keydbcfg-parser.y"
    {
      add_date_entry(celist, (yyvsp[(2) - (6)].digit), (yyvsp[(4) - (6)].digit), (yyvsp[(6) - (6)].digit));
    }
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 363 "keydbcfg-parser.y"
    {
      add_entry(celist, ENTRY_TYPE_MEK, (yyvsp[(2) - (2)].string));
    }
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 370 "keydbcfg-parser.y"
    {
      add_entry(celist, ENTRY_TYPE_VID, (yyvsp[(2) - (2)].string));
    }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 377 "keydbcfg-parser.y"
    {
      dkplist = NULL;
    }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 389 "keydbcfg-parser.y"
    {
      if (!dkplist)
      {
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
        celist->entry.bn = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_BN, (yyvsp[(1) - (3)].digit), (yyvsp[(3) - (3)].string));
    }
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 401 "keydbcfg-parser.y"
    {
      add_entry(celist, ENTRY_TYPE_VUK, (yyvsp[(2) - (2)].string));
    }
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 408 "keydbcfg-parser.y"
    {
      dkplist = NULL;
    }
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 420 "keydbcfg-parser.y"
    {
      if (!dkplist)
      {
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
        celist->entry.pak = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_PAK, (yyvsp[(1) - (3)].digit), (yyvsp[(3) - (3)].string));
    }
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 432 "keydbcfg-parser.y"
    {
      dkplist = NULL;
    }
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 444 "keydbcfg-parser.y"
    {
      if (!dkplist)
      {
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
        celist->entry.tk = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_TK, (yyvsp[(1) - (3)].digit), (yyvsp[(3) - (3)].string));
    }
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 456 "keydbcfg-parser.y"
    {
      dkplist = NULL;
    }
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 468 "keydbcfg-parser.y"
    {
      if (!dkplist)
      {
        dkplist = (digit_key_pair_list *)malloc(sizeof(*dkplist));
        celist->entry.uk = dkplist;
      }
      dkplist = add_digit_key_pair_entry(dkplist, ENTRY_TYPE_UK, (yyvsp[(1) - (3)].digit), (yyvsp[(3) - (3)].string));
    }
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 480 "keydbcfg-parser.y"
    {
      char *str = (char*)malloc(strlen((yyvsp[(1) - (2)].string)) + strlen((yyvsp[(2) - (2)].string)) + 1);
      strcpy(str, (yyvsp[(1) - (2)].string));
      strcat(str, (yyvsp[(2) - (2)].string));
      (yyval.string) = str;
      X_FREE((yyvsp[(1) - (2)].string));
    }
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 488 "keydbcfg-parser.y"
    {
      char *str = (char*)malloc(strlen((yyvsp[(1) - (1)].string)) + 1);
      strcpy(str, (yyvsp[(1) - (1)].string));
      (yyval.string) = str;
    }
    break;



/* Line 1455 of yacc.c  */
#line 2012 "keydbcfg-parser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, dklist, pklist, clist, celist, dkplist, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (scanner, dklist, pklist, clist, celist, dkplist, yymsg);
	  }
	else
	  {
	    yyerror (scanner, dklist, pklist, clist, celist, dkplist, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, scanner, dklist, pklist, clist, celist, dkplist);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, scanner, dklist, pklist, clist, celist, dkplist);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, dklist, pklist, clist, celist, dkplist, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, scanner, dklist, pklist, clist, celist, dkplist);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, scanner, dklist, pklist, clist, celist, dkplist);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 494 "keydbcfg-parser.y"

/* Function to parse a config file */
int keydbcfg_parse_config(config_file *cfgfile, const char *path)
{
  FILE * fp = fopen(path, "r");
  if (!fp)
    return 0;

  dk_list *head_dklist = new_dk_list();
  pk_list *head_pklist = new_pk_list();
  cert_list *head_clist = new_cert_list();
  title_entry_list *head_celist = new_title_entry_list();
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

/* Function that returns pointer to new title entry list */
title_entry_list *new_title_entry_list()
{
  title_entry_list *list = (title_entry_list *)malloc(sizeof(*list));
  if (!list)
  {
    printf("Error allocating memory for new title entry list!\n");
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
static int add_entry(title_entry_list *list, int type, const char *entry)
{
  if (!list)
  {
    printf("Error: No title list passed as parameter.\n");
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
void yyerror (void *scanner, dk_list *dklist, pk_list *pklist, cert_list *clist,
              title_entry_list *celist, digit_key_pair_list *dkplist,
              const char *msg)
{
  fprintf(stderr, "%s: line %d\n", msg, yyget_lineno(scanner));
}

