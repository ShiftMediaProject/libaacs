/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30706

/* Bison version string.  */
#define YYBISON_VERSION "3.7.6"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         libaacs_yyparse
#define yylex           libaacs_yylex
#define yyerror         libaacs_yyerror
#define yydebug         libaacs_yydebug
#define yynerrs         libaacs_yynerrs


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "keydbcfg-parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_HEXSTRING = 3,                  /* HEXSTRING  */
  YYSYMBOL_DISC_TITLE = 4,                 /* DISC_TITLE  */
  YYSYMBOL_DIGIT = 5,                      /* DIGIT  */
  YYSYMBOL_KEYWORD_DEVICE_KEY = 6,         /* KEYWORD_DEVICE_KEY  */
  YYSYMBOL_KEYWORD_DEVICE_NODE = 7,        /* KEYWORD_DEVICE_NODE  */
  YYSYMBOL_KEYWORD_KEY_UV = 8,             /* KEYWORD_KEY_UV  */
  YYSYMBOL_KEYWORD_KEY_U_MASK_SHIFT = 9,   /* KEYWORD_KEY_U_MASK_SHIFT  */
  YYSYMBOL_KEYWORD_HOST_PRIV_KEY = 10,     /* KEYWORD_HOST_PRIV_KEY  */
  YYSYMBOL_KEYWORD_HOST_CERT = 11,         /* KEYWORD_HOST_CERT  */
  YYSYMBOL_KEYWORD_HOST_NONCE = 12,        /* KEYWORD_HOST_NONCE  */
  YYSYMBOL_KEYWORD_HOST_KEY_POINT = 13,    /* KEYWORD_HOST_KEY_POINT  */
  YYSYMBOL_PUNCT_EQUALS_SIGN = 14,         /* PUNCT_EQUALS_SIGN  */
  YYSYMBOL_PUNCT_VERTICAL_BAR = 15,        /* PUNCT_VERTICAL_BAR  */
  YYSYMBOL_PUNCT_HYPHEN = 16,              /* PUNCT_HYPHEN  */
  YYSYMBOL_ENTRY_ID_DK = 17,               /* ENTRY_ID_DK  */
  YYSYMBOL_ENTRY_ID_PK = 18,               /* ENTRY_ID_PK  */
  YYSYMBOL_ENTRY_ID_HC = 19,               /* ENTRY_ID_HC  */
  YYSYMBOL_ENTRY_ID_DATE = 20,             /* ENTRY_ID_DATE  */
  YYSYMBOL_ENTRY_ID_MEK = 21,              /* ENTRY_ID_MEK  */
  YYSYMBOL_ENTRY_ID_VID = 22,              /* ENTRY_ID_VID  */
  YYSYMBOL_ENTRY_ID_BN = 23,               /* ENTRY_ID_BN  */
  YYSYMBOL_ENTRY_ID_VUK = 24,              /* ENTRY_ID_VUK  */
  YYSYMBOL_ENTRY_ID_PAK = 25,              /* ENTRY_ID_PAK  */
  YYSYMBOL_ENTRY_ID_TK = 26,               /* ENTRY_ID_TK  */
  YYSYMBOL_ENTRY_ID_UK = 27,               /* ENTRY_ID_UK  */
  YYSYMBOL_NEWLINE = 28,                   /* NEWLINE  */
  YYSYMBOL_BAD_ENTRY = 29,                 /* BAD_ENTRY  */
  YYSYMBOL_YYACCEPT = 30,                  /* $accept  */
  YYSYMBOL_config_file = 31,               /* config_file  */
  YYSYMBOL_config_entry_list = 32,         /* config_entry_list  */
  YYSYMBOL_config_entry = 33,              /* config_entry  */
  YYSYMBOL_dk_entry = 34,                  /* dk_entry  */
  YYSYMBOL_device_key = 35,                /* device_key  */
  YYSYMBOL_device_node = 36,               /* device_node  */
  YYSYMBOL_key_uv = 37,                    /* key_uv  */
  YYSYMBOL_key_u_mask_shift = 38,          /* key_u_mask_shift  */
  YYSYMBOL_pk_entry = 39,                  /* pk_entry  */
  YYSYMBOL_host_cert_entry = 40,           /* host_cert_entry  */
  YYSYMBOL_host_priv_key = 41,             /* host_priv_key  */
  YYSYMBOL_host_cert = 42,                 /* host_cert  */
  YYSYMBOL_host_nonce = 43,                /* host_nonce  */
  YYSYMBOL_host_key_point = 44,            /* host_key_point  */
  YYSYMBOL_title_entry = 45,               /* title_entry  */
  YYSYMBOL_newline_list = 46,              /* newline_list  */
  YYSYMBOL_disc_info = 47,                 /* disc_info  */
  YYSYMBOL_discid = 48,                    /* discid  */
  YYSYMBOL_disc_title = 49,                /* disc_title  */
  YYSYMBOL_entry_list = 50,                /* entry_list  */
  YYSYMBOL_entry = 51,                     /* entry  */
  YYSYMBOL_date_entry = 52,                /* date_entry  */
  YYSYMBOL_mek_entry = 53,                 /* mek_entry  */
  YYSYMBOL_vid_entry = 54,                 /* vid_entry  */
  YYSYMBOL_bn_entry = 55,                  /* bn_entry  */
  YYSYMBOL_bn_data_list = 56,              /* bn_data_list  */
  YYSYMBOL_bn_data = 57,                   /* bn_data  */
  YYSYMBOL_vuk_entry = 58,                 /* vuk_entry  */
  YYSYMBOL_pak_entry = 59,                 /* pak_entry  */
  YYSYMBOL_pak_data_list = 60,             /* pak_data_list  */
  YYSYMBOL_pak_data = 61,                  /* pak_data  */
  YYSYMBOL_tk_entry = 62,                  /* tk_entry  */
  YYSYMBOL_tk_data_list = 63,              /* tk_data_list  */
  YYSYMBOL_tk_data = 64,                   /* tk_data  */
  YYSYMBOL_uk_entry = 65,                  /* uk_entry  */
  YYSYMBOL_uk_data_list = 66,              /* uk_data_list  */
  YYSYMBOL_uk_data = 67,                   /* uk_data  */
  YYSYMBOL_hexkey = 68,                    /* hexkey  */
  YYSYMBOL_hexstring_list = 69             /* hexstring_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */

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


#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  25
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   140

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  71
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  147

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   284


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      25,    26,    27,    28,    29
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   180,   180,   181,   182,   183,   187,   188,   192,   193,
     194,   195,   196,   205,   209,   213,   217,   224,   229,   234,
     239,   244,   248,   255,   262,   269,   273,   280,   285,   290,
     295,   300,   301,   305,   306,   310,   329,   333,   337,   338,
     342,   343,   344,   345,   346,   347,   348,   349,   353,   362,
     369,   376,   383,   384,   388,   402,   409,   416,   417,   421,
     435,   442,   443,   447,   461,   468,   469,   473,   497,   508,
     522,   532
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "HEXSTRING",
  "DISC_TITLE", "DIGIT", "KEYWORD_DEVICE_KEY", "KEYWORD_DEVICE_NODE",
  "KEYWORD_KEY_UV", "KEYWORD_KEY_U_MASK_SHIFT", "KEYWORD_HOST_PRIV_KEY",
  "KEYWORD_HOST_CERT", "KEYWORD_HOST_NONCE", "KEYWORD_HOST_KEY_POINT",
  "PUNCT_EQUALS_SIGN", "PUNCT_VERTICAL_BAR", "PUNCT_HYPHEN", "ENTRY_ID_DK",
  "ENTRY_ID_PK", "ENTRY_ID_HC", "ENTRY_ID_DATE", "ENTRY_ID_MEK",
  "ENTRY_ID_VID", "ENTRY_ID_BN", "ENTRY_ID_VUK", "ENTRY_ID_PAK",
  "ENTRY_ID_TK", "ENTRY_ID_UK", "NEWLINE", "BAD_ENTRY", "$accept",
  "config_file", "config_entry_list", "config_entry", "dk_entry",
  "device_key", "device_node", "key_uv", "key_u_mask_shift", "pk_entry",
  "host_cert_entry", "host_priv_key", "host_cert", "host_nonce",
  "host_key_point", "title_entry", "newline_list", "disc_info", "discid",
  "disc_title", "entry_list", "entry", "date_entry", "mek_entry",
  "vid_entry", "bn_entry", "bn_data_list", "bn_data", "vuk_entry",
  "pak_entry", "pak_data_list", "pak_data", "tk_entry", "tk_data_list",
  "tk_data", "uk_entry", "uk_data_list", "uk_data", "hexkey",
  "hexstring_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284
};
#endif

#define YYPACT_NINF (-39)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-6)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      11,   -15,   -39,    13,    14,    31,   -39,    23,    15,   -39,
     -39,   -39,   -39,   -39,    18,    67,    28,    42,   -39,    14,
      32,   -39,    -2,    14,    33,   -39,   -39,    18,    13,    14,
      31,   -39,    67,    44,    47,    47,    46,    47,    60,    62,
      63,    34,   -39,   -39,   -39,   -39,   -39,   -39,   -39,   -39,
     -39,    48,   -39,    68,    56,   -39,   -39,    68,    59,    57,
      -1,    61,    58,    79,    42,    42,    80,    82,   -39,    42,
      83,    86,   -39,    87,    89,   -39,    90,    92,   -39,   -39,
     -39,   -39,   -39,    14,    -8,    14,    -6,    56,   -39,    59,
     -39,    69,    14,    46,    14,    60,    14,    62,    47,    63,
      68,    94,   -39,    68,    93,   -39,    10,    16,    95,    68,
     -39,    68,   -39,    68,   -39,    42,   -39,    14,    97,    14,
      98,    94,   -39,    93,   -39,   103,    68,   100,    68,   102,
     104,   105,   -39,    14,    96,    14,    99,   100,   102,    68,
     -39,    68,   -39,   101,   106,   -39,   -39
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,    69,     0,     0,     0,    34,     0,     0,     7,
       8,     9,    10,    11,     4,     0,     0,    36,    12,     0,
       0,    71,     0,     0,     0,     1,     6,     2,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    39,    40,    41,    42,    43,    44,    45,    46,
      47,     0,    68,    17,     0,    70,    22,    27,     0,     0,
       0,     0,     0,     0,    49,    50,     0,    51,    53,    55,
       0,    56,    58,     0,    60,    62,     0,    64,    66,    32,
      38,    37,    35,     0,     0,     0,     0,     0,    21,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,     0,    16,    28,     0,    26,     0,     0,     0,    54,
      52,    59,    57,    63,    61,    67,    65,     0,     0,     0,
       0,     0,    14,     0,    25,     0,    19,     0,    29,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,    20,
      15,    30,    24,     0,     0,    13,    23
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -39,   -39,   -39,   109,   -39,   107,    35,   -11,   -16,   -39,
     -39,    88,    36,     0,   -12,   -39,   120,    26,   -39,   -39,
     108,   -38,   -39,   -39,   -39,   -39,   -39,    37,   -39,   -39,
     -39,    38,   -39,   -39,    39,   -39,   -39,    40,   -29,   -19
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     7,     8,     9,    10,    20,    84,   118,   134,    11,
      12,    24,    86,   120,   136,    13,    14,    15,    16,    82,
      41,    42,    43,    44,    45,    46,    67,    68,    47,    48,
      71,    72,    49,    74,    75,    50,    77,    78,    17,    22
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      53,    55,    55,    80,    57,    64,    65,   101,    69,   104,
      60,    -5,     1,    18,     2,    -3,     1,    21,     2,    19,
     102,     2,   105,    25,    80,   121,    56,    88,     3,     4,
       5,   123,     3,     4,     5,    28,    29,    30,   122,     6,
      32,    23,    51,     6,   124,    52,    31,    54,    58,    63,
       2,    66,    81,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    79,    83,   100,    70,   103,    73,    76,   115,
      85,    55,    87,   109,   108,   111,    89,   113,    33,    34,
      35,    36,    37,    38,    39,    40,    90,    33,    34,    35,
      36,    37,    38,    39,    40,    91,    92,    93,   126,    94,
     128,    95,   117,    96,    97,   119,    98,    99,   132,   133,
     130,   125,   127,   129,   139,   135,   141,    26,    61,   137,
     138,   143,   106,   131,   140,   107,   144,   142,    27,   145,
     110,     0,     0,   112,   146,    59,   114,     0,     0,   116,
      62
};

static const yytype_int16 yycheck[] =
{
      19,     3,     3,    41,    23,    34,    35,    15,    37,    15,
      29,     0,     1,    28,     3,     0,     1,     3,     3,     6,
      28,     3,    28,     0,    62,    15,    28,    28,    17,    18,
      19,    15,    17,    18,    19,    17,    18,    19,    28,    28,
      14,    10,    14,    28,    28,     3,    28,    15,    15,     5,
       3,     5,     4,    27,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     7,    83,     5,    85,     5,     5,    98,
      11,     3,    15,    92,     5,    94,    15,    96,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    20,    21,    22,
      23,    24,    25,    26,    27,    16,    16,    15,   117,    16,
     119,    15,     8,    16,    15,    12,    16,    15,     5,     9,
     121,    16,    15,    15,   133,    13,   135,     8,    30,    15,
      15,   137,    87,   123,    28,    89,   138,    28,     8,    28,
      93,    -1,    -1,    95,    28,    28,    97,    -1,    -1,    99,
      32
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     3,    17,    18,    19,    28,    31,    32,    33,
      34,    39,    40,    45,    46,    47,    48,    68,    28,     6,
      35,     3,    69,    10,    41,     0,    33,    46,    17,    18,
      19,    28,    47,    20,    21,    22,    23,    24,    25,    26,
      27,    50,    51,    52,    53,    54,    55,    58,    59,    62,
      65,    14,     3,    69,    15,     3,    28,    69,    15,    35,
      69,    41,    50,     5,    68,    68,     5,    56,    57,    68,
       5,    60,    61,     5,    63,    64,     5,    66,    67,    28,
      51,     4,    49,     7,    36,    11,    42,    15,    28,    15,
      28,    16,    16,    15,    16,    15,    16,    15,    16,    15,
      69,    15,    28,    69,    15,    28,    36,    42,     5,    69,
      57,    69,    61,    69,    64,    68,    67,     8,    37,    12,
      43,    15,    28,    15,    28,    16,    69,    15,    69,    15,
      37,    43,     5,     9,    38,    13,    44,    15,    15,    69,
      28,    69,    28,    38,    44,    28,    28
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    30,    31,    31,    31,    31,    32,    32,    33,    33,
      33,    33,    33,    34,    34,    34,    34,    35,    36,    37,
      38,    39,    39,    40,    40,    40,    40,    41,    42,    43,
      44,    45,    45,    46,    46,    47,    48,    49,    50,    50,
      51,    51,    51,    51,    51,    51,    51,    51,    52,    53,
      54,    55,    56,    56,    57,    58,    59,    60,    60,    61,
      62,    63,    63,    64,    65,    66,    66,    67,    68,    68,
      69,    69
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     0,     2,     1,     1,     1,
       1,     1,     2,    10,     6,     9,     5,     2,     2,     2,
       2,     4,     3,    10,     9,     6,     5,     2,     2,     2,
       2,     4,     3,     2,     1,     3,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     6,     2,
       2,     2,     3,     1,     3,     2,     2,     3,     1,     3,
       2,     3,     1,     3,     2,     3,     1,     3,     2,     1,
       2,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, cf, ps, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scanner, cf, ps); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner, config_file *cf, parser_state *ps)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  YY_USE (cf);
  YY_USE (ps);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner, config_file *cf, parser_state *ps)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner, cf, ps);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scanner, config_file *cf, parser_state *ps)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner, cf, ps);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner, cf, ps); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner, config_file *cf, parser_state *ps)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  YY_USE (cf);
  YY_USE (ps);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, config_file *cf, parser_state *ps)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 12: /* config_entry: error NEWLINE  */
    {
      fprintf(stderr, "bad entry at or around line %d\n",
              libaacs_yyget_lineno(scanner) - 1);
      yyerrok;
    }
    break;

  case 13: /* dk_entry: newline_list ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node PUNCT_VERTICAL_BAR key_uv PUNCT_VERTICAL_BAR key_u_mask_shift NEWLINE  */
    {
      add_dk_entry(cf, (yyvsp[-7].string), (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string));
    }
    break;

  case 14: /* dk_entry: newline_list ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node NEWLINE  */
    {
      add_dk_entry(cf, (yyvsp[-3].string), (yyvsp[-1].string), NULL, NULL);
    }
    break;

  case 15: /* dk_entry: ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node PUNCT_VERTICAL_BAR key_uv PUNCT_VERTICAL_BAR key_u_mask_shift NEWLINE  */
    {
      add_dk_entry(cf, (yyvsp[-7].string), (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string));
    }
    break;

  case 16: /* dk_entry: ENTRY_ID_DK device_key PUNCT_VERTICAL_BAR device_node NEWLINE  */
    {
      add_dk_entry(cf, (yyvsp[-3].string), (yyvsp[-1].string), NULL, NULL);
    }
    break;

  case 17: /* device_key: KEYWORD_DEVICE_KEY hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 18: /* device_node: KEYWORD_DEVICE_NODE hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 19: /* key_uv: KEYWORD_KEY_UV hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 20: /* key_u_mask_shift: KEYWORD_KEY_U_MASK_SHIFT hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 21: /* pk_entry: newline_list ENTRY_ID_PK hexstring_list NEWLINE  */
    {
      add_pk_entry(cf, (yyvsp[-1].string));
    }
    break;

  case 22: /* pk_entry: ENTRY_ID_PK hexstring_list NEWLINE  */
    {
      add_pk_entry(cf, (yyvsp[-1].string));
    }
    break;

  case 23: /* host_cert_entry: newline_list ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert PUNCT_VERTICAL_BAR host_nonce PUNCT_VERTICAL_BAR host_key_point NEWLINE  */
    {
      /* host_nonce and host_key_point are ignored, keep this for backward compatibility */
      X_FREE((yyvsp[-3].string));
      X_FREE((yyvsp[-1].string));
      add_cert_entry(cf, (yyvsp[-7].string), (yyvsp[-5].string));
    }
    break;

  case 24: /* host_cert_entry: ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert PUNCT_VERTICAL_BAR host_nonce PUNCT_VERTICAL_BAR host_key_point NEWLINE  */
    {
      /* host_nonce and host_key_point are ignored, keep this for backward compatibility */
      X_FREE((yyvsp[-3].string));
      X_FREE((yyvsp[-1].string));
      add_cert_entry(cf, (yyvsp[-7].string), (yyvsp[-5].string));
    }
    break;

  case 25: /* host_cert_entry: newline_list ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert NEWLINE  */
    {
      add_cert_entry(cf, (yyvsp[-3].string), (yyvsp[-1].string));
    }
    break;

  case 26: /* host_cert_entry: ENTRY_ID_HC host_priv_key PUNCT_VERTICAL_BAR host_cert NEWLINE  */
    {
      add_cert_entry(cf, (yyvsp[-3].string), (yyvsp[-1].string));
    }
    break;

  case 27: /* host_priv_key: KEYWORD_HOST_PRIV_KEY hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 28: /* host_cert: KEYWORD_HOST_CERT hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 29: /* host_nonce: KEYWORD_HOST_NONCE hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 30: /* host_key_point: KEYWORD_HOST_KEY_POINT hexstring_list  */
    { (yyval.string) = (yyvsp[0].string); }
    break;

  case 35: /* disc_info: discid PUNCT_EQUALS_SIGN disc_title  */
    {
      if (ps->hexkey_size != 40) {
        fprintf(stderr, "Ignoring invalid disc id: %s (len = %zu)\n", (yyvsp[-2].string), ps->hexkey_size);
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
    break;

  case 49: /* mek_entry: ENTRY_ID_MEK hexkey  */
    {
      add_entry(ps->celist, ENTRY_TYPE_MEK, (yyvsp[0].string));
    }
    break;

  case 50: /* vid_entry: ENTRY_ID_VID hexkey  */
    {
      add_entry(ps->celist, ENTRY_TYPE_VID, (yyvsp[0].string));
    }
    break;

  case 51: /* bn_entry: ENTRY_ID_BN bn_data_list  */
    {
      ps->dkplist = NULL;
    }
    break;

  case 55: /* vuk_entry: ENTRY_ID_VUK hexkey  */
    {
      add_entry(ps->celist, ENTRY_TYPE_VUK, (yyvsp[0].string));
    }
    break;

  case 56: /* pak_entry: ENTRY_ID_PAK pak_data_list  */
    {
      ps->dkplist = NULL;
    }
    break;

  case 60: /* tk_entry: ENTRY_ID_TK tk_data_list  */
    {
      ps->dkplist = NULL;
    }
    break;

  case 64: /* uk_entry: ENTRY_ID_UK uk_data_list  */
    {
      ps->dkplist = NULL;
    }
    break;

  case 67: /* uk_data: DIGIT PUNCT_HYPHEN hexkey  */
    {
      if (ps->celist) {
      if (!ps->dkplist)
      {
        if (ps->celist->entry.uk) {
          /* duplicate entry */
          char disc_id[41];
          fprintf(stderr, "Ignoring duplicate unit key entry for %s\n",
                  str_print_hex(disc_id, ps->celist->entry.discid, 20));
        } else {
          ps->dkplist = new_digit_key_pair_entry(ENTRY_TYPE_UK, (yyvsp[-2].digit), (yyvsp[0].string));
          ps->celist->entry.uk = ps->dkplist;
        }
      } else {
        ps->dkplist->next = new_digit_key_pair_entry(ENTRY_TYPE_UK, (yyvsp[-2].digit), (yyvsp[0].string));
        if (ps->dkplist->next)
          ps->dkplist = ps->dkplist->next;
      }
      }
    }
    break;

  case 68: /* hexkey: hexkey HEXSTRING  */
    {
      size_t len = strlen((yyvsp[0].string));
      if (ps->hexkey_size + len >= sizeof(ps->hexkey.b)) {
        fprintf(stderr, "too long key: %s %s\n", ps->hexkey.b, (yyvsp[0].string));
      } else {
        memcpy(ps->hexkey.b + ps->hexkey_size, (yyvsp[0].string), len + 1);
        ps->hexkey_size += len;
      }
      (yyval.string) = ps->hexkey.b;
    }
    break;

  case 69: /* hexkey: HEXSTRING  */
    {
      size_t len = strlen((yyvsp[0].string));
      if (len >= sizeof(ps->hexkey.b)) {
        fprintf(stderr, "too long key: %s\n", (yyvsp[0].string));
        ps->hexkey.b[0] = 0;
        ps->hexkey_size = 0;
      } else {
        memcpy(ps->hexkey.b, (yyvsp[0].string), len + 1);
        ps->hexkey_size = len;
      }
      (yyval.string) = ps->hexkey.b;
    }
    break;

  case 70: /* hexstring_list: hexstring_list HEXSTRING  */
    {
      char *str = (char*)malloc(strlen((yyvsp[-1].string)) + strlen((yyvsp[0].string)) + 1);
      if (str) {
        strcpy(str, (yyvsp[-1].string));
        strcat(str, (yyvsp[0].string));
      }
      (yyval.string) = str;
      X_FREE((yyvsp[-1].string));
    }
    break;

  case 71: /* hexstring_list: HEXSTRING  */
    {
      char *str = (char*)malloc(strlen((yyvsp[0].string)) + 1);
      if (str) {
        strcpy(str, (yyvsp[0].string));
      }
      (yyval.string) = str;
    }
    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, cf, ps, YY_("syntax error"));
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
                      yytoken, &yylval, scanner, cf, ps);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner, cf, ps);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

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


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, cf, ps, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner, cf, ps);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner, cf, ps);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}


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
    str_print_hex(want_disc_id.b, disc_id, 20);
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
