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
/* Line 1285 of yacc.c.  */
#line 267 "glslang.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





