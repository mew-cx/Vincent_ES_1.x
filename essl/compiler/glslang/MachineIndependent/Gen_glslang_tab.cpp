/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ATTRIBUTE = 258,
     CONST_QUAL = 259,
     BOOL_TYPE = 260,
     FLOAT_TYPE = 261,
     INT_TYPE = 262,
     BREAK = 263,
     CONTINUE = 264,
     DO = 265,
     ELSE = 266,
     FOR = 267,
     IF = 268,
     DISCARD = 269,
     RETURN = 270,
     BVEC2 = 271,
     BVEC3 = 272,
     BVEC4 = 273,
     IVEC2 = 274,
     IVEC3 = 275,
     IVEC4 = 276,
     VEC2 = 277,
     VEC3 = 278,
     VEC4 = 279,
     MATRIX2 = 280,
     MATRIX3 = 281,
     MATRIX4 = 282,
     IN_QUAL = 283,
     OUT_QUAL = 284,
     INOUT_QUAL = 285,
     UNIFORM = 286,
     VARYING = 287,
     STRUCT = 288,
     VOID_TYPE = 289,
     WHILE = 290,
     SAMPLER1D = 291,
     SAMPLER2D = 292,
     SAMPLER3D = 293,
     SAMPLERCUBE = 294,
     SAMPLER1DSHADOW = 295,
     SAMPLER2DSHADOW = 296,
     SAMPLERRECTARB = 297,
     SAMPLERRECTSHADOWARB = 298,
     LOW_PRECISION = 299,
     MEDIUM_PRECISION = 300,
     HIGH_PRECISION = 301,
     INVARIANT = 302,
     PRECISION = 303,
     IDENTIFIER = 304,
     TYPE_NAME = 305,
     FLOATCONSTANT = 306,
     INTCONSTANT = 307,
     BOOLCONSTANT = 308,
     FIELD_SELECTION = 309,
     LEFT_OP = 310,
     RIGHT_OP = 311,
     INC_OP = 312,
     DEC_OP = 313,
     LE_OP = 314,
     GE_OP = 315,
     EQ_OP = 316,
     NE_OP = 317,
     AND_OP = 318,
     OR_OP = 319,
     XOR_OP = 320,
     MUL_ASSIGN = 321,
     DIV_ASSIGN = 322,
     ADD_ASSIGN = 323,
     MOD_ASSIGN = 324,
     LEFT_ASSIGN = 325,
     RIGHT_ASSIGN = 326,
     AND_ASSIGN = 327,
     XOR_ASSIGN = 328,
     OR_ASSIGN = 329,
     SUB_ASSIGN = 330,
     LEFT_PAREN = 331,
     RIGHT_PAREN = 332,
     LEFT_BRACKET = 333,
     RIGHT_BRACKET = 334,
     LEFT_BRACE = 335,
     RIGHT_BRACE = 336,
     DOT = 337,
     COMMA = 338,
     COLON = 339,
     EQUAL = 340,
     SEMICOLON = 341,
     BANG = 342,
     DASH = 343,
     TILDE = 344,
     PLUS = 345,
     STAR = 346,
     SLASH = 347,
     PERCENT = 348,
     LEFT_ANGLE = 349,
     RIGHT_ANGLE = 350,
     VERTICAL_BAR = 351,
     CARET = 352,
     AMPERSAND = 353,
     QUESTION = 354
   };
#endif
#define ATTRIBUTE 258
#define CONST_QUAL 259
#define BOOL_TYPE 260
#define FLOAT_TYPE 261
#define INT_TYPE 262
#define BREAK 263
#define CONTINUE 264
#define DO 265
#define ELSE 266
#define FOR 267
#define IF 268
#define DISCARD 269
#define RETURN 270
#define BVEC2 271
#define BVEC3 272
#define BVEC4 273
#define IVEC2 274
#define IVEC3 275
#define IVEC4 276
#define VEC2 277
#define VEC3 278
#define VEC4 279
#define MATRIX2 280
#define MATRIX3 281
#define MATRIX4 282
#define IN_QUAL 283
#define OUT_QUAL 284
#define INOUT_QUAL 285
#define UNIFORM 286
#define VARYING 287
#define STRUCT 288
#define VOID_TYPE 289
#define WHILE 290
#define SAMPLER1D 291
#define SAMPLER2D 292
#define SAMPLER3D 293
#define SAMPLERCUBE 294
#define SAMPLER1DSHADOW 295
#define SAMPLER2DSHADOW 296
#define SAMPLERRECTARB 297
#define SAMPLERRECTSHADOWARB 298
#define LOW_PRECISION 299
#define MEDIUM_PRECISION 300
#define HIGH_PRECISION 301
#define INVARIANT 302
#define PRECISION 303
#define IDENTIFIER 304
#define TYPE_NAME 305
#define FLOATCONSTANT 306
#define INTCONSTANT 307
#define BOOLCONSTANT 308
#define FIELD_SELECTION 309
#define LEFT_OP 310
#define RIGHT_OP 311
#define INC_OP 312
#define DEC_OP 313
#define LE_OP 314
#define GE_OP 315
#define EQ_OP 316
#define NE_OP 317
#define AND_OP 318
#define OR_OP 319
#define XOR_OP 320
#define MUL_ASSIGN 321
#define DIV_ASSIGN 322
#define ADD_ASSIGN 323
#define MOD_ASSIGN 324
#define LEFT_ASSIGN 325
#define RIGHT_ASSIGN 326
#define AND_ASSIGN 327
#define XOR_ASSIGN 328
#define OR_ASSIGN 329
#define SUB_ASSIGN 330
#define LEFT_PAREN 331
#define RIGHT_PAREN 332
#define LEFT_BRACKET 333
#define RIGHT_BRACKET 334
#define LEFT_BRACE 335
#define RIGHT_BRACE 336
#define DOT 337
#define COMMA 338
#define COLON 339
#define EQUAL 340
#define SEMICOLON 341
#define BANG 342
#define DASH 343
#define TILDE 344
#define PLUS 345
#define STAR 346
#define SLASH 347
#define PERCENT 348
#define LEFT_ANGLE 349
#define RIGHT_ANGLE 350
#define VERTICAL_BAR 351
#define CARET 352
#define AMPERSAND 353
#define QUESTION 354




/* Copy the first part of user declarations.  */
#line 39 "glslang.y"


/* Based on:
ANSI C Yacc grammar

In 1985, Jeff Lee published his Yacc grammar (which is accompanied by a 
matching Lex specification) for the April 30, 1985 draft version of the 
ANSI C standard.  Tom Stockfisch reposted it to net.sources in 1987; that
original, as mentioned in the answer to question 17.25 of the comp.lang.c
FAQ, can be ftp'ed from ftp.uu.net, file usenet/net.sources/ansi.c.grammar.Z.
 
I intend to keep this version as close to the current C Standard grammar as 
possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

#include "SymbolTable.h"
#include "ParseHelper.h"
#include "../Public/ShaderLang.h"

#ifdef _WIN32
    #define YYPARSE_PARAM parseContext
    #define YYPARSE_PARAM_DECL TParseContext&
    #define YY_DECL int yylex(YYSTYPE* pyylval, TParseContext& parseContext)
    #define YYLEX_PARAM parseContext
#else
    #define YYPARSE_PARAM parseContextLocal
    #define parseContext (*((TParseContext*)(parseContextLocal)))
    #define YY_DECL int yylex(YYSTYPE* pyylval, void* parseContextLocal)
    #define YYLEX_PARAM (void*)(parseContextLocal)
    extern void yyerror(char*);    
#endif

#define FRAG_VERT_ONLY(S, L) {                                                  \
    if (parseContext.language != EShLangFragment &&                             \
        parseContext.language != EShLangVertex) {                               \
        parseContext.error(L, " supported in vertex/fragment shaders only ", S, "", "");   \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define VERTEX_ONLY(S, L) {                                                     \
    if (parseContext.language != EShLangVertex) {                               \
        parseContext.error(L, " supported in vertex shaders only ", S, "", "");            \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define FRAG_ONLY(S, L) {                                                       \
    if (parseContext.language != EShLangFragment) {                             \
        parseContext.error(L, " supported in fragment shaders only ", S, "", "");          \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define PACK_ONLY(S, L) {                                                       \
    if (parseContext.language != EShLangPack) {                                 \
        parseContext.error(L, " supported in pack shaders only ", S, "", "");              \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define UNPACK_ONLY(S, L) {                                                     \
    if (parseContext.language != EShLangUnpack) {                               \
        parseContext.error(L, " supported in unpack shaders only ", S, "", "");            \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define PACK_UNPACK_ONLY(S, L) {                                                \
    if (parseContext.language != EShLangUnpack &&                               \
        parseContext.language != EShLangPack) {                                 \
        parseContext.error(L, " supported in pack/unpack shaders only ", S, "", "");       \
        parseContext.recover();                                                            \
    }                                                                           \
}


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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 117 "glslang.y"
typedef union YYSTYPE {
    struct {
        TSourceLoc line;
        union {
            TString *string;
            float f;
            int i;
            bool b;
        };
        TSymbol* symbol;
    } lex;
    struct {
        TSourceLoc line;
        TOperator op;
        union {
            TIntermNode* intermNode;
            TIntermNodePair nodePair;
            TIntermTyped* intermTypedNode;
            TIntermAggregate* intermAggregate;
        };
        union {
            TPublicType type;
            TQualifier qualifier;
            TQualifier precision;								// Added for ESSL support
            TFunction* function;
            TParameter param;
            TTypeLine typeLine;
            TTypeList* typeList;
        };
    } interm;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 385 "glslang.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 149 "glslang.y"

#ifndef _WIN32
    extern int yylex(YYSTYPE*, void*);
#endif


/* Line 214 of yacc.c.  */
#line 402 "glslang.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  75
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1441

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  100
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  78
/* YYNRULES -- Number of rules. */
#define YYNRULES  216
/* YYNRULES -- Number of states. */
#define YYNSTATES  329

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   354

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    17,    19,
      24,    26,    30,    33,    36,    38,    40,    42,    46,    49,
      52,    55,    57,    60,    64,    67,    69,    71,    73,    75,
      78,    81,    84,    86,    88,    90,    92,    94,    98,   102,
     106,   108,   112,   116,   118,   122,   126,   128,   132,   136,
     140,   144,   146,   150,   154,   156,   160,   162,   166,   168,
     172,   174,   178,   180,   184,   186,   190,   192,   198,   200,
     204,   206,   208,   210,   212,   214,   216,   218,   220,   222,
     224,   226,   228,   232,   234,   237,   240,   245,   248,   250,
     252,   255,   259,   263,   266,   272,   276,   279,   283,   286,
     287,   289,   291,   293,   295,   297,   301,   307,   314,   322,
     331,   337,   339,   342,   347,   353,   360,   368,   373,   376,
     378,   381,   383,   385,   387,   390,   392,   394,   399,   401,
     404,   406,   408,   410,   412,   414,   416,   418,   420,   422,
     424,   426,   428,   430,   432,   434,   436,   438,   440,   442,
     444,   446,   448,   450,   452,   454,   456,   458,   460,   462,
     468,   473,   475,   478,   482,   484,   488,   490,   495,   497,
     499,   501,   503,   505,   507,   509,   511,   513,   516,   517,
     518,   524,   526,   528,   531,   535,   537,   540,   542,   545,
     551,   555,   557,   559,   564,   565,   572,   573,   582,   583,
     591,   593,   595,   597,   598,   601,   605,   608,   611,   614,
     618,   621,   623,   626,   628,   630,   631
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     174,     0,    -1,    49,    -1,   101,    -1,    52,    -1,    51,
      -1,    53,    -1,    76,   128,    77,    -1,   102,    -1,   103,
      78,   104,    79,    -1,   105,    -1,   103,    82,    54,    -1,
     103,    57,    -1,   103,    58,    -1,   128,    -1,   106,    -1,
     107,    -1,   103,    82,   107,    -1,   109,    77,    -1,   108,
      77,    -1,   110,    34,    -1,   110,    -1,   110,   126,    -1,
     109,    83,   126,    -1,   111,    76,    -1,   143,    -1,    49,
      -1,    54,    -1,   103,    -1,    57,   112,    -1,    58,   112,
      -1,   113,   112,    -1,    90,    -1,    88,    -1,    87,    -1,
      89,    -1,   112,    -1,   114,    91,   112,    -1,   114,    92,
     112,    -1,   114,    93,   112,    -1,   114,    -1,   115,    90,
     114,    -1,   115,    88,   114,    -1,   115,    -1,   116,    55,
     115,    -1,   116,    56,   115,    -1,   116,    -1,   117,    94,
     116,    -1,   117,    95,   116,    -1,   117,    59,   116,    -1,
     117,    60,   116,    -1,   117,    -1,   118,    61,   117,    -1,
     118,    62,   117,    -1,   118,    -1,   119,    98,   118,    -1,
     119,    -1,   120,    97,   119,    -1,   120,    -1,   121,    96,
     120,    -1,   121,    -1,   122,    63,   121,    -1,   122,    -1,
     123,    65,   122,    -1,   123,    -1,   124,    64,   123,    -1,
     124,    -1,   124,    99,   128,    84,   126,    -1,   125,    -1,
     112,   127,   126,    -1,    85,    -1,    66,    -1,    67,    -1,
      69,    -1,    68,    -1,    75,    -1,    70,    -1,    71,    -1,
      72,    -1,    73,    -1,    74,    -1,   126,    -1,   128,    83,
     126,    -1,   125,    -1,   131,    86,    -1,   139,    86,    -1,
      48,   146,   145,    86,    -1,   132,    77,    -1,   134,    -1,
     133,    -1,   134,   136,    -1,   133,    83,   136,    -1,   141,
      49,    76,    -1,   143,    49,    -1,   143,    49,    78,   129,
      79,    -1,   142,   137,   135,    -1,   137,   135,    -1,   142,
     137,   138,    -1,   137,   138,    -1,    -1,    28,    -1,    29,
      -1,    30,    -1,   143,    -1,   140,    -1,   139,    83,    49,
      -1,   139,    83,    49,    78,    79,    -1,   139,    83,    49,
      78,   129,    79,    -1,   139,    83,    49,    78,    79,    85,
     152,    -1,   139,    83,    49,    78,   129,    79,    85,   152,
      -1,   139,    83,    49,    85,   152,    -1,   141,    -1,   141,
      49,    -1,   141,    49,    78,    79,    -1,   141,    49,    78,
     129,    79,    -1,   141,    49,    78,    79,    85,   152,    -1,
     141,    49,    78,   129,    79,    85,   152,    -1,   141,    49,
      85,   152,    -1,    47,    49,    -1,   143,    -1,   142,   143,
      -1,     4,    -1,     3,    -1,    32,    -1,    47,    32,    -1,
      31,    -1,   144,    -1,   144,    78,   129,    79,    -1,   145,
      -1,   146,   145,    -1,    34,    -1,     6,    -1,     7,    -1,
       5,    -1,    22,    -1,    23,    -1,    24,    -1,    16,    -1,
      17,    -1,    18,    -1,    19,    -1,    20,    -1,    21,    -1,
      25,    -1,    26,    -1,    27,    -1,    36,    -1,    37,    -1,
      38,    -1,    39,    -1,    40,    -1,    41,    -1,    42,    -1,
      43,    -1,   147,    -1,    50,    -1,    46,    -1,    45,    -1,
      44,    -1,    33,    49,    80,   148,    81,    -1,    33,    80,
     148,    81,    -1,   149,    -1,   148,   149,    -1,   143,   150,
      86,    -1,   151,    -1,   150,    83,   151,    -1,    49,    -1,
      49,    78,   129,    79,    -1,   126,    -1,   130,    -1,   156,
      -1,   155,    -1,   153,    -1,   162,    -1,   163,    -1,   166,
      -1,   173,    -1,    80,    81,    -1,    -1,    -1,    80,   157,
     161,   158,    81,    -1,   160,    -1,   155,    -1,    80,    81,
      -1,    80,   161,    81,    -1,   154,    -1,   161,   154,    -1,
      86,    -1,   128,    86,    -1,    13,    76,   128,    77,   164,
      -1,   154,    11,   154,    -1,   154,    -1,   128,    -1,   141,
      49,    85,   152,    -1,    -1,    35,    76,   167,   165,    77,
     159,    -1,    -1,    10,   168,   154,    35,    76,   128,    77,
      86,    -1,    -1,    12,    76,   169,   170,   172,    77,   159,
      -1,   162,    -1,   153,    -1,   165,    -1,    -1,   171,    86,
      -1,   171,    86,   128,    -1,     9,    86,    -1,     8,    86,
      -1,    15,    86,    -1,    15,   128,    86,    -1,    14,    86,
      -1,   175,    -1,   174,   175,    -1,   176,    -1,   130,    -1,
      -1,   131,   177,   160,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   226,   226,   261,   264,   277,   282,   287,   293,   296,
     372,   376,   488,   498,   511,   519,   634,   637,   659,   663,
     670,   674,   681,   687,   696,   704,   768,   777,   787,   790,
     800,   810,   832,   833,   834,   835,   841,   842,   851,   860,
     872,   873,   881,   892,   893,   902,   914,   915,   925,   935,
     945,   958,   959,   970,   984,   985,   997,   998,  1010,  1011,
    1023,  1024,  1037,  1038,  1051,  1052,  1065,  1066,  1083,  1084,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1112,  1115,  1126,  1134,  1135,  1141,  1149,  1185,  1188,
    1195,  1203,  1224,  1243,  1254,  1283,  1288,  1298,  1303,  1313,
    1316,  1319,  1322,  1328,  1335,  1338,  1359,  1377,  1398,  1431,
    1467,  1490,  1494,  1508,  1527,  1550,  1585,  1624,  1644,  1730,
    1740,  1768,  1771,  1777,  1785,  1794,  1802,  1805,  1821,  1831,
    1839,  1843,  1847,  1851,  1860,  1865,  1870,  1875,  1880,  1885,
    1890,  1895,  1900,  1905,  1911,  1917,  1923,  1928,  1933,  1938,
    1943,  1948,  1953,  1963,  1973,  1978,  1993,  1996,  1999,  2006,
    2016,  2024,  2027,  2042,  2068,  2072,  2078,  2083,  2096,  2100,
    2104,  2105,  2111,  2112,  2113,  2114,  2115,  2119,  2120,  2120,
    2120,  2128,  2129,  2134,  2137,  2145,  2148,  2154,  2155,  2159,
    2167,  2171,  2181,  2186,  2203,  2203,  2208,  2208,  2215,  2215,
    2228,  2231,  2237,  2240,  2246,  2250,  2257,  2264,  2271,  2278,
    2289,  2298,  2302,  2309,  2312,  2318,  2318
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ATTRIBUTE", "CONST_QUAL", "BOOL_TYPE",
  "FLOAT_TYPE", "INT_TYPE", "BREAK", "CONTINUE", "DO", "ELSE", "FOR", "IF",
  "DISCARD", "RETURN", "BVEC2", "BVEC3", "BVEC4", "IVEC2", "IVEC3",
  "IVEC4", "VEC2", "VEC3", "VEC4", "MATRIX2", "MATRIX3", "MATRIX4",
  "IN_QUAL", "OUT_QUAL", "INOUT_QUAL", "UNIFORM", "VARYING", "STRUCT",
  "VOID_TYPE", "WHILE", "SAMPLER1D", "SAMPLER2D", "SAMPLER3D",
  "SAMPLERCUBE", "SAMPLER1DSHADOW", "SAMPLER2DSHADOW", "SAMPLERRECTARB",
  "SAMPLERRECTSHADOWARB", "LOW_PRECISION", "MEDIUM_PRECISION",
  "HIGH_PRECISION", "INVARIANT", "PRECISION", "IDENTIFIER", "TYPE_NAME",
  "FLOATCONSTANT", "INTCONSTANT", "BOOLCONSTANT", "FIELD_SELECTION",
  "LEFT_OP", "RIGHT_OP", "INC_OP", "DEC_OP", "LE_OP", "GE_OP", "EQ_OP",
  "NE_OP", "AND_OP", "OR_OP", "XOR_OP", "MUL_ASSIGN", "DIV_ASSIGN",
  "ADD_ASSIGN", "MOD_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN",
  "XOR_ASSIGN", "OR_ASSIGN", "SUB_ASSIGN", "LEFT_PAREN", "RIGHT_PAREN",
  "LEFT_BRACKET", "RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE", "DOT",
  "COMMA", "COLON", "EQUAL", "SEMICOLON", "BANG", "DASH", "TILDE", "PLUS",
  "STAR", "SLASH", "PERCENT", "LEFT_ANGLE", "RIGHT_ANGLE", "VERTICAL_BAR",
  "CARET", "AMPERSAND", "QUESTION", "$accept", "variable_identifier",
  "primary_expression", "postfix_expression", "integer_expression",
  "function_call", "function_call_or_method", "function_call_generic",
  "function_call_header_no_parameters",
  "function_call_header_with_parameters", "function_call_header",
  "function_identifier", "unary_expression", "unary_operator",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_xor_expression",
  "logical_or_expression", "conditional_expression",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "declaration", "function_prototype",
  "function_declarator", "function_header_with_parameters",
  "function_header", "parameter_declarator", "parameter_declaration",
  "parameter_qualifier", "parameter_type_specifier",
  "init_declarator_list", "single_declaration", "fully_specified_type",
  "type_qualifier", "type_specifier", "type_specifier_nonarray",
  "type_specifier_nonarray_no_prec", "precision_qualifier",
  "struct_specifier", "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "initializer",
  "declaration_statement", "statement", "simple_statement",
  "compound_statement", "@1", "@2", "statement_no_new_scope",
  "compound_statement_no_new_scope", "statement_list",
  "expression_statement", "selection_statement",
  "selection_rest_statement", "condition", "iteration_statement", "@3",
  "@4", "@5", "for_init_statement", "conditionopt", "for_rest_statement",
  "jump_statement", "translation_unit", "external_declaration",
  "function_definition", "@6", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   100,   101,   102,   102,   102,   102,   102,   103,   103,
     103,   103,   103,   103,   104,   105,   106,   106,   107,   107,
     108,   108,   109,   109,   110,   111,   111,   111,   112,   112,
     112,   112,   113,   113,   113,   113,   114,   114,   114,   114,
     115,   115,   115,   116,   116,   116,   117,   117,   117,   117,
     117,   118,   118,   118,   119,   119,   120,   120,   121,   121,
     122,   122,   123,   123,   124,   124,   125,   125,   126,   126,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   128,   128,   129,   130,   130,   130,   131,   132,   132,
     133,   133,   134,   135,   135,   136,   136,   136,   136,   137,
     137,   137,   137,   138,   139,   139,   139,   139,   139,   139,
     139,   140,   140,   140,   140,   140,   140,   140,   140,   141,
     141,   142,   142,   142,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   146,   146,   146,   147,
     147,   148,   148,   149,   150,   150,   151,   151,   152,   153,
     154,   154,   155,   155,   155,   155,   155,   156,   157,   158,
     156,   159,   159,   160,   160,   161,   161,   162,   162,   163,
     164,   164,   165,   165,   167,   166,   168,   166,   169,   166,
     170,   170,   171,   171,   172,   172,   173,   173,   173,   173,
     173,   174,   174,   175,   175,   177,   176
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     3,     1,     4,
       1,     3,     2,     2,     1,     1,     1,     3,     2,     2,
       2,     1,     2,     3,     2,     1,     1,     1,     1,     2,
       2,     2,     1,     1,     1,     1,     1,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     5,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     2,     4,     2,     1,     1,
       2,     3,     3,     2,     5,     3,     2,     3,     2,     0,
       1,     1,     1,     1,     1,     3,     5,     6,     7,     8,
       5,     1,     2,     4,     5,     6,     7,     4,     2,     1,
       2,     1,     1,     1,     2,     1,     1,     4,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     5,
       4,     1,     2,     3,     1,     3,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     0,     0,
       5,     1,     1,     2,     3,     1,     2,     1,     2,     5,
       3,     1,     1,     4,     0,     6,     0,     8,     0,     7,
       1,     1,     1,     0,     2,     3,     2,     2,     2,     3,
       2,     1,     2,     1,     1,     0,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   122,   121,   133,   131,   132,   137,   138,   139,   140,
     141,   142,   134,   135,   136,   143,   144,   145,   125,   123,
       0,   130,   146,   147,   148,   149,   150,   151,   152,   153,
     158,   157,   156,     0,     0,   155,   214,   215,     0,    89,
      99,     0,   104,   111,     0,   119,   126,   128,     0,   154,
       0,   211,   213,     0,     0,   124,   118,     0,    84,     0,
      87,    99,   100,   101,   102,     0,    90,     0,    99,     0,
      85,   112,   120,     0,   129,     1,   212,     0,     0,     0,
     161,     0,     0,   216,    91,    96,    98,   103,     0,   105,
      92,     0,     0,     2,     5,     4,     6,    27,     0,     0,
       0,    34,    33,    35,    32,     3,     8,    28,    10,    15,
      16,     0,     0,    21,     0,    36,     0,    40,    43,    46,
      51,    54,    56,    58,    60,    62,    64,    66,    83,     0,
      25,     0,   166,     0,   164,   160,   162,    86,     0,     0,
     196,     0,     0,     0,     0,     0,   178,   183,   187,    36,
      68,    81,     0,   169,     0,   119,   172,   185,   171,   170,
       0,   173,   174,   175,   176,    93,    95,    97,     0,     0,
     113,     0,   168,   117,    29,    30,     0,    12,    13,     0,
       0,    19,    18,     0,   130,    22,    24,    31,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   127,   159,
       0,     0,   163,   207,   206,     0,   198,     0,   210,   208,
       0,   194,   177,     0,    71,    72,    74,    73,    76,    77,
      78,    79,    80,    75,    70,     0,     0,   188,   184,   186,
       0,   106,     0,   110,     0,   114,     7,     0,    14,    26,
      11,    17,    23,    37,    38,    39,    42,    41,    44,    45,
      49,    50,    47,    48,    52,    53,    55,    57,    59,    61,
      63,    65,     0,     0,   165,     0,     0,     0,   209,     0,
     179,    69,    82,     0,     0,   107,   115,     0,     9,     0,
     167,     0,   201,   200,   203,     0,   192,     0,     0,     0,
      94,   108,     0,   116,    67,     0,   202,     0,     0,   191,
     189,     0,     0,   180,   109,     0,   204,     0,     0,     0,
     182,   195,   181,     0,   205,   199,   190,   193,   197
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,   105,   106,   107,   247,   108,   109,   110,   111,   112,
     113,   114,   149,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   150,   151,   235,   152,   129,
     153,   154,    38,    39,    40,    85,    66,    67,    86,    41,
      42,    43,    44,   130,    46,    47,    48,    49,    79,    80,
     133,   134,   173,   156,   157,   158,   159,   223,   299,   321,
     322,   160,   161,   162,   310,   298,   163,   279,   215,   276,
     294,   307,   308,   164,    50,    51,    52,    59
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -262
static const short int yypact[] =
{
    1303,  -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,
    -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,
     -32,  -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,  -262,
    -262,  -262,  -262,   -28,    84,  -262,  -262,   -67,   -26,   -38,
       6,   -10,  -262,    31,  1349,  -262,   -22,  -262,  1391,  -262,
    1205,  -262,  -262,    26,  1349,  -262,  -262,  1391,  -262,    40,
    -262,    11,  -262,  -262,  -262,    59,  -262,  1349,   104,    62,
    -262,   -15,  -262,   958,  -262,  -262,  -262,  1349,    77,  1108,
    -262,    52,   316,  -262,  -262,  -262,  -262,    91,  1349,   -62,
    -262,   220,   958,   105,  -262,  -262,  -262,  -262,   958,   958,
     958,  -262,  -262,  -262,  -262,  -262,  -262,     8,  -262,  -262,
    -262,   108,   -57,  1033,   110,  -262,   958,    44,    35,    87,
     -35,    85,    89,    92,    94,   125,   126,   -52,  -262,   113,
    -262,  1154,   116,    24,  -262,  -262,  -262,  -262,   109,   114,
    -262,   127,   128,   115,   808,   129,   121,  -262,  -262,    28,
    -262,  -262,    32,  -262,   -67,   130,  -262,  -262,  -262,  -262,
     404,  -262,  -262,  -262,  -262,   131,  -262,  -262,   883,   958,
     122,   133,  -262,  -262,  -262,  -262,    -9,  -262,  -262,   958,
    1251,  -262,  -262,   958,   137,  -262,  -262,  -262,   958,   958,
     958,   958,   958,   958,   958,   958,   958,   958,   958,   958,
     958,   958,   958,   958,   958,   958,   958,   958,  -262,  -262,
     958,    77,  -262,  -262,  -262,   492,  -262,   958,  -262,  -262,
      33,  -262,  -262,   492,  -262,  -262,  -262,  -262,  -262,  -262,
    -262,  -262,  -262,  -262,  -262,   958,   958,  -262,  -262,  -262,
     958,   123,   138,  -262,   958,   134,  -262,   139,   141,  -262,
     140,  -262,  -262,  -262,  -262,  -262,    44,    44,    35,    35,
      87,    87,    87,    87,   -35,   -35,    85,    89,    92,    94,
     125,   126,    65,   143,  -262,   194,   658,    -8,  -262,   733,
     492,  -262,  -262,   151,   958,   146,  -262,   958,  -262,   958,
    -262,   156,  -262,  -262,   733,   492,   141,   184,   157,   154,
    -262,  -262,   958,  -262,  -262,   958,  -262,   162,   172,   239,
    -262,   166,   580,  -262,  -262,    10,   958,   580,   492,   958,
    -262,  -262,  -262,   169,   141,  -262,  -262,  -262,  -262
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -262,  -262,  -262,  -262,  -262,  -262,  -262,    72,  -262,  -262,
    -262,  -262,   -27,  -262,   -40,   -39,  -168,   -41,    66,    73,
      78,    64,    79,    76,  -262,   -60,   -91,  -262,   -95,   -83,
       2,     7,  -262,  -262,  -262,   195,   224,   218,   199,  -262,
    -262,  -261,   -29,     0,  -262,    57,   254,  -262,   212,   -76,
    -262,    80,  -163,    14,   -98,  -208,  -262,  -262,  -262,   -25,
     234,    74,    22,  -262,  -262,     9,  -262,  -262,  -262,  -262,
    -262,  -262,  -262,  -262,  -262,   250,  -262,  -262
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -89
static const short int yytable[] =
{
      45,   172,    36,   136,    55,   176,   243,    37,   171,     1,
       2,    68,   206,   128,     1,     2,   168,    53,   297,    58,
     182,    56,   185,   169,   195,   196,   183,   260,   261,   262,
     263,   128,    68,   297,    62,    63,    64,    18,    19,    62,
      63,    64,    18,    19,    72,    61,   115,   207,    54,   220,
      45,    60,    36,    65,    78,   136,    73,    37,    65,   197,
     198,    90,   239,    91,   115,   177,   178,    87,   246,   295,
      92,   174,   175,    69,   236,   236,    70,    78,   172,    78,
      71,   286,   155,   -88,   248,   242,   179,   323,    87,   187,
     180,    55,   252,   236,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   320,    74,    77,   211,   128,   320,
     212,    89,   272,   234,    81,   236,   236,   275,   237,   278,
      82,   301,   277,   191,   303,   192,   132,   273,    30,    31,
      32,    78,    62,    63,    64,   188,   189,   190,   137,   314,
     165,   115,   193,   194,   281,   282,   199,   200,   236,   289,
     128,   256,   257,   172,   258,   259,   327,   283,   264,   265,
     155,   253,   254,   255,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     128,   -26,   239,   115,   296,   181,   186,   201,   204,   202,
     203,   205,   208,   172,   210,   213,   172,   309,   304,   296,
     214,   218,   222,   216,   217,   221,   -25,   244,   284,   240,
     315,   172,   245,   115,   -20,   155,   -27,   285,   288,   287,
     326,   324,   290,   155,   236,     3,     4,     5,   172,   291,
     300,   302,   305,   311,   312,   313,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,   316,   317,
     318,   319,   251,    20,    21,   328,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,   266,   269,    93,
      35,    94,    95,    96,    97,   267,   155,    98,    99,   155,
     155,   268,   271,   166,   270,    84,    88,   167,    57,   131,
     292,   274,   325,    83,   155,   155,   100,   280,   293,   170,
      76,     0,     0,   306,     0,     0,     0,   101,   102,   103,
     104,     0,   155,     0,     0,     0,     0,   155,   155,     1,
       2,     3,     4,     5,   138,   139,   140,     0,   141,   142,
     143,   144,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     0,     0,     0,    18,    19,    20,
      21,   145,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    93,    35,    94,    95,    96,
      97,     0,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   100,     0,     0,     0,   146,   147,     0,     0,
       0,     0,   148,   101,   102,   103,   104,     1,     2,     3,
       4,     5,   138,   139,   140,     0,   141,   142,   143,   144,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,     0,    18,    19,    20,    21,   145,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    93,    35,    94,    95,    96,    97,     0,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     100,     0,     0,     0,   146,   238,     0,     0,     0,     0,
     148,   101,   102,   103,   104,     1,     2,     3,     4,     5,
     138,   139,   140,     0,   141,   142,   143,   144,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,     0,    18,    19,    20,    21,   145,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    93,    35,    94,    95,    96,    97,     0,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   100,     0,
       0,     0,   146,     0,     0,     0,     0,     0,   148,   101,
     102,   103,   104,     1,     2,     3,     4,     5,   138,   139,
     140,     0,   141,   142,   143,   144,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,    18,    19,    20,    21,   145,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    93,
      35,    94,    95,    96,    97,     0,     0,    98,    99,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   100,     0,     0,     0,
      82,     1,     2,     3,     4,     5,   148,   101,   102,   103,
     104,     0,     0,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,     0,     0,     0,    18,
      19,    20,    21,     0,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    93,    35,    94,
      95,    96,    97,     0,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,     0,     1,     2,     3,     4,
       5,     0,     0,     0,   148,   101,   102,   103,   104,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,     0,    18,    19,    20,    21,     0,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      65,     0,    93,    35,    94,    95,    96,    97,     0,     0,
      98,    99,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
       0,     0,     0,     3,     4,     5,     0,     0,     0,     0,
     101,   102,   103,   104,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,     0,     0,     0,     0,
       0,    20,    21,     0,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     0,     0,    93,    35,    94,
      95,    96,    97,     0,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,     0,     0,     0,     3,     4,
       5,     0,     0,     0,   219,   101,   102,   103,   104,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,     0,     0,     0,    20,    21,     0,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       0,     0,    93,    35,    94,    95,    96,    97,     0,     0,
      98,    99,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
       0,     0,   241,     3,     4,     5,     0,     0,     0,     0,
     101,   102,   103,   104,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,     0,     0,     0,     0,
       0,    20,    21,     0,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     0,     0,    93,    35,    94,
      95,    96,    97,     0,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,     0,     0,     0,     3,     4,
       5,     0,     0,     0,     0,   101,   102,   103,   104,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,     0,     0,     0,    20,   184,     0,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       0,     0,    93,    35,    94,    95,    96,    97,     0,     0,
      98,    99,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
       0,     0,     0,     3,     4,     5,     0,     0,     0,     0,
     101,   102,   103,   104,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,     0,     0,     0,     0,
       0,    20,    21,     0,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,     0,     0,     0,    35,     3,
       4,     5,     0,     0,     0,     0,     0,     0,     0,     0,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,     0,     0,     0,    20,    21,   135,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,     0,     0,     0,    35,    75,     0,     0,     1,     2,
       3,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,   209,    18,    19,    20,    21,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,     0,    35,     3,     4,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,     0,    20,    21,     0,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,     0,     0,
     249,    35,     0,     0,     0,   250,     1,     2,     3,     4,
       5,     0,     0,     0,     0,     0,     0,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,     0,    18,    19,    20,    21,     0,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     0,    35,     3,     4,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,     0,
       0,     0,    20,    21,     0,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,     3,     4,     5,    35,
       0,     0,     0,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,     0,    20,    21,     0,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,     0,
       0,    35
};

static const short int yycheck[] =
{
       0,    92,     0,    79,    32,   100,   169,     0,    91,     3,
       4,    40,    64,    73,     3,     4,    78,    49,   279,    86,
      77,    49,   113,    85,    59,    60,    83,   195,   196,   197,
     198,    91,    61,   294,    28,    29,    30,    31,    32,    28,
      29,    30,    31,    32,    44,    83,    73,    99,    80,   144,
      50,    77,    50,    47,    54,   131,    78,    50,    47,    94,
      95,    76,   160,    78,    91,    57,    58,    67,    77,    77,
      85,    98,    99,    83,    83,    83,    86,    77,   169,    79,
      49,   244,    82,    77,   179,   168,    78,    77,    88,   116,
      82,    32,   183,    83,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,   312,    48,    80,    83,   168,   317,
      86,    49,   207,    85,    57,    83,    83,   215,    86,    86,
      80,   284,   217,    88,   287,    90,    49,   210,    44,    45,
      46,   131,    28,    29,    30,    91,    92,    93,    86,   302,
      49,   168,    55,    56,   235,   236,    61,    62,    83,    84,
     210,   191,   192,   244,   193,   194,   319,   240,   199,   200,
     160,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     240,    76,   280,   210,   279,    77,    76,    98,    63,    97,
      96,    65,    79,   284,    78,    86,   287,   295,   289,   294,
      86,    86,    81,    76,    76,    76,    76,    85,    85,    78,
     305,   302,    79,   240,    77,   215,    76,    79,    79,    85,
     318,   316,    79,   223,    83,     5,     6,     7,   319,    35,
      79,    85,    76,    49,    77,    81,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    86,    77,
      11,    85,   180,    33,    34,    86,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,   201,   204,    49,
      50,    51,    52,    53,    54,   202,   276,    57,    58,   279,
     280,   203,   206,    88,   205,    61,    68,    88,    34,    77,
     276,   211,   317,    59,   294,   295,    76,   223,   276,    79,
      50,    -1,    -1,   294,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,   312,    -1,    -1,    -1,    -1,   317,   318,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    -1,    -1,    57,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    -1,    -1,    -1,    80,    81,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
      -1,    57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    80,    81,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      -1,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,    57,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    -1,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    -1,    -1,    57,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,
      80,     3,     4,     5,     6,     7,    86,    87,    88,    89,
      90,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    31,
      32,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    57,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    86,    87,    88,    89,    90,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    -1,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    50,    51,    52,    53,    54,    -1,    -1,
      57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    -1,     5,     6,     7,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    49,    50,    51,
      52,    53,    54,    -1,    -1,    57,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,     5,     6,
       7,    -1,    -1,    -1,    86,    87,    88,    89,    90,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    -1,    -1,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      -1,    -1,    49,    50,    51,    52,    53,    54,    -1,    -1,
      57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    79,     5,     6,     7,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    49,    50,    51,
      52,    53,    54,    -1,    -1,    57,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,     5,     6,
       7,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    -1,    -1,    -1,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      -1,    -1,    49,    50,    51,    52,    53,    54,    -1,    -1,
      57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    -1,     5,     6,     7,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    -1,
      -1,    33,    34,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    50,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    -1,    -1,    -1,    33,    34,    81,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    -1,    -1,    -1,    50,     0,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    -1,    -1,    81,    31,    32,    33,    34,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    -1,    -1,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    -1,
      49,    50,    -1,    -1,    -1,    54,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    -1,    -1,    -1,    31,    32,    33,    34,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    -1,    -1,    -1,
      -1,    -1,    33,    34,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,     5,     6,     7,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    -1,    -1,    33,    34,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    31,    32,
      33,    34,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    50,   130,   131,   132,   133,
     134,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     174,   175,   176,    49,    80,    32,    49,   146,    86,   177,
      77,    83,    28,    29,    30,    47,   136,   137,   142,    83,
      86,    49,   143,    78,   145,     0,   175,    80,   143,   148,
     149,   145,    80,   160,   136,   135,   138,   143,   137,    49,
      76,    78,    85,    49,    51,    52,    53,    54,    57,    58,
      76,    87,    88,    89,    90,   101,   102,   103,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   129,
     143,   148,    49,   150,   151,    81,   149,    86,     8,     9,
      10,    12,    13,    14,    15,    35,    80,    81,    86,   112,
     125,   126,   128,   130,   131,   143,   153,   154,   155,   156,
     161,   162,   163,   166,   173,    49,   135,   138,    78,    85,
      79,   129,   126,   152,   112,   112,   128,    57,    58,    78,
      82,    77,    77,    83,    34,   126,    76,   112,    91,    92,
      93,    88,    90,    55,    56,    59,    60,    94,    95,    61,
      62,    98,    97,    96,    63,    65,    64,    99,    79,    81,
      78,    83,    86,    86,    86,   168,    76,    76,    86,    86,
     128,    76,    81,   157,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    85,   127,    83,    86,    81,   154,
      78,    79,   129,   152,    85,    79,    77,   104,   128,    49,
      54,   107,   126,   112,   112,   112,   114,   114,   115,   115,
     116,   116,   116,   116,   117,   117,   118,   119,   120,   121,
     122,   123,   128,   129,   151,   154,   169,   128,    86,   167,
     161,   126,   126,   129,    85,    79,   152,    85,    79,    84,
      79,    35,   153,   162,   170,    77,   128,   141,   165,   158,
      79,   152,    85,   152,   126,    76,   165,   171,   172,   154,
     164,    49,    77,    81,   152,   128,    86,    77,    11,    85,
     155,   159,   160,    77,   128,   159,   154,   152,    86
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

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
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
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

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
        case 2:
#line 226 "glslang.y"
    {
        // The symbol table search was done in the lexical phase
        const TSymbol* symbol = yyvsp[0].lex.symbol;
        const TVariable* variable;
        if (symbol == 0) {
            parseContext.error(yyvsp[0].lex.line, "undeclared identifier", yyvsp[0].lex.string->c_str(), "");
            parseContext.recover();
            TType type(EbtFloat);
            TVariable* fakeVariable = new TVariable(yyvsp[0].lex.string, type);
            parseContext.symbolTable.insert(*fakeVariable);
            variable = fakeVariable;
        } else {
            // This identifier can only be a variable type symbol 
            if (! symbol->isVariable()) {
                parseContext.error(yyvsp[0].lex.line, "variable expected", yyvsp[0].lex.string->c_str(), "");
                parseContext.recover();
            }
            variable = static_cast<const TVariable*>(symbol);
        }

        // don't delete $1.string, it's used by error recovery, and the pool
        // pop will reclaim the memory

        if (variable->getType().getQualifier() == EvqConst ) {
            constUnion* constArray = variable->getConstPointer();
            TType t(variable->getType());
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(constArray, t, yyvsp[0].lex.line);        
        } else
            yyval.interm.intermTypedNode = parseContext.intermediate.addSymbol(variable->getUniqueId(), 
                                                     variable->getName(), 
                                                     variable->getType(), yyvsp[0].lex.line);
    ;}
    break;

  case 3:
#line 261 "glslang.y"
    {
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 4:
#line 264 "glslang.y"
    {
        //
        // INT_TYPE is only 16-bit plus sign bit for vertex/fragment shaders, 
        // check for overflow for constants
        //
        if (abs(yyvsp[0].lex.i) >= (1 << 16)) {
            parseContext.error(yyvsp[0].lex.line, " integer constant overflow", "", "");
            parseContext.recover();
        }
        constUnion *unionArray = new constUnion[1];
        unionArray->setIConst(yyvsp[0].lex.i);
        yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EvqConst), yyvsp[0].lex.line);
    ;}
    break;

  case 5:
#line 277 "glslang.y"
    {
        constUnion *unionArray = new constUnion[1];
        unionArray->setFConst(yyvsp[0].lex.f);
        yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtFloat, EvqConst), yyvsp[0].lex.line);
    ;}
    break;

  case 6:
#line 282 "glslang.y"
    {
        constUnion *unionArray = new constUnion[1];
        unionArray->setBConst(yyvsp[0].lex.b);
        yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[0].lex.line);
    ;}
    break;

  case 7:
#line 287 "glslang.y"
    {
        yyval.interm.intermTypedNode = yyvsp[-1].interm.intermTypedNode;
    ;}
    break;

  case 8:
#line 293 "glslang.y"
    { 
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 9:
#line 296 "glslang.y"
    {
        if (!yyvsp[-3].interm.intermTypedNode->isArray() && !yyvsp[-3].interm.intermTypedNode->isMatrix() && !yyvsp[-3].interm.intermTypedNode->isVector()) {
            if (yyvsp[-3].interm.intermTypedNode->getAsSymbolNode())
                parseContext.error(yyvsp[-2].lex.line, " left of '[' is not of type array, matrix, or vector ", yyvsp[-3].interm.intermTypedNode->getAsSymbolNode()->getSymbol().c_str(), "");
            else
                parseContext.error(yyvsp[-2].lex.line, " left of '[' is not of type array, matrix, or vector ", "expression", "");
            parseContext.recover();
        }
        if (yyvsp[-3].interm.intermTypedNode->getType().getQualifier() == EvqConst && yyvsp[-1].interm.intermTypedNode->getQualifier() == EvqConst) {
            if (yyvsp[-3].interm.intermTypedNode->isArray()) { // constant folding for arrays
                yyval.interm.intermTypedNode = parseContext.addConstArrayNode(yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst(), yyvsp[-3].interm.intermTypedNode, yyvsp[-2].lex.line);
            } else if (yyvsp[-3].interm.intermTypedNode->isVector()) {  // constant folding for vectors
                TVectorFields fields;                
                fields.num = 1;
                fields.offsets[0] = yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst(); // need to do it this way because v.xy sends fields integer array
                yyval.interm.intermTypedNode = parseContext.addConstVectorNode(fields, yyvsp[-3].interm.intermTypedNode, yyvsp[-2].lex.line);
            } else if (yyvsp[-3].interm.intermTypedNode->isMatrix()) { // constant folding for matrices
                yyval.interm.intermTypedNode = parseContext.addConstMatrixNode(yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst(), yyvsp[-3].interm.intermTypedNode, yyvsp[-2].lex.line);
            } 
        } else {
            if (yyvsp[-1].interm.intermTypedNode->getQualifier() == EvqConst) {
                if ((yyvsp[-3].interm.intermTypedNode->isVector() || yyvsp[-3].interm.intermTypedNode->isMatrix()) && yyvsp[-3].interm.intermTypedNode->getType().getNominalSize() <= yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst() && !yyvsp[-3].interm.intermTypedNode->isArray() ) {
                    parseContext.error(yyvsp[-2].lex.line, "", "[", "field selection out of range '%d'", yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst());
                    parseContext.recover();
                } else {
                    if (yyvsp[-3].interm.intermTypedNode->isArray()) {
                        if (yyvsp[-3].interm.intermTypedNode->getType().getArraySize() == 0) {
                            if (yyvsp[-3].interm.intermTypedNode->getType().getMaxArraySize() <= yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst()) {
                                if (parseContext.arraySetMaxSize(yyvsp[-3].interm.intermTypedNode->getAsSymbolNode(), yyvsp[-3].interm.intermTypedNode->getTypePointer(), yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst(), true, yyvsp[-2].lex.line))
                                    parseContext.recover(); 
                            } else {
                                if (parseContext.arraySetMaxSize(yyvsp[-3].interm.intermTypedNode->getAsSymbolNode(), yyvsp[-3].interm.intermTypedNode->getTypePointer(), 0, false, yyvsp[-2].lex.line))
                                    parseContext.recover(); 
                            }
                        } else if ( yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst() >= yyvsp[-3].interm.intermTypedNode->getType().getArraySize()) {
                            parseContext.error(yyvsp[-2].lex.line, "", "[", "array index out of range '%d'", yyvsp[-1].interm.intermTypedNode->getAsConstantUnion()->getUnionArrayPointer()->getIConst());
                            parseContext.recover();
                        }
                    }
                    yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpIndexDirect, yyvsp[-3].interm.intermTypedNode, yyvsp[-1].interm.intermTypedNode, yyvsp[-2].lex.line);
                }
            } else {
                if (yyvsp[-3].interm.intermTypedNode->isArray() && yyvsp[-3].interm.intermTypedNode->getType().getArraySize() == 0) {
                    parseContext.error(yyvsp[-2].lex.line, "", "[", "array must be redeclared with a size before being indexed with a variable");
                    parseContext.recover();
                }
                
                yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpIndexIndirect, yyvsp[-3].interm.intermTypedNode, yyvsp[-1].interm.intermTypedNode, yyvsp[-2].lex.line);
            }
        } 
        if (yyval.interm.intermTypedNode == 0) {
            constUnion *unionArray = new constUnion[1];
            unionArray->setFConst(0.0f);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtFloat, EvqConst), yyvsp[-2].lex.line);
        } else if (yyvsp[-3].interm.intermTypedNode->isArray()) {
            if (yyvsp[-3].interm.intermTypedNode->getType().getStruct())
                yyval.interm.intermTypedNode->setType(TType(yyvsp[-3].interm.intermTypedNode->getType().getStruct(), yyvsp[-3].interm.intermTypedNode->getType().getTypeName()));
            else
                //Altered for ESSL support
                yyval.interm.intermTypedNode->setType(TType(yyvsp[-3].interm.intermTypedNode->getBasicType(), EvqTemporary, yyvsp[-3].interm.intermTypedNode->getPrecision(), yyvsp[-3].interm.intermTypedNode->getNominalSize(), yyvsp[-3].interm.intermTypedNode->isMatrix()));
                
            if (yyvsp[-3].interm.intermTypedNode->getType().getQualifier() == EvqConst)
                yyval.interm.intermTypedNode->getTypePointer()->changeQualifier(EvqConst);
        } else if (yyvsp[-3].interm.intermTypedNode->isMatrix() && yyvsp[-3].interm.intermTypedNode->getType().getQualifier() == EvqConst)         
			//Altered for ESSL support
            yyval.interm.intermTypedNode->setType(TType(yyvsp[-3].interm.intermTypedNode->getBasicType(), EvqConst, yyvsp[-3].interm.intermTypedNode->getPrecision(), yyvsp[-3].interm.intermTypedNode->getNominalSize()));     
        else if (yyvsp[-3].interm.intermTypedNode->isMatrix())            
			//Altered for ESSL support
            yyval.interm.intermTypedNode->setType(TType(yyvsp[-3].interm.intermTypedNode->getBasicType(), EvqTemporary, yyvsp[-3].interm.intermTypedNode->getPrecision(), yyvsp[-3].interm.intermTypedNode->getNominalSize()));     
        else if (yyvsp[-3].interm.intermTypedNode->isVector() && yyvsp[-3].interm.intermTypedNode->getType().getQualifier() == EvqConst)          
            yyval.interm.intermTypedNode->setType(TType(yyvsp[-3].interm.intermTypedNode->getBasicType(), EvqConst));     
        else if (yyvsp[-3].interm.intermTypedNode->isVector())       
            yyval.interm.intermTypedNode->setType(TType(yyvsp[-3].interm.intermTypedNode->getBasicType(), EvqTemporary));
        else
            yyval.interm.intermTypedNode->setType(yyvsp[-3].interm.intermTypedNode->getType()); 
    ;}
    break;

  case 10:
#line 372 "glslang.y"
    {
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;

    ;}
    break;

  case 11:
#line 376 "glslang.y"
    {        
        if (yyvsp[-2].interm.intermTypedNode->isArray()) {
            parseContext.error(yyvsp[0].lex.line, "cannot apply dot operator to an array", ".", "");
            parseContext.recover();
        }

        if (yyvsp[-2].interm.intermTypedNode->isVector()) {
            TVectorFields fields;
            if (! parseContext.parseVectorFields(*yyvsp[0].lex.string, yyvsp[-2].interm.intermTypedNode->getNominalSize(), fields, yyvsp[0].lex.line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

            if (yyvsp[-2].interm.intermTypedNode->getType().getQualifier() == EvqConst) { // constant folding for vector fields
                yyval.interm.intermTypedNode = parseContext.addConstVectorNode(fields, yyvsp[-2].interm.intermTypedNode, yyvsp[0].lex.line);
                if (yyval.interm.intermTypedNode == 0) {
                    parseContext.recover();
                    yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
                }
                else
					//Altered for ESSL support
                    yyval.interm.intermTypedNode->setType(TType(yyvsp[-2].interm.intermTypedNode->getBasicType(), EvqConst, yyvsp[-2].interm.intermTypedNode->getPrecision(), (int) (*yyvsp[0].lex.string).size()));
            } else {
                if (fields.num == 1) {
                    constUnion *unionArray = new constUnion[1];
                    unionArray->setIConst(fields.offsets[0]);
                    TIntermTyped* index = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EvqConst), yyvsp[0].lex.line);
                    yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpIndexDirect, yyvsp[-2].interm.intermTypedNode, index, yyvsp[-1].lex.line);
                    yyval.interm.intermTypedNode->setType(TType(yyvsp[-2].interm.intermTypedNode->getBasicType()));
                } else {
                    TString vectorString = *yyvsp[0].lex.string;
                    TIntermTyped* index = parseContext.intermediate.addSwizzle(fields, yyvsp[0].lex.line);                
                    yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpVectorSwizzle, yyvsp[-2].interm.intermTypedNode, index, yyvsp[-1].lex.line);
					//Altered for ESSL support
                    yyval.interm.intermTypedNode->setType(TType(yyvsp[-2].interm.intermTypedNode->getBasicType(),EvqTemporary, yyvsp[-2].interm.intermTypedNode->getPrecision(), (int) vectorString.size()));  
                }
            }
        } else if (yyvsp[-2].interm.intermTypedNode->isMatrix()) {
            TMatrixFields fields;
            if (! parseContext.parseMatrixFields(*yyvsp[0].lex.string, yyvsp[-2].interm.intermTypedNode->getNominalSize(), fields, yyvsp[0].lex.line)) {
                fields.wholeRow = false;
                fields.wholeCol = false;
                fields.row = 0;
                fields.col = 0;
                parseContext.recover();
            }

            if (fields.wholeRow || fields.wholeCol) {
                parseContext.error(yyvsp[-1].lex.line, " non-scalar fields not implemented yet", ".", "");
                parseContext.recover();
                constUnion *unionArray = new constUnion[1];
                unionArray->setIConst(0);
                TIntermTyped* index = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EvqConst), yyvsp[0].lex.line);
                yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpIndexDirect, yyvsp[-2].interm.intermTypedNode, index, yyvsp[-1].lex.line);                
				//Altered for ESSL support
                yyval.interm.intermTypedNode->setType(TType(yyvsp[-2].interm.intermTypedNode->getBasicType(), EvqTemporary, yyvsp[-2].interm.intermTypedNode->getPrecision(),yyvsp[-2].interm.intermTypedNode->getNominalSize()));
            } else {
                constUnion *unionArray = new constUnion[1];
                unionArray->setIConst(fields.col * yyvsp[-2].interm.intermTypedNode->getNominalSize() + fields.row);
                TIntermTyped* index = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EvqConst), yyvsp[0].lex.line);
                yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpIndexDirect, yyvsp[-2].interm.intermTypedNode, index, yyvsp[-1].lex.line);                
                yyval.interm.intermTypedNode->setType(TType(yyvsp[-2].interm.intermTypedNode->getBasicType()));
            }
        } else if (yyvsp[-2].interm.intermTypedNode->getBasicType() == EbtStruct) {
            bool fieldFound = false;
            TTypeList* fields = yyvsp[-2].interm.intermTypedNode->getType().getStruct();
            if (fields == 0) {
                parseContext.error(yyvsp[-1].lex.line, "structure has no fields", "Internal Error", "");
                parseContext.recover();
                yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
            } else {
                unsigned int i;
                for (i = 0; i < fields->size(); ++i) {
                    if ((*fields)[i].type->getFieldName() == *yyvsp[0].lex.string) {
                        fieldFound = true;
                        break;
                    }                
                }
                if (fieldFound) {
                    if (yyvsp[-2].interm.intermTypedNode->getType().getQualifier() == EvqConst) {
                        yyval.interm.intermTypedNode = parseContext.addConstStruct(*yyvsp[0].lex.string, yyvsp[-2].interm.intermTypedNode, yyvsp[-1].lex.line);
                        if (yyval.interm.intermTypedNode == 0) {
                            parseContext.recover();
                            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
                        }
                        else {
                            yyval.interm.intermTypedNode->setType(*(*fields)[i].type);
                            // change the qualifier of the return type, not of the structure field
                            // as the structure definition is shared between various structures.
                            yyval.interm.intermTypedNode->getTypePointer()->changeQualifier(EvqConst);
                        }
                    } else {
                        constUnion *unionArray = new constUnion[1];
                        unionArray->setIConst(i);
                        TIntermTyped* index = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EvqConst), yyvsp[0].lex.line);
                        yyval.interm.intermTypedNode = parseContext.intermediate.addIndex(EOpIndexDirectStruct, yyvsp[-2].interm.intermTypedNode, index, yyvsp[-1].lex.line);                
                        yyval.interm.intermTypedNode->setType(*(*fields)[i].type);
                    }
                } else {
                    parseContext.error(yyvsp[-1].lex.line, " no such field in structure", yyvsp[0].lex.string->c_str(), "");
                    parseContext.recover();
                    yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
                }
            }
        } else {
            parseContext.error(yyvsp[-1].lex.line, " field selection requires structure, vector, or matrix on left hand side", yyvsp[0].lex.string->c_str(), "");
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
        // don't delete $3.string, it's from the pool
    ;}
    break;

  case 12:
#line 488 "glslang.y"
    {
        if (parseContext.lValueErrorCheck(yyvsp[0].lex.line, "++", yyvsp[-1].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermTypedNode = parseContext.intermediate.addUnaryMath(EOpPostIncrement, yyvsp[-1].interm.intermTypedNode, yyvsp[0].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.unaryOpError(yyvsp[0].lex.line, "++", yyvsp[-1].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-1].interm.intermTypedNode;
        }
    ;}
    break;

  case 13:
#line 498 "glslang.y"
    {
        if (parseContext.lValueErrorCheck(yyvsp[0].lex.line, "--", yyvsp[-1].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermTypedNode = parseContext.intermediate.addUnaryMath(EOpPostDecrement, yyvsp[-1].interm.intermTypedNode, yyvsp[0].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.unaryOpError(yyvsp[0].lex.line, "--", yyvsp[-1].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-1].interm.intermTypedNode;
        }
    ;}
    break;

  case 14:
#line 511 "glslang.y"
    {
        if (parseContext.integerErrorCheck(yyvsp[0].interm.intermTypedNode, "[]"))
            parseContext.recover();
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; 
    ;}
    break;

  case 15:
#line 519 "glslang.y"
    {
        TFunction* fnCall = yyvsp[0].interm.function;
        TOperator op = fnCall->getBuiltInOp();

        
        if (op == EOpArrayLength) {
            if (yyvsp[0].interm.intermNode->getAsTyped() == 0 || yyvsp[0].interm.intermNode->getAsTyped()->getType().getArraySize() == 0) {
                parseContext.error(yyvsp[0].interm.line, "", fnCall->getName().c_str(), "array must be declared with a size before using this method");
                parseContext.recover();
            }

            constUnion *unionArray = new constUnion[1];
            unionArray->setIConst(yyvsp[0].interm.intermNode->getAsTyped()->getType().getArraySize());
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EvqConst), yyvsp[0].interm.line);
        } else if (op != EOpNull) {
            //
            // Then this should be a constructor.
            // Don't go through the symbol table for constructors.  
            // Their parameters will be verified algorithmically.
            //
            TType type(EbtVoid);  // use this to get the type back
            if (parseContext.constructorErrorCheck(yyvsp[0].interm.line, yyvsp[0].interm.intermNode, *fnCall, op, &type)) {
                yyval.interm.intermTypedNode = 0;
            } else {
                //
                // It's a constructor, of type 'type'.
                //
                yyval.interm.intermTypedNode = parseContext.addConstructor(yyvsp[0].interm.intermNode, &type, op, fnCall, yyvsp[0].interm.line);
            }
            
            if (yyval.interm.intermTypedNode == 0) {        
                parseContext.recover();
                yyval.interm.intermTypedNode = parseContext.intermediate.setAggregateOperator(0, op, yyvsp[0].interm.line);
            }
            yyval.interm.intermTypedNode->setType(type);
        } else {
            //
            // Not a constructor.  Find it in the symbol table.
            //
            const TFunction* fnCandidate;
            bool builtIn;
            fnCandidate = parseContext.findFunction(yyvsp[0].interm.line, fnCall, &builtIn);

            if (fnCandidate) {
                //
                // A declared function.  But, it might still map to a built-in
                // operation.
                //

				// Added for ESSL support - checks if the function needs an extension
				if (parseContext.extensionizedFunctionErrorCheck(yyvsp[0].interm.line, fnCandidate->getMangledName() )){
					parseContext.recover();                
				}


                op = fnCandidate->getBuiltInOp();
                if (builtIn && op != EOpNull) {
                    //
                    // A function call mapped to a built-in operation.
                    //
                    if (fnCandidate->getParamCount() == 1) {
                        //
                        // Treat it like a built-in unary operator.
                        //
                        yyval.interm.intermTypedNode = parseContext.intermediate.addUnaryMath(op, yyvsp[0].interm.intermNode, 0, parseContext.symbolTable);
                        if (yyval.interm.intermTypedNode == 0)  {
                            parseContext.error(yyvsp[0].interm.intermNode->getLine(), " wrong operand type", "Internal Error", 
                                "built in unary operator function.  Type: %s",
                                static_cast<TIntermTyped*>(yyvsp[0].interm.intermNode)->getCompleteString().c_str());
                            YYERROR;
                        }
                    } else {
                        yyval.interm.intermTypedNode = parseContext.intermediate.setAggregateOperator(yyvsp[0].interm.intermAggregate, op, yyvsp[0].interm.line);
                    }
                } else {
                    // This is a real function call
                    
                    yyval.interm.intermTypedNode = parseContext.intermediate.setAggregateOperator(yyvsp[0].interm.intermAggregate, EOpFunctionCall, yyvsp[0].interm.line);
                    yyval.interm.intermTypedNode->setType(fnCandidate->getReturnType());                   
                    
                    // this is how we know whether the given function is a builtIn function or a user defined function
                    // if builtIn == false, it's a userDefined -> could be an overloaded builtIn function also
                    // if builtIn == true, it's definitely a builtIn function with EOpNull
                    if (!builtIn) 
                        yyval.interm.intermTypedNode->getAsAggregate()->setUserDefined(); 
                    yyval.interm.intermTypedNode->getAsAggregate()->setName(fnCandidate->getMangledName());

                    TQualifier qual;
                    TQualifierList& qualifierList = yyval.interm.intermTypedNode->getAsAggregate()->getQualifier();
                    for (int i = 0; i < fnCandidate->getParamCount(); ++i) {
                        qual = (*fnCandidate)[i].type->getQualifier();
                        if (qual == EvqOut || qual == EvqInOut) {
                            if (parseContext.lValueErrorCheck(yyval.interm.intermTypedNode->getLine(), "assign", yyval.interm.intermTypedNode->getAsAggregate()->getSequence()[i]->getAsTyped())) {
                                parseContext.error(yyvsp[0].interm.intermNode->getLine(), "Constant value cannot be passed for 'out' or 'inout' parameters.", "Error", "");
                                parseContext.recover();
                            }
                        }
                        qualifierList.push_back(qual);
                    }
                }
                yyval.interm.intermTypedNode->setType(fnCandidate->getReturnType());
            } else {
                // error message was put out by PaFindFunction()
                // Put on a dummy node for error recovery
                constUnion *unionArray = new constUnion[1];
                unionArray->setFConst(0.0f);
                yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtFloat, EvqConst), yyvsp[0].interm.line);
                parseContext.recover();
            }
        }
        delete fnCall;
    ;}
    break;

  case 16:
#line 634 "glslang.y"
    {
        yyval.interm = yyvsp[0].interm;
    ;}
    break;

  case 17:
#line 637 "glslang.y"
    {
        if (yyvsp[-2].interm.intermTypedNode->isArray() && yyvsp[0].interm.function->getName() == "length") {
            //
            // implement array.length()
            //
            if (parseContext.extensionErrorCheck(yyvsp[0].interm.line, "GL_3DL_array_objects")) {
                parseContext.recover();
                yyval.interm = yyvsp[0].interm;
            } else {
                yyval.interm = yyvsp[0].interm;
                yyval.interm.intermNode = yyvsp[-2].interm.intermTypedNode;
                yyval.interm.function->relateToOperator(EOpArrayLength);
            }
        } else {
            parseContext.error(yyvsp[0].interm.line, "methods are not supported", "", "");
            parseContext.recover();
            yyval.interm = yyvsp[0].interm;
        }
    ;}
    break;

  case 18:
#line 659 "glslang.y"
    {
        yyval.interm = yyvsp[-1].interm;
        yyval.interm.line = yyvsp[0].lex.line;
    ;}
    break;

  case 19:
#line 663 "glslang.y"
    {
        yyval.interm = yyvsp[-1].interm;
        yyval.interm.line = yyvsp[0].lex.line;
    ;}
    break;

  case 20:
#line 670 "glslang.y"
    {
        yyval.interm.function = yyvsp[-1].interm.function;
        yyval.interm.intermNode = 0;
    ;}
    break;

  case 21:
#line 674 "glslang.y"
    {
        yyval.interm.function = yyvsp[0].interm.function;
        yyval.interm.intermNode = 0;
    ;}
    break;

  case 22:
#line 681 "glslang.y"
    {
        TParameter param = { 0, new TType(yyvsp[0].interm.intermTypedNode->getType()) };
        yyvsp[-1].interm.function->addParameter(param);
        yyval.interm.function = yyvsp[-1].interm.function;
        yyval.interm.intermNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 23:
#line 687 "glslang.y"
    {
        TParameter param = { 0, new TType(yyvsp[0].interm.intermTypedNode->getType()) };
        yyvsp[-2].interm.function->addParameter(param);
        yyval.interm.function = yyvsp[-2].interm.function;
        yyval.interm.intermNode = parseContext.intermediate.growAggregate(yyvsp[-2].interm.intermNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line);
    ;}
    break;

  case 24:
#line 696 "glslang.y"
    {
        yyval.interm.function = yyvsp[-1].interm.function;
    ;}
    break;

  case 25:
#line 704 "glslang.y"
    {
        //
        // Constructor
        //
        if (yyvsp[0].interm.type.array) {
            if (parseContext.extensionErrorCheck(yyvsp[0].interm.type.line, "GL_3DL_array_objects")) {
                parseContext.recover();
                yyvsp[0].interm.type.setArray(false);
            }
        }

        if (yyvsp[0].interm.type.userDef) {
            TString tempString = "";
            TType type(yyvsp[0].interm.type);
            TFunction *function = new TFunction(&tempString, type, EOpConstructStruct);
            yyval.interm.function = function;
        } else {
            TOperator op = EOpNull;
            switch (yyvsp[0].interm.type.type) {
            case EbtFloat:
                if (yyvsp[0].interm.type.matrix) {
                    switch(yyvsp[0].interm.type.size) {
                    case 2:                                     op = EOpConstructMat2;  break;
                    case 3:                                     op = EOpConstructMat3;  break;
                    case 4:                                     op = EOpConstructMat4;  break;
                    }         
                } else {      
                    switch(yyvsp[0].interm.type.size) {
                    case 1:                                     op = EOpConstructFloat; break;
                    case 2:                                     op = EOpConstructVec2;  break;
                    case 3:                                     op = EOpConstructVec3;  break;
                    case 4:                                     op = EOpConstructVec4;  break;
                    }       
                }  
                break;               
            case EbtInt:
                switch(yyvsp[0].interm.type.size) {
                case 1:                                         op = EOpConstructInt;   break;
                case 2:       FRAG_VERT_ONLY("ivec2", yyvsp[0].interm.type.line); op = EOpConstructIVec2; break;
                case 3:       FRAG_VERT_ONLY("ivec3", yyvsp[0].interm.type.line); op = EOpConstructIVec3; break;
                case 4:       FRAG_VERT_ONLY("ivec4", yyvsp[0].interm.type.line); op = EOpConstructIVec4; break;
                }         
                break;    
            case EbtBool:
                switch(yyvsp[0].interm.type.size) {
                case 1:                                         op = EOpConstructBool;  break;
                case 2:       FRAG_VERT_ONLY("bvec2", yyvsp[0].interm.type.line); op = EOpConstructBVec2; break;
                case 3:       FRAG_VERT_ONLY("bvec3", yyvsp[0].interm.type.line); op = EOpConstructBVec3; break;
                case 4:       FRAG_VERT_ONLY("bvec4", yyvsp[0].interm.type.line); op = EOpConstructBVec4; break;
                }         
                break;
            }
            if (op == EOpNull) {                    
                parseContext.error(yyvsp[0].interm.type.line, "cannot construct this type", TType::getBasicString(yyvsp[0].interm.type.type), "");
                parseContext.recover();
                yyvsp[0].interm.type.type = EbtFloat;
                op = EOpConstructFloat;
            }            
            TString tempString = "";
            TType type(yyvsp[0].interm.type);
            TFunction *function = new TFunction(&tempString, type, op);
            yyval.interm.function = function;
        }
    ;}
    break;

  case 26:
#line 768 "glslang.y"
    {
        if (parseContext.reservedErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string)) 
            parseContext.recover();
		
        TType type(EbtVoid);
        TFunction *function = new TFunction(yyvsp[0].lex.string, type);
        yyval.interm.function = function;

    ;}
    break;

  case 27:
#line 777 "glslang.y"
    {
        if (parseContext.reservedErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string)) 
            parseContext.recover();
        TType type(EbtVoid);
        TFunction *function = new TFunction(yyvsp[0].lex.string, type);
        yyval.interm.function = function;
    ;}
    break;

  case 28:
#line 787 "glslang.y"
    {
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 29:
#line 790 "glslang.y"
    {
        if (parseContext.lValueErrorCheck(yyvsp[-1].lex.line, "++", yyvsp[0].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermTypedNode = parseContext.intermediate.addUnaryMath(EOpPreIncrement, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.unaryOpError(yyvsp[-1].lex.line, "++", yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
        }
    ;}
    break;

  case 30:
#line 800 "glslang.y"
    {
        if (parseContext.lValueErrorCheck(yyvsp[-1].lex.line, "--", yyvsp[0].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermTypedNode = parseContext.intermediate.addUnaryMath(EOpPreDecrement, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.unaryOpError(yyvsp[-1].lex.line, "--", yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
        }
    ;}
    break;

  case 31:
#line 810 "glslang.y"
    {
        if (yyvsp[-1].interm.op != EOpNull) {
            yyval.interm.intermTypedNode = parseContext.intermediate.addUnaryMath(yyvsp[-1].interm.op, yyvsp[0].interm.intermTypedNode, yyvsp[-1].interm.line, parseContext.symbolTable);
            if (yyval.interm.intermTypedNode == 0) {
                char* errorOp = "";
                switch(yyvsp[-1].interm.op) {
                case EOpNegative:   errorOp = "-"; break;
                case EOpLogicalNot: errorOp = "!"; break;
                case EOpBitwiseNot: errorOp = "~"; break;
				default: break;
                }
                parseContext.unaryOpError(yyvsp[-1].interm.line, errorOp, yyvsp[0].interm.intermTypedNode->getCompleteString());
                parseContext.recover();
                yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
            }
        } else
            yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 32:
#line 832 "glslang.y"
    { yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpNull; ;}
    break;

  case 33:
#line 833 "glslang.y"
    { yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpNegative; ;}
    break;

  case 34:
#line 834 "glslang.y"
    { yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpLogicalNot; ;}
    break;

  case 35:
#line 835 "glslang.y"
    { PACK_UNPACK_ONLY("~", yyvsp[0].lex.line);  
              yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpBitwiseNot; ;}
    break;

  case 36:
#line 841 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 37:
#line 842 "glslang.y"
    {
        FRAG_VERT_ONLY("*", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpMul, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "*", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 38:
#line 851 "glslang.y"
    {
        FRAG_VERT_ONLY("/", yyvsp[-1].lex.line); 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpDiv, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "/", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 39:
#line 860 "glslang.y"
    {
        PACK_UNPACK_ONLY("%", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpMod, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "%", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 40:
#line 872 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 41:
#line 873 "glslang.y"
    {  
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpAdd, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "+", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 42:
#line 881 "glslang.y"
    {
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpSub, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "-", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        } 
    ;}
    break;

  case 43:
#line 892 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 44:
#line 893 "glslang.y"
    {
        PACK_UNPACK_ONLY("<<", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpLeftShift, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "<<", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 45:
#line 902 "glslang.y"
    {
        PACK_UNPACK_ONLY(">>", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpRightShift, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, ">>", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 46:
#line 914 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 47:
#line 915 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpLessThan, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "<", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 48:
#line 925 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpGreaterThan, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, ">", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 49:
#line 935 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpLessThanEqual, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "<=", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 50:
#line 945 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpGreaterThanEqual, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, ">=", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 51:
#line 958 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 52:
#line 959 "glslang.y"
    {
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpEqual, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "==", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        } else if ((yyvsp[-2].interm.intermTypedNode->isArray() || yyvsp[0].interm.intermTypedNode->isArray()) && parseContext.extensionErrorCheck(yyvsp[-1].lex.line, "GL_3DL_array_objects"))
            parseContext.recover();
    ;}
    break;

  case 53:
#line 970 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpNotEqual, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "!=", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        } else if ((yyvsp[-2].interm.intermTypedNode->isArray() || yyvsp[0].interm.intermTypedNode->isArray()) && parseContext.extensionErrorCheck(yyvsp[-1].lex.line, "GL_3DL_array_objects"))
            parseContext.recover();
    ;}
    break;

  case 54:
#line 984 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 55:
#line 985 "glslang.y"
    {
        PACK_UNPACK_ONLY("&", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpAnd, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "&", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 56:
#line 997 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 57:
#line 998 "glslang.y"
    {
        PACK_UNPACK_ONLY("^", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpExclusiveOr, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "^", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 58:
#line 1010 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 59:
#line 1011 "glslang.y"
    {
        PACK_UNPACK_ONLY("|", yyvsp[-1].lex.line);
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpInclusiveOr, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "|", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        }
    ;}
    break;

  case 60:
#line 1023 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 61:
#line 1024 "glslang.y"
    {
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpLogicalAnd, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "&&", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 62:
#line 1037 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 63:
#line 1038 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpLogicalXor, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "^^", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 64:
#line 1051 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 65:
#line 1052 "glslang.y"
    { 
        yyval.interm.intermTypedNode = parseContext.intermediate.addBinaryMath(EOpLogicalOr, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line, parseContext.symbolTable);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, "||", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            yyval.interm.intermTypedNode = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EvqConst), yyvsp[-1].lex.line);
        }
    ;}
    break;

  case 66:
#line 1065 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 67:
#line 1066 "glslang.y"
    {
       if (parseContext.boolErrorCheck(yyvsp[-3].lex.line, yyvsp[-4].interm.intermTypedNode))
            parseContext.recover();
       
        yyval.interm.intermTypedNode = parseContext.intermediate.addSelection(yyvsp[-4].interm.intermTypedNode, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-3].lex.line);
        if (yyvsp[-2].interm.intermTypedNode->getType() != yyvsp[0].interm.intermTypedNode->getType())
            yyval.interm.intermTypedNode = 0;
            
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-3].lex.line, ":", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
        }
    ;}
    break;

  case 68:
#line 1083 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 69:
#line 1084 "glslang.y"
    {        
        if (parseContext.lValueErrorCheck(yyvsp[-1].interm.line, "assign", yyvsp[-2].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermTypedNode = parseContext.intermediate.addAssign(yyvsp[-1].interm.op, yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].interm.line);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.assignError(yyvsp[-1].interm.line, "assign", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[-2].interm.intermTypedNode;
        } else if ((yyvsp[-2].interm.intermTypedNode->isArray() || yyvsp[0].interm.intermTypedNode->isArray()) && parseContext.extensionErrorCheck(yyvsp[-1].interm.line, "GL_3DL_array_objects"))
            parseContext.recover();
    ;}
    break;

  case 70:
#line 1098 "glslang.y"
    {                                    yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpAssign; ;}
    break;

  case 71:
#line 1099 "glslang.y"
    { FRAG_VERT_ONLY("*=", yyvsp[0].lex.line);     yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpMulAssign; ;}
    break;

  case 72:
#line 1100 "glslang.y"
    { FRAG_VERT_ONLY("/=", yyvsp[0].lex.line);     yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpDivAssign; ;}
    break;

  case 73:
#line 1101 "glslang.y"
    { PACK_UNPACK_ONLY("%=", yyvsp[0].lex.line);   yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpModAssign; ;}
    break;

  case 74:
#line 1102 "glslang.y"
    {                                    yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpAddAssign; ;}
    break;

  case 75:
#line 1103 "glslang.y"
    {                                    yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpSubAssign; ;}
    break;

  case 76:
#line 1104 "glslang.y"
    { PACK_UNPACK_ONLY("<<=", yyvsp[0].lex.line);  yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpLeftShiftAssign; ;}
    break;

  case 77:
#line 1105 "glslang.y"
    { PACK_UNPACK_ONLY("<<=", yyvsp[0].lex.line);  yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpRightShiftAssign; ;}
    break;

  case 78:
#line 1106 "glslang.y"
    { PACK_UNPACK_ONLY("&=",  yyvsp[0].lex.line);  yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpAndAssign; ;}
    break;

  case 79:
#line 1107 "glslang.y"
    { PACK_UNPACK_ONLY("^=",  yyvsp[0].lex.line);  yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpExclusiveOrAssign; ;}
    break;

  case 80:
#line 1108 "glslang.y"
    { PACK_UNPACK_ONLY("|=",  yyvsp[0].lex.line);  yyval.interm.line = yyvsp[0].lex.line; yyval.interm.op = EOpInclusiveOrAssign; ;}
    break;

  case 81:
#line 1112 "glslang.y"
    {
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 82:
#line 1115 "glslang.y"
    {
        yyval.interm.intermTypedNode = parseContext.intermediate.addComma(yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.intermTypedNode, yyvsp[-1].lex.line);
        if (yyval.interm.intermTypedNode == 0) {
            parseContext.binaryOpError(yyvsp[-1].lex.line, ",", yyvsp[-2].interm.intermTypedNode->getCompleteString(), yyvsp[0].interm.intermTypedNode->getCompleteString());
            parseContext.recover();
            yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
        }
    ;}
    break;

  case 83:
#line 1126 "glslang.y"
    {
        if (parseContext.constErrorCheck(yyvsp[0].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 84:
#line 1134 "glslang.y"
    { yyval.interm.intermNode = 0; ;}
    break;

  case 85:
#line 1135 "glslang.y"
    { 
        if (yyvsp[-1].interm.intermAggregate)
            yyvsp[-1].interm.intermAggregate->setOperator(EOpSequence); 
        yyval.interm.intermNode = yyvsp[-1].interm.intermAggregate; 
    ;}
    break;

  case 86:
#line 1141 "glslang.y"
    {
		parseContext.symbolTable.setDefaultPrecision( yyvsp[-1].interm.type.type, yyvsp[-2].interm.precision );				// Added for ESSL support
		yyval.interm.intermNode = 0;

    ;}
    break;

  case 87:
#line 1149 "glslang.y"
    {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions 
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //        
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find(yyvsp[-1].interm.function->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != yyvsp[-1].interm.function->getReturnType()) {
                parseContext.error(yyvsp[0].lex.line, "overloaded functions must have the same return type", yyvsp[-1].interm.function->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*yyvsp[-1].interm.function)[i].type->getQualifier()) {
                    parseContext.error(yyvsp[0].lex.line, "overloaded functions must have the same parameter qualifiers", (*yyvsp[-1].interm.function)[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }
        
        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        yyval.interm.function = yyvsp[-1].interm.function;
        yyval.interm.line = yyvsp[0].lex.line;

        parseContext.symbolTable.insert(*yyval.interm.function);
    ;}
    break;

  case 88:
#line 1185 "glslang.y"
    {
        yyval.interm.function = yyvsp[0].interm.function;
    ;}
    break;

  case 89:
#line 1188 "glslang.y"
    { 
        yyval.interm.function = yyvsp[0].interm.function;  
    ;}
    break;

  case 90:
#line 1195 "glslang.y"
    {
        // Add the parameter 
        yyval.interm.function = yyvsp[-1].interm.function;
        if (yyvsp[0].interm.param.type->getBasicType() != EbtVoid)
            yyvsp[-1].interm.function->addParameter(yyvsp[0].interm.param);
        else
            delete yyvsp[0].interm.param.type;
    ;}
    break;

  case 91:
#line 1203 "glslang.y"
    {   
        //
        // Only first parameter of one-parameter functions can be void
        // The check for named parameters not being void is done in parameter_declarator 
        //
        if (yyvsp[0].interm.param.type->getBasicType() == EbtVoid) {
            //
            // This parameter > first is void
            //
            parseContext.error(yyvsp[-1].lex.line, "cannot be an argument type except for '(void)'", "void", "");
            parseContext.recover();
            delete yyvsp[0].interm.param.type;
        } else {
            // Add the parameter 
            yyval.interm.function = yyvsp[-2].interm.function; 
            yyvsp[-2].interm.function->addParameter(yyvsp[0].interm.param);
        }
    ;}
    break;

  case 92:
#line 1224 "glslang.y"
    {
        if (yyvsp[-2].interm.type.qualifier != EvqGlobal && yyvsp[-2].interm.type.qualifier != EvqTemporary) {
            parseContext.error(yyvsp[-1].lex.line, "no qualifiers allowed for function return", getQualifierString(yyvsp[-2].interm.type.qualifier), "");
            parseContext.recover();
        }
        // make sure a sampler is not involved as well...
        if (parseContext.structQualifierErrorCheck(yyvsp[-1].lex.line, yyvsp[-2].interm.type))
            parseContext.recover();
        
        // Add the function as a prototype after parsing it (we do not support recursion) 
        TFunction *function;
        TType type(yyvsp[-2].interm.type);
        function = new TFunction(yyvsp[-1].lex.string, type);
        yyval.interm.function = function;
    ;}
    break;

  case 93:
#line 1243 "glslang.y"
    {
        if (yyvsp[-1].interm.type.type == EbtVoid) {
            parseContext.error(yyvsp[0].lex.line, "illegal use of type 'void'", yyvsp[0].lex.string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string))
            parseContext.recover();
        TParameter param = {yyvsp[0].lex.string, new TType(yyvsp[-1].interm.type)};
        yyval.interm.line = yyvsp[0].lex.line;
        yyval.interm.param = param;
    ;}
    break;

  case 94:
#line 1254 "glslang.y"
    {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck(yyvsp[-2].lex.line, yyvsp[-4].interm.type))
            parseContext.recover();
            
        if (parseContext.reservedErrorCheck(yyvsp[-3].lex.line, *yyvsp[-3].lex.string))
            parseContext.recover();
            
        int size;
        if (parseContext.arraySizeErrorCheck(yyvsp[-2].lex.line, yyvsp[-1].interm.intermTypedNode, size))
            parseContext.recover();
        yyvsp[-4].interm.type.setArray(true, size);
             
        TType* type = new TType(yyvsp[-4].interm.type);
        TParameter param = { yyvsp[-3].lex.string, type };
        yyval.interm.line = yyvsp[-3].lex.line;
        yyval.interm.param = param;
    ;}
    break;

  case 95:
#line 1283 "glslang.y"
    {
        yyval.interm = yyvsp[0].interm;
        if (parseContext.paramErrorCheck(yyvsp[0].interm.line, yyvsp[-2].interm.type.qualifier, yyvsp[-1].interm.qualifier, yyval.interm.param.type))
            parseContext.recover();
    ;}
    break;

  case 96:
#line 1288 "glslang.y"
    {
        yyval.interm = yyvsp[0].interm;
        if (parseContext.parameterSamplerErrorCheck(yyvsp[0].interm.line, yyvsp[-1].interm.qualifier, *yyvsp[0].interm.param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck(yyvsp[0].interm.line, EvqTemporary, yyvsp[-1].interm.qualifier, yyval.interm.param.type))
            parseContext.recover();
    ;}
    break;

  case 97:
#line 1298 "glslang.y"
    {
        yyval.interm = yyvsp[0].interm;
        if (parseContext.paramErrorCheck(yyvsp[0].interm.line, yyvsp[-2].interm.type.qualifier, yyvsp[-1].interm.qualifier, yyval.interm.param.type))
            parseContext.recover();
    ;}
    break;

  case 98:
#line 1303 "glslang.y"
    {
        yyval.interm = yyvsp[0].interm;
        if (parseContext.parameterSamplerErrorCheck(yyvsp[0].interm.line, yyvsp[-1].interm.qualifier, *yyvsp[0].interm.param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck(yyvsp[0].interm.line, EvqTemporary, yyvsp[-1].interm.qualifier, yyval.interm.param.type))
            parseContext.recover();
    ;}
    break;

  case 99:
#line 1313 "glslang.y"
    {
        yyval.interm.qualifier = EvqIn;
    ;}
    break;

  case 100:
#line 1316 "glslang.y"
    {
        yyval.interm.qualifier = EvqIn;
    ;}
    break;

  case 101:
#line 1319 "glslang.y"
    {
        yyval.interm.qualifier = EvqOut;
    ;}
    break;

  case 102:
#line 1322 "glslang.y"
    {
        yyval.interm.qualifier = EvqInOut;
    ;}
    break;

  case 103:
#line 1328 "glslang.y"
    {
        TParameter param = { 0, new TType(yyvsp[0].interm.type) };
        yyval.interm.param = param;
    ;}
    break;

  case 104:
#line 1335 "glslang.y"
    {
        yyval.interm = yyvsp[0].interm;
    ;}
    break;

  case 105:
#line 1338 "glslang.y"
    {
        yyval.interm = yyvsp[-2].interm;

		bool finished = false;
		// Added for ESSL support
		if( ! yyvsp[-2].interm.type.validType ){
			if( parseContext.invariantDeclarationErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string ) ){
				parseContext.recover();
			}
			finished = true;
		}

        if ( !finished && parseContext.structQualifierErrorCheck(yyvsp[0].lex.line, yyval.interm.type))
            parseContext.recover();
        
        if (!finished &&parseContext.nonInitConstErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string, yyval.interm.type))
            parseContext.recover();

        if (!finished &&parseContext.nonInitErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string, yyval.interm.type))
            parseContext.recover();
    ;}
    break;

  case 106:
#line 1359 "glslang.y"
    {
        if (parseContext.structQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-4].interm.type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck(yyvsp[-2].lex.line, *yyvsp[-2].lex.string, yyvsp[-4].interm.type))
            parseContext.recover();

        yyval.interm = yyvsp[-4].interm;
        
        if (parseContext.arrayTypeErrorCheck(yyvsp[-1].lex.line, yyvsp[-4].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-1].lex.line, yyvsp[-4].interm.type))
            parseContext.recover();
        else {
            yyvsp[-4].interm.type.setArray(true);
            TVariable* variable;
            if (parseContext.arrayErrorCheck(yyvsp[-1].lex.line, *yyvsp[-2].lex.string, yyvsp[-4].interm.type, variable))
                parseContext.recover();
        }
    ;}
    break;

  case 107:
#line 1377 "glslang.y"
    {
        if (parseContext.structQualifierErrorCheck(yyvsp[-3].lex.line, yyvsp[-5].interm.type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck(yyvsp[-3].lex.line, *yyvsp[-3].lex.string, yyvsp[-5].interm.type))
            parseContext.recover();
        
        yyval.interm = yyvsp[-5].interm;

        if (parseContext.arrayTypeErrorCheck(yyvsp[-2].lex.line, yyvsp[-5].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-5].interm.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck(yyvsp[-2].lex.line, yyvsp[-1].interm.intermTypedNode, size))
                parseContext.recover();
            yyvsp[-5].interm.type.setArray(true, size);        
            TVariable* variable;
            if (parseContext.arrayErrorCheck(yyvsp[-2].lex.line, *yyvsp[-3].lex.string, yyvsp[-5].interm.type, variable))
                parseContext.recover();
        }
    ;}
    break;

  case 108:
#line 1398 "glslang.y"
    {
        if (parseContext.structQualifierErrorCheck(yyvsp[-4].lex.line, yyvsp[-6].interm.type))
            parseContext.recover();
            
        yyval.interm = yyvsp[-6].interm;
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck(yyvsp[-3].lex.line, yyvsp[-6].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-3].lex.line, yyvsp[-6].interm.type))
            parseContext.recover();
        else {
			yyvsp[-6].interm.type.setArray(true, yyvsp[0].interm.intermTypedNode->getType().getArraySize());
            if (parseContext.arrayErrorCheck(yyvsp[-3].lex.line, *yyvsp[-4].lex.string, yyvsp[-6].interm.type, variable))
                parseContext.recover();
        }

        if (parseContext.extensionErrorCheck(yyval.interm.line, "GL_3DL_array_objects"))
            parseContext.recover();
        else {
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer(yyvsp[-4].lex.line, *yyvsp[-4].lex.string, yyvsp[-6].interm.type, yyvsp[0].interm.intermTypedNode, intermNode, variable)) {
                //
                // build the intermediate representation
                //
                if (intermNode)
                    yyval.interm.intermAggregate = parseContext.intermediate.growAggregate(yyvsp[-6].interm.intermNode, intermNode, yyvsp[-1].lex.line);
                else
                    yyval.interm.intermAggregate = yyvsp[-6].interm.intermAggregate;
            } else {
                parseContext.recover();
                yyval.interm.intermAggregate = 0;
            }
        }
    ;}
    break;

  case 109:
#line 1431 "glslang.y"
    {
        if (parseContext.structQualifierErrorCheck(yyvsp[-5].lex.line, yyvsp[-7].interm.type))
            parseContext.recover();
            
        yyval.interm = yyvsp[-7].interm;
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck(yyvsp[-4].lex.line, yyvsp[-7].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-4].lex.line, yyvsp[-7].interm.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck(yyvsp[-4].lex.line, yyvsp[-3].interm.intermTypedNode, size))
                parseContext.recover();
            yyvsp[-7].interm.type.setArray(true, size);
            if (parseContext.arrayErrorCheck(yyvsp[-4].lex.line, *yyvsp[-5].lex.string, yyvsp[-7].interm.type, variable))
                parseContext.recover();
        }

        if (parseContext.extensionErrorCheck(yyval.interm.line, "GL_3DL_array_objects"))
            parseContext.recover();
        else {
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer(yyvsp[-5].lex.line, *yyvsp[-5].lex.string, yyvsp[-7].interm.type, yyvsp[0].interm.intermTypedNode, intermNode, variable)) {
                //
                // build the intermediate representation
                //
                if (intermNode)
                    yyval.interm.intermAggregate = parseContext.intermediate.growAggregate(yyvsp[-7].interm.intermNode, intermNode, yyvsp[-1].lex.line);
                else
                    yyval.interm.intermAggregate = yyvsp[-7].interm.intermAggregate;
            } else {
                parseContext.recover();
                yyval.interm.intermAggregate = 0;
            }
        }
    ;}
    break;

  case 110:
#line 1467 "glslang.y"
    {
        if (parseContext.structQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-4].interm.type))
            parseContext.recover();
        
        yyval.interm = yyvsp[-4].interm;
        
        TIntermNode* intermNode;
        if (!parseContext.executeInitializer(yyvsp[-2].lex.line, *yyvsp[-2].lex.string, yyvsp[-4].interm.type, yyvsp[0].interm.intermTypedNode, intermNode)) {
            //
            // build the intermediate representation
            //
            if (intermNode)
                yyval.interm.intermAggregate = parseContext.intermediate.growAggregate(yyvsp[-4].interm.intermNode, intermNode, yyvsp[-1].lex.line);
            else
                yyval.interm.intermAggregate = yyvsp[-4].interm.intermAggregate;
        } else {
            parseContext.recover();
            yyval.interm.intermAggregate = 0;
        }
    ;}
    break;

  case 111:
#line 1490 "glslang.y"
    {
        yyval.interm.type = yyvsp[0].interm.type;
        yyval.interm.intermAggregate = 0;
    ;}
    break;

  case 112:
#line 1494 "glslang.y"
    {
        yyval.interm.intermAggregate = 0;
        yyval.interm.type = yyvsp[-1].interm.type;

        if (parseContext.structQualifierErrorCheck(yyvsp[0].lex.line, yyval.interm.type))
            parseContext.recover();
        
        if (parseContext.nonInitConstErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string, yyval.interm.type))
            parseContext.recover();

        if (parseContext.nonInitErrorCheck(yyvsp[0].lex.line, *yyvsp[0].lex.string, yyval.interm.type))
            parseContext.recover();
		
    ;}
    break;

  case 113:
#line 1508 "glslang.y"
    {
        yyval.interm.intermAggregate = 0;
        if (parseContext.structQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-3].interm.type))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck(yyvsp[-2].lex.line, *yyvsp[-2].lex.string, yyvsp[-3].interm.type))
            parseContext.recover();

        yyval.interm.type = yyvsp[-3].interm.type;

        if (parseContext.arrayTypeErrorCheck(yyvsp[-1].lex.line, yyvsp[-3].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-1].lex.line, yyvsp[-3].interm.type))
            parseContext.recover();
        else {
            yyvsp[-3].interm.type.setArray(true);        
            TVariable* variable;
            if (parseContext.arrayErrorCheck(yyvsp[-1].lex.line, *yyvsp[-2].lex.string, yyvsp[-3].interm.type, variable))
                parseContext.recover();
        }
    ;}
    break;

  case 114:
#line 1527 "glslang.y"
    {
        yyval.interm.intermAggregate = 0;
        if (parseContext.structQualifierErrorCheck(yyvsp[-3].lex.line, yyvsp[-4].interm.type))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck(yyvsp[-3].lex.line, *yyvsp[-3].lex.string, yyvsp[-4].interm.type))
            parseContext.recover();
            
        yyval.interm.type = yyvsp[-4].interm.type;

        if (parseContext.arrayTypeErrorCheck(yyvsp[-2].lex.line, yyvsp[-4].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-4].interm.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck(yyvsp[-2].lex.line, yyvsp[-1].interm.intermTypedNode, size))
                parseContext.recover();
            
            yyvsp[-4].interm.type.setArray(true, size);
            TVariable* variable;
            if (parseContext.arrayErrorCheck(yyvsp[-2].lex.line, *yyvsp[-3].lex.string, yyvsp[-4].interm.type, variable))
                parseContext.recover();
        }
    ;}
    break;

  case 115:
#line 1550 "glslang.y"
    {
        yyval.interm.intermAggregate = 0;

        if (parseContext.structQualifierErrorCheck(yyvsp[-4].lex.line, yyvsp[-5].interm.type))
            parseContext.recover();

        yyval.interm.type = yyvsp[-5].interm.type;

        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck(yyvsp[-3].lex.line, yyvsp[-5].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-3].lex.line, yyvsp[-5].interm.type))
            parseContext.recover();
        else {
            yyvsp[-5].interm.type.setArray(true, yyvsp[0].interm.intermTypedNode->getType().getArraySize());
            if (parseContext.arrayErrorCheck(yyvsp[-3].lex.line, *yyvsp[-4].lex.string, yyvsp[-5].interm.type, variable))
                parseContext.recover();
        }

        if (parseContext.extensionErrorCheck(yyval.interm.line, "GL_3DL_array_objects"))
            parseContext.recover();
        else {        
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer(yyvsp[-4].lex.line, *yyvsp[-4].lex.string, yyvsp[-5].interm.type, yyvsp[0].interm.intermTypedNode, intermNode, variable)) {
                //
                // Build intermediate representation
                //
                if (intermNode)
                    yyval.interm.intermAggregate = parseContext.intermediate.makeAggregate(intermNode, yyvsp[-1].lex.line);
                else
                    yyval.interm.intermAggregate = 0;
            } else {
                parseContext.recover();
                yyval.interm.intermAggregate = 0;
            }
        }
    ;}
    break;

  case 116:
#line 1585 "glslang.y"
    {
        yyval.interm.intermAggregate = 0;

        if (parseContext.structQualifierErrorCheck(yyvsp[-5].lex.line, yyvsp[-6].interm.type))
            parseContext.recover();

        yyval.interm.type = yyvsp[-6].interm.type;

        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck(yyvsp[-4].lex.line, yyvsp[-6].interm.type) || parseContext.arrayQualifierErrorCheck(yyvsp[-4].lex.line, yyvsp[-6].interm.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck(yyvsp[-4].lex.line, yyvsp[-3].interm.intermTypedNode, size))
                parseContext.recover();
            
            yyvsp[-6].interm.type.setArray(true, size);
            if (parseContext.arrayErrorCheck(yyvsp[-4].lex.line, *yyvsp[-5].lex.string, yyvsp[-6].interm.type, variable))
                parseContext.recover();
        }

        if (parseContext.extensionErrorCheck(yyval.interm.line, "GL_3DL_array_objects"))
            parseContext.recover();
        else {        
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer(yyvsp[-5].lex.line, *yyvsp[-5].lex.string, yyvsp[-6].interm.type, yyvsp[0].interm.intermTypedNode, intermNode, variable)) {
                //
                // Build intermediate representation
                //
                if (intermNode)
                    yyval.interm.intermAggregate = parseContext.intermediate.makeAggregate(intermNode, yyvsp[-1].lex.line);
                else
                    yyval.interm.intermAggregate = 0;
            } else {
                parseContext.recover();
                yyval.interm.intermAggregate = 0;
            }
        }        
    ;}
    break;

  case 117:
#line 1624 "glslang.y"
    {
        if (parseContext.structQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-3].interm.type))
            parseContext.recover();

        yyval.interm.type = yyvsp[-3].interm.type;

        TIntermNode* intermNode;
        if (!parseContext.executeInitializer(yyvsp[-2].lex.line, *yyvsp[-2].lex.string, yyvsp[-3].interm.type, yyvsp[0].interm.intermTypedNode, intermNode)) {
            //
            // Build intermediate representation
            //
            if (intermNode)
                yyval.interm.intermAggregate = parseContext.intermediate.makeAggregate(intermNode, yyvsp[-1].lex.line);
            else
                yyval.interm.intermAggregate = 0;
        } else {
            parseContext.recover();
            yyval.interm.intermAggregate = 0;
        }
    ;}
    break;

  case 118:
#line 1644 "glslang.y"
    {			// Added for ESSL support
		// Test not performed:
		// 
		// Is the variable to declare invariant allready used ?
		//

		if( parseContext.invariantDeclarationErrorCheck(yyvsp[-1].lex.line, *yyvsp[0].lex.string ) ){
			parseContext.recover();
		}
		TPublicType type;
		type.setBasic(EbtVoid,EvqTemporary, yyvsp[-1].lex.line);
		type.setInvalid();									// INVARIANT is not a valid type. 
		yyval.interm.type = type;
		yyval.interm.intermAggregate = 0;
    ;}
    break;

  case 119:
#line 1730 "glslang.y"
    {
        yyval.interm.type = yyvsp[0].interm.type;
                
        if (yyvsp[0].interm.type.array) {
            if (parseContext.extensionErrorCheck(yyvsp[0].interm.type.line, "GL_3DL_array_objects")) {
                parseContext.recover();
                yyvsp[0].interm.type.setArray(false);
            }
        }
    ;}
    break;

  case 120:
#line 1740 "glslang.y"
    {        
        if (yyvsp[0].interm.type.array && parseContext.extensionErrorCheck(yyvsp[0].interm.type.line, "GL_3DL_array_objects")) {
            parseContext.recover();
            yyvsp[0].interm.type.setArray(false);
        }
        if (yyvsp[0].interm.type.array && parseContext.arrayQualifierErrorCheck(yyvsp[0].interm.type.line, yyvsp[-1].interm.type)) {
            parseContext.recover();
            yyvsp[0].interm.type.setArray(false);
        }
        
        if (yyvsp[-1].interm.type.qualifier == EvqAttribute &&
            (yyvsp[0].interm.type.type == EbtBool || yyvsp[0].interm.type.type == EbtInt)) {
            parseContext.error(yyvsp[0].interm.type.line, "cannot be bool or int", getQualifierString(yyvsp[-1].interm.type.qualifier), "");
            parseContext.recover();
        }
        if ((yyvsp[-1].interm.type.qualifier == EvqVaryingIn || yyvsp[-1].interm.type.qualifier == EvqVaryingOut) &&
            (yyvsp[0].interm.type.type == EbtBool || yyvsp[0].interm.type.type == EbtInt)) {
            parseContext.error(yyvsp[0].interm.type.line, "cannot be bool or int", getQualifierString(yyvsp[-1].interm.type.qualifier), "");
            parseContext.recover();
        }
        yyval.interm.type = yyvsp[0].interm.type; 
        yyval.interm.type.qualifier = yyvsp[-1].interm.type.qualifier;
        if ( yyvsp[-1].interm.type.invariant ) yyval.interm.type.makeInvariant();				// Added for ESSL support
        
    ;}
    break;

  case 121:
#line 1768 "glslang.y"
    {
        yyval.interm.type.setBasic(EbtVoid, EvqConst, yyvsp[0].lex.line);
    ;}
    break;

  case 122:
#line 1771 "glslang.y"
    { 
        VERTEX_ONLY("attribute", yyvsp[0].lex.line);
        if (parseContext.globalErrorCheck(yyvsp[0].lex.line, parseContext.symbolTable.atGlobalLevel(), "attribute"))
            parseContext.recover();
        yyval.interm.type.setBasic(EbtVoid, EvqAttribute, yyvsp[0].lex.line);
    ;}
    break;

  case 123:
#line 1777 "glslang.y"
    {
        if (parseContext.globalErrorCheck(yyvsp[0].lex.line, parseContext.symbolTable.atGlobalLevel(), "varying"))
            parseContext.recover();
        if (parseContext.language == EShLangVertex)
            yyval.interm.type.setBasic(EbtVoid, EvqVaryingOut, yyvsp[0].lex.line);
        else
            yyval.interm.type.setBasic(EbtVoid, EvqVaryingIn, yyvsp[0].lex.line);
    ;}
    break;

  case 124:
#line 1785 "glslang.y"
    {
        VERTEX_ONLY("invariant varying", yyvsp[-1].lex.line);
        if (parseContext.globalErrorCheck(yyvsp[-1].lex.line, parseContext.symbolTable.atGlobalLevel(), "varying"))
            parseContext.recover();

        yyval.interm.type.setBasic(EbtVoid, EvqVaryingOut, yyvsp[-1].lex.line);
        yyval.interm.type.makeInvariant();
		
    ;}
    break;

  case 125:
#line 1794 "glslang.y"
    {
        if (parseContext.globalErrorCheck(yyvsp[0].lex.line, parseContext.symbolTable.atGlobalLevel(), "uniform"))
            parseContext.recover();
        yyval.interm.type.setBasic(EbtVoid, EvqUniform, yyvsp[0].lex.line);
    ;}
    break;

  case 126:
#line 1802 "glslang.y"
    {
        yyval.interm.type = yyvsp[0].interm.type;
    ;}
    break;

  case 127:
#line 1805 "glslang.y"
    {
        yyval.interm.type = yyvsp[-3].interm.type;
        
        if (parseContext.arrayTypeErrorCheck(yyvsp[-2].lex.line, yyvsp[-3].interm.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck(yyvsp[-2].lex.line, yyvsp[-1].interm.intermTypedNode, size))
                parseContext.recover();
            yyval.interm.type.setArray(true, size);
        }
    ;}
    break;

  case 128:
#line 1821 "glslang.y"
    {
		TQualifier prec  = parseContext.symbolTable.getDefaultPrecision( yyvsp[0].interm.type.type );
		if( !parseContext.symbolTable.atBuiltInLevel() ){
			if( parseContext.validPrecisionErrorCheck(yyvsp[0].interm.type.line, prec, yyvsp[0].interm.type.type) ){
				parseContext.recover();
			}
		}		
		yyval.interm.type = yyvsp[0].interm.type;
		yyval.interm.type.precision = prec;
	;}
    break;

  case 129:
#line 1831 "glslang.y"
    {
		yyval.interm.type = yyvsp[0].interm.type;
		yyval.interm.type.precision = yyvsp[-1].interm.precision;
	;}
    break;

  case 130:
#line 1839 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtVoid, qual, yyvsp[0].lex.line); 
    ;}
    break;

  case 131:
#line 1843 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 132:
#line 1847 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtInt, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 133:
#line 1851 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtBool, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 134:
#line 1860 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(2);
    ;}
    break;

  case 135:
#line 1865 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(3);
    ;}
    break;

  case 136:
#line 1870 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(4);
    ;}
    break;

  case 137:
#line 1875 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtBool, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(2);
    ;}
    break;

  case 138:
#line 1880 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtBool, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(3);
    ;}
    break;

  case 139:
#line 1885 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtBool, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(4);
    ;}
    break;

  case 140:
#line 1890 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtInt, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(2);
    ;}
    break;

  case 141:
#line 1895 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtInt, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(3);
    ;}
    break;

  case 142:
#line 1900 "glslang.y"
    {
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtInt, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(4);
    ;}
    break;

  case 143:
#line 1905 "glslang.y"
    {
        FRAG_VERT_ONLY("mat2", yyvsp[0].lex.line); 
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(2, true);
    ;}
    break;

  case 144:
#line 1911 "glslang.y"
    { 
        FRAG_VERT_ONLY("mat3", yyvsp[0].lex.line); 
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(3, true);
    ;}
    break;

  case 145:
#line 1917 "glslang.y"
    { 
        FRAG_VERT_ONLY("mat4", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtFloat, qual, yyvsp[0].lex.line);
        yyval.interm.type.setAggregate(4, true);
    ;}
    break;

  case 146:
#line 1923 "glslang.y"
    {
        FRAG_VERT_ONLY("sampler1D", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSampler1D, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 147:
#line 1928 "glslang.y"
    {
        FRAG_VERT_ONLY("sampler2D", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSampler2D, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 148:
#line 1933 "glslang.y"
    {
        FRAG_VERT_ONLY("sampler3D", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSampler3D, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 149:
#line 1938 "glslang.y"
    {
        FRAG_VERT_ONLY("samplerCube", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSamplerCube, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 150:
#line 1943 "glslang.y"
    {
        FRAG_VERT_ONLY("sampler1DShadow", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSampler1DShadow, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 151:
#line 1948 "glslang.y"
    {
        FRAG_VERT_ONLY("sampler2DShadow", yyvsp[0].lex.line);
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSampler2DShadow, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 152:
#line 1953 "glslang.y"
    {
        // ARB_texture_rectangle

        FRAG_VERT_ONLY("samplerRectARB", yyvsp[0].lex.line);
        if (parseContext.extensionErrorCheck(yyvsp[0].lex.line, "GL_ARB_texture_rectangle"))
            parseContext.recover();
        
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSamplerRect, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 153:
#line 1963 "glslang.y"
    {
        // ARB_texture_rectangle

        FRAG_VERT_ONLY("samplerRectShadowARB", yyvsp[0].lex.line);
        if (parseContext.extensionErrorCheck(yyvsp[0].lex.line, "GL_ARB_texture_rectangle"))
            parseContext.recover();

        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtSamplerRectShadow, qual, yyvsp[0].lex.line);
    ;}
    break;

  case 154:
#line 1973 "glslang.y"
    {
        FRAG_VERT_ONLY("struct", yyvsp[0].interm.type.line);
        yyval.interm.type = yyvsp[0].interm.type;
        yyval.interm.type.qualifier = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
    ;}
    break;

  case 155:
#line 1978 "glslang.y"
    {     
        //
        // This is for user defined type names.  The lexical phase looked up the 
        // type.
        //
        TType& structure = static_cast<TVariable*>(yyvsp[0].lex.symbol)->getType();
        TQualifier qual = parseContext.symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary;
        yyval.interm.type.setBasic(EbtStruct, qual, yyvsp[0].lex.line);
        yyval.interm.type.userDef = &structure;
    ;}
    break;

  case 156:
#line 1993 "glslang.y"
    {
		yyval.interm.precision = EvqHighp;
	;}
    break;

  case 157:
#line 1996 "glslang.y"
    {
		yyval.interm.precision = EvqMediump;
	;}
    break;

  case 158:
#line 1999 "glslang.y"
    {
		yyval.interm.precision = EvqLowp;
	;}
    break;

  case 159:
#line 2006 "glslang.y"
    {
        TType* structure = new TType(yyvsp[-1].interm.typeList, *yyvsp[-3].lex.string);
        TVariable* userTypeDef = new TVariable(yyvsp[-3].lex.string, *structure, true);
        if (! parseContext.symbolTable.insert(*userTypeDef)) {
            parseContext.error(yyvsp[-3].lex.line, "redefinition", yyvsp[-3].lex.string->c_str(), "struct");
            parseContext.recover();
        }
        yyval.interm.type.setBasic(EbtStruct, EvqTemporary, yyvsp[-4].lex.line);
        yyval.interm.type.userDef = structure;
    ;}
    break;

  case 160:
#line 2016 "glslang.y"
    {
        TType* structure = new TType(yyvsp[-1].interm.typeList, TString(""));
        yyval.interm.type.setBasic(EbtStruct, EvqTemporary, yyvsp[-3].lex.line);
        yyval.interm.type.userDef = structure;
    ;}
    break;

  case 161:
#line 2024 "glslang.y"
    {
        yyval.interm.typeList = yyvsp[0].interm.typeList;
    ;}
    break;

  case 162:
#line 2027 "glslang.y"
    {
        yyval.interm.typeList = yyvsp[-1].interm.typeList;
        for (unsigned int i = 0; i < yyvsp[0].interm.typeList->size(); ++i) {
            for (unsigned int j = 0; j < yyval.interm.typeList->size(); ++j) {
                if ((*yyval.interm.typeList)[j].type->getFieldName() == (*yyvsp[0].interm.typeList)[i].type->getFieldName()) {
                    parseContext.error((*yyvsp[0].interm.typeList)[i].line, "duplicate field name in structure:", "struct", (*yyvsp[0].interm.typeList)[i].type->getFieldName().c_str());
                    parseContext.recover();
                }
            }
            yyval.interm.typeList->push_back((*yyvsp[0].interm.typeList)[i]);
        }
    ;}
    break;

  case 163:
#line 2042 "glslang.y"
    {
        yyval.interm.typeList = yyvsp[-1].interm.typeList;
        
        if (parseContext.voidErrorCheck(yyvsp[-2].interm.type.line, (*yyvsp[-1].interm.typeList)[0].type->getFieldName(), yyvsp[-2].interm.type)) {
            parseContext.recover();
        }
        for (unsigned int i = 0; i < yyval.interm.typeList->size(); ++i) {
            //
            // Careful not to replace already know aspects of type, like array-ness
            //
            (*yyval.interm.typeList)[i].type->setType(yyvsp[-2].interm.type.type, yyvsp[-2].interm.type.size, yyvsp[-2].interm.type.matrix, yyvsp[-2].interm.type.userDef);

            // don't allow arrays of arrays
            if ((*yyval.interm.typeList)[i].type->isArray()) {
                if (parseContext.arrayTypeErrorCheck(yyvsp[-2].interm.type.line, yyvsp[-2].interm.type))
                    parseContext.recover();
            }
            if (yyvsp[-2].interm.type.array)
                (*yyval.interm.typeList)[i].type->setArraySize(yyvsp[-2].interm.type.arraySize);
            if (yyvsp[-2].interm.type.userDef)
                (*yyval.interm.typeList)[i].type->setTypeName(yyvsp[-2].interm.type.userDef->getTypeName());
        }
    ;}
    break;

  case 164:
#line 2068 "glslang.y"
    {
        yyval.interm.typeList = NewPoolTTypeList();
        yyval.interm.typeList->push_back(yyvsp[0].interm.typeLine);
    ;}
    break;

  case 165:
#line 2072 "glslang.y"
    {
        yyval.interm.typeList->push_back(yyvsp[0].interm.typeLine);
    ;}
    break;

  case 166:
#line 2078 "glslang.y"
    {
        yyval.interm.typeLine.type = new TType(EbtVoid);
        yyval.interm.typeLine.line = yyvsp[0].lex.line;
        yyval.interm.typeLine.type->setFieldName(*yyvsp[0].lex.string);
    ;}
    break;

  case 167:
#line 2083 "glslang.y"
    {
        yyval.interm.typeLine.type = new TType(EbtVoid);
        yyval.interm.typeLine.line = yyvsp[-3].lex.line;
        yyval.interm.typeLine.type->setFieldName(*yyvsp[-3].lex.string);
        
        int size;
        if (parseContext.arraySizeErrorCheck(yyvsp[-2].lex.line, yyvsp[-1].interm.intermTypedNode, size))
            parseContext.recover();
        yyval.interm.typeLine.type->setArraySize(size);
    ;}
    break;

  case 168:
#line 2096 "glslang.y"
    { yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; ;}
    break;

  case 169:
#line 2100 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 170:
#line 2104 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermAggregate; ;}
    break;

  case 171:
#line 2105 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 172:
#line 2111 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 173:
#line 2112 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 174:
#line 2113 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 175:
#line 2114 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 176:
#line 2115 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 177:
#line 2119 "glslang.y"
    { yyval.interm.intermAggregate = 0; ;}
    break;

  case 178:
#line 2120 "glslang.y"
    { parseContext.symbolTable.push(); ;}
    break;

  case 179:
#line 2120 "glslang.y"
    { parseContext.symbolTable.pop(); ;}
    break;

  case 180:
#line 2120 "glslang.y"
    {
        if (yyvsp[-2].interm.intermAggregate != 0)            
            yyvsp[-2].interm.intermAggregate->setOperator(EOpSequence); 
        yyval.interm.intermAggregate = yyvsp[-2].interm.intermAggregate;
    ;}
    break;

  case 181:
#line 2128 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 182:
#line 2129 "glslang.y"
    { yyval.interm.intermNode = yyvsp[0].interm.intermNode; ;}
    break;

  case 183:
#line 2134 "glslang.y"
    { 
        yyval.interm.intermNode = 0; 
    ;}
    break;

  case 184:
#line 2137 "glslang.y"
    { 
        if (yyvsp[-1].interm.intermAggregate)
            yyvsp[-1].interm.intermAggregate->setOperator(EOpSequence); 
        yyval.interm.intermNode = yyvsp[-1].interm.intermAggregate; 
    ;}
    break;

  case 185:
#line 2145 "glslang.y"
    {
        yyval.interm.intermAggregate = parseContext.intermediate.makeAggregate(yyvsp[0].interm.intermNode, 0); 
    ;}
    break;

  case 186:
#line 2148 "glslang.y"
    { 
        yyval.interm.intermAggregate = parseContext.intermediate.growAggregate(yyvsp[-1].interm.intermAggregate, yyvsp[0].interm.intermNode, 0);
    ;}
    break;

  case 187:
#line 2154 "glslang.y"
    { yyval.interm.intermNode = 0; ;}
    break;

  case 188:
#line 2155 "glslang.y"
    { yyval.interm.intermNode = static_cast<TIntermNode*>(yyvsp[-1].interm.intermTypedNode); ;}
    break;

  case 189:
#line 2159 "glslang.y"
    { 
        if (parseContext.boolErrorCheck(yyvsp[-4].lex.line, yyvsp[-2].interm.intermTypedNode))
            parseContext.recover();
        yyval.interm.intermNode = parseContext.intermediate.addSelection(yyvsp[-2].interm.intermTypedNode, yyvsp[0].interm.nodePair, yyvsp[-4].lex.line);
    ;}
    break;

  case 190:
#line 2167 "glslang.y"
    {
        yyval.interm.nodePair.node1 = yyvsp[-2].interm.intermNode;
        yyval.interm.nodePair.node2 = yyvsp[0].interm.intermNode;
    ;}
    break;

  case 191:
#line 2171 "glslang.y"
    { 
        yyval.interm.nodePair.node1 = yyvsp[0].interm.intermNode;
        yyval.interm.nodePair.node2 = 0;
    ;}
    break;

  case 192:
#line 2181 "glslang.y"
    {
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
        if (parseContext.boolErrorCheck(yyvsp[0].interm.intermTypedNode->getLine(), yyvsp[0].interm.intermTypedNode))
            parseContext.recover();          
    ;}
    break;

  case 193:
#line 2186 "glslang.y"
    {
        TIntermNode* intermNode;
        if (parseContext.structQualifierErrorCheck(yyvsp[-2].lex.line, yyvsp[-3].interm.type))
            parseContext.recover();
        if (parseContext.boolErrorCheck(yyvsp[-2].lex.line, yyvsp[-3].interm.type))
            parseContext.recover();
        
        if (!parseContext.executeInitializer(yyvsp[-2].lex.line, *yyvsp[-2].lex.string, yyvsp[-3].interm.type, yyvsp[0].interm.intermTypedNode, intermNode))
            yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode;
        else {
            parseContext.recover();
            yyval.interm.intermTypedNode = 0;
        }
    ;}
    break;

  case 194:
#line 2203 "glslang.y"
    { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; ;}
    break;

  case 195:
#line 2203 "glslang.y"
    { 
        parseContext.symbolTable.pop();
        yyval.interm.intermNode = parseContext.intermediate.addLoop(yyvsp[0].interm.intermNode, yyvsp[-2].interm.intermTypedNode, 0, true, yyvsp[-5].lex.line);
        --parseContext.loopNestingLevel;
    ;}
    break;

  case 196:
#line 2208 "glslang.y"
    { ++parseContext.loopNestingLevel; ;}
    break;

  case 197:
#line 2208 "glslang.y"
    {
        if (parseContext.boolErrorCheck(yyvsp[0].lex.line, yyvsp[-2].interm.intermTypedNode))
            parseContext.recover();
                    
        yyval.interm.intermNode = parseContext.intermediate.addLoop(yyvsp[-5].interm.intermNode, yyvsp[-2].interm.intermTypedNode, 0, false, yyvsp[-4].lex.line);
        --parseContext.loopNestingLevel;
    ;}
    break;

  case 198:
#line 2215 "glslang.y"
    { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; ;}
    break;

  case 199:
#line 2215 "glslang.y"
    {
        parseContext.symbolTable.pop();
        yyval.interm.intermNode = parseContext.intermediate.makeAggregate(yyvsp[-3].interm.intermNode, yyvsp[-5].lex.line);
        yyval.interm.intermNode = parseContext.intermediate.growAggregate(
                yyval.interm.intermNode,
                parseContext.intermediate.addLoop(yyvsp[0].interm.intermNode, reinterpret_cast<TIntermTyped*>(yyvsp[-2].interm.nodePair.node1), reinterpret_cast<TIntermTyped*>(yyvsp[-2].interm.nodePair.node2), true, yyvsp[-6].lex.line),
                yyvsp[-6].lex.line);
        yyval.interm.intermNode->getAsAggregate()->setOperator(EOpSequence);
        --parseContext.loopNestingLevel;
    ;}
    break;

  case 200:
#line 2228 "glslang.y"
    {
        yyval.interm.intermNode = yyvsp[0].interm.intermNode; 
    ;}
    break;

  case 201:
#line 2231 "glslang.y"
    {
        yyval.interm.intermNode = yyvsp[0].interm.intermNode;
    ;}
    break;

  case 202:
#line 2237 "glslang.y"
    { 
        yyval.interm.intermTypedNode = yyvsp[0].interm.intermTypedNode; 
    ;}
    break;

  case 203:
#line 2240 "glslang.y"
    { 
        yyval.interm.intermTypedNode = 0; 
    ;}
    break;

  case 204:
#line 2246 "glslang.y"
    { 
        yyval.interm.nodePair.node1 = yyvsp[-1].interm.intermTypedNode;
        yyval.interm.nodePair.node2 = 0;
    ;}
    break;

  case 205:
#line 2250 "glslang.y"
    {
        yyval.interm.nodePair.node1 = yyvsp[-2].interm.intermTypedNode;
        yyval.interm.nodePair.node2 = yyvsp[0].interm.intermTypedNode;
    ;}
    break;

  case 206:
#line 2257 "glslang.y"
    {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error(yyvsp[-1].lex.line, "continue statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        yyval.interm.intermNode = parseContext.intermediate.addBranch(EOpContinue, yyvsp[-1].lex.line);
    ;}
    break;

  case 207:
#line 2264 "glslang.y"
    {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error(yyvsp[-1].lex.line, "break statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        yyval.interm.intermNode = parseContext.intermediate.addBranch(EOpBreak, yyvsp[-1].lex.line);
    ;}
    break;

  case 208:
#line 2271 "glslang.y"
    {
        yyval.interm.intermNode = parseContext.intermediate.addBranch(EOpReturn, yyvsp[-1].lex.line);
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid) {
            parseContext.error(yyvsp[-1].lex.line, "non-void function must return a value", "return", "");
            parseContext.recover();
        }
    ;}
    break;

  case 209:
#line 2278 "glslang.y"
    {        
        yyval.interm.intermNode = parseContext.intermediate.addBranch(EOpReturn, yyvsp[-1].interm.intermTypedNode, yyvsp[-2].lex.line);
        parseContext.functionReturnsValue = true;
        if (parseContext.currentFunctionType->getBasicType() == EbtVoid) {
            parseContext.error(yyvsp[-2].lex.line, "void function cannot return a value", "return", "");
            parseContext.recover();
        } else if (*(parseContext.currentFunctionType) != yyvsp[-1].interm.intermTypedNode->getType()) {
            parseContext.error(yyvsp[-2].lex.line, "function return is not matching type:", "return", "");
            parseContext.recover();
        }
    ;}
    break;

  case 210:
#line 2289 "glslang.y"
    {
        FRAG_ONLY("discard", yyvsp[-1].lex.line);
        yyval.interm.intermNode = parseContext.intermediate.addBranch(EOpKill, yyvsp[-1].lex.line);
    ;}
    break;

  case 211:
#line 2298 "glslang.y"
    { 
        yyval.interm.intermNode = yyvsp[0].interm.intermNode; 
        parseContext.treeRoot = yyval.interm.intermNode; 
    ;}
    break;

  case 212:
#line 2302 "glslang.y"
    {
        yyval.interm.intermNode = parseContext.intermediate.growAggregate(yyvsp[-1].interm.intermNode, yyvsp[0].interm.intermNode, 0);
        parseContext.treeRoot = yyval.interm.intermNode;
    ;}
    break;

  case 213:
#line 2309 "glslang.y"
    { 
        yyval.interm.intermNode = yyvsp[0].interm.intermNode; 
    ;}
    break;

  case 214:
#line 2312 "glslang.y"
    {
        yyval.interm.intermNode = yyvsp[0].interm.intermNode; 
    ;}
    break;

  case 215:
#line 2318 "glslang.y"
    {
        TFunction& function = *(yyvsp[0].interm.function);
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find(function.getMangledName()));
        //
        // Note:  'prevDec' could be 'function' if this is the first time we've seen function
        // as it would have just been put in the symbol table.  Otherwise, we're looking up
        // an earlier occurance.
        //
        if (prevDec->isDefined()) {
            //
            // Then this function already has a body.
            //
            parseContext.error(yyvsp[0].interm.line, "function already has a body", function.getName().c_str(), "");
            parseContext.recover();
        }
        prevDec->setDefined();
        
        //
        // Raise error message if main function takes any parameters or return anything other than void
        //
        if (function.getName() == "main") {
            if (function.getParamCount() > 0) {
                parseContext.error(yyvsp[0].interm.line, "function cannot take any parameter(s)", function.getName().c_str(), "");
                parseContext.recover();
            }
            if (function.getReturnType().getBasicType() != EbtVoid) {
                parseContext.error(yyvsp[0].interm.line, "", function.getReturnType().getBasicString(), "main function cannot return a value");
                parseContext.recover();
            }            
        }
   
        //
        // New symbol table scope for body of function plus its arguments
        //
        parseContext.symbolTable.push();
        
        //
        // Remember the return type for later checking for RETURN statements.
        //
        parseContext.currentFunctionType = &(prevDec->getReturnType());
        parseContext.functionReturnsValue = false;
        
        // 
        // Insert parameters into the symbol table.
        // If the parameter has no name, it's not an error, just don't insert it 
        // (could be used for unused args).
        //
        // Also, accumulate the list of parameters into the HIL, so lower level code
        // knows where to find parameters.
        //
        TIntermAggregate* paramNodes = new TIntermAggregate;
        for (int i = 0; i < function.getParamCount(); i++) {
            TParameter& param = function[i];
            if (param.name != 0) {
                TVariable *variable = new TVariable(param.name, *param.type);
                // 
                // Insert the parameters with name in the symbol table.
                //
                if (! parseContext.symbolTable.insert(*variable)) {
                    parseContext.error(yyvsp[0].interm.line, "redefinition", variable->getName().c_str(), "");
                    parseContext.recover();
                    delete variable;
                }
                //
                // Transfer ownership of name pointer to symbol table.
                //
                param.name = 0;
                
                //
                // Add the parameter to the HIL
                //                
                paramNodes = parseContext.intermediate.growAggregate(
                                               paramNodes, 
                                               parseContext.intermediate.addSymbol(variable->getUniqueId(),
                                                                       variable->getName(),
                                                                       variable->getType(), yyvsp[0].interm.line), 
                                               yyvsp[0].interm.line);
            } else {
                paramNodes = parseContext.intermediate.growAggregate(paramNodes, parseContext.intermediate.addSymbol(0, "", *param.type, yyvsp[0].interm.line), yyvsp[0].interm.line);
            }
        }
        parseContext.intermediate.setAggregateOperator(paramNodes, EOpParameters, yyvsp[0].interm.line);
        yyvsp[0].interm.intermAggregate = paramNodes;
        parseContext.loopNestingLevel = 0;
    ;}
    break;

  case 216:
#line 2403 "glslang.y"
    {
        //?? Check that all paths return a value if return type != void ?
        //   May be best done as post process phase on intermediate code
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid && ! parseContext.functionReturnsValue) {
            parseContext.error(yyvsp[-2].interm.line, "function does not return a value:", "", yyvsp[-2].interm.function->getName().c_str());
            parseContext.recover();
        }
        parseContext.symbolTable.pop();
        yyval.interm.intermNode = parseContext.intermediate.growAggregate(yyvsp[-2].interm.intermAggregate, yyvsp[0].interm.intermNode, 0);
        parseContext.intermediate.setAggregateOperator(yyval.interm.intermNode, EOpFunction, yyvsp[-2].interm.line);
        yyval.interm.intermNode->getAsAggregate()->setName(yyvsp[-2].interm.function->getMangledName().c_str());
        yyval.interm.intermNode->getAsAggregate()->setType(yyvsp[-2].interm.function->getReturnType());
        
        // store the pragma information for debug and optimize and other vendor specific 
        // information. This information can be queried from the parse tree
        yyval.interm.intermNode->getAsAggregate()->setOptimize(parseContext.contextPragma.optimize);
        yyval.interm.intermNode->getAsAggregate()->setDebug(parseContext.contextPragma.debug);
        yyval.interm.intermNode->getAsAggregate()->addToPragmaTable(parseContext.contextPragma.pragmaTable);
    ;}
    break;


    }

/* Line 1010 of yacc.c.  */
#line 4626 "glslang.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 2424 "glslang.y"


