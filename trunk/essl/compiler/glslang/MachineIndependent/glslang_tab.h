typedef union {
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
#define	ATTRIBUTE	258
#define	CONST_QUAL	259
#define	BOOL_TYPE	260
#define	FLOAT_TYPE	261
#define	INT_TYPE	262
#define	BREAK	263
#define	CONTINUE	264
#define	DO	265
#define	ELSE	266
#define	FOR	267
#define	IF	268
#define	DISCARD	269
#define	RETURN	270
#define	BVEC2	271
#define	BVEC3	272
#define	BVEC4	273
#define	IVEC2	274
#define	IVEC3	275
#define	IVEC4	276
#define	VEC2	277
#define	VEC3	278
#define	VEC4	279
#define	MATRIX2	280
#define	MATRIX3	281
#define	MATRIX4	282
#define	IN_QUAL	283
#define	OUT_QUAL	284
#define	INOUT_QUAL	285
#define	UNIFORM	286
#define	VARYING	287
#define	STRUCT	288
#define	VOID_TYPE	289
#define	WHILE	290
#define	SAMPLER1D	291
#define	SAMPLER2D	292
#define	SAMPLER3D	293
#define	SAMPLERCUBE	294
#define	SAMPLER1DSHADOW	295
#define	SAMPLER2DSHADOW	296
#define	SAMPLERRECTARB	297
#define	SAMPLERRECTSHADOWARB	298
#define	LOW_PRECISION	299
#define	MEDIUM_PRECISION	300
#define	HIGH_PRECISION	301
#define	INVARIANT	302
#define	PRECISION	303
#define	IDENTIFIER	304
#define	TYPE_NAME	305
#define	FLOATCONSTANT	306
#define	INTCONSTANT	307
#define	BOOLCONSTANT	308
#define	FIELD_SELECTION	309
#define	LEFT_OP	310
#define	RIGHT_OP	311
#define	INC_OP	312
#define	DEC_OP	313
#define	LE_OP	314
#define	GE_OP	315
#define	EQ_OP	316
#define	NE_OP	317
#define	AND_OP	318
#define	OR_OP	319
#define	XOR_OP	320
#define	MUL_ASSIGN	321
#define	DIV_ASSIGN	322
#define	ADD_ASSIGN	323
#define	MOD_ASSIGN	324
#define	LEFT_ASSIGN	325
#define	RIGHT_ASSIGN	326
#define	AND_ASSIGN	327
#define	XOR_ASSIGN	328
#define	OR_ASSIGN	329
#define	SUB_ASSIGN	330
#define	LEFT_PAREN	331
#define	RIGHT_PAREN	332
#define	LEFT_BRACKET	333
#define	RIGHT_BRACKET	334
#define	LEFT_BRACE	335
#define	RIGHT_BRACE	336
#define	DOT	337
#define	COMMA	338
#define	COLON	339
#define	EQUAL	340
#define	SEMICOLON	341
#define	BANG	342
#define	DASH	343
#define	TILDE	344
#define	PLUS	345
#define	STAR	346
#define	SLASH	347
#define	PERCENT	348
#define	LEFT_ANGLE	349
#define	RIGHT_ANGLE	350
#define	VERTICAL_BAR	351
#define	CARET	352
#define	AMPERSAND	353
#define	QUESTION	354

