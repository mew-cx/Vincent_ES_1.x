/*
//
//Copyright (C) 2005-2006  Falanx Microsystems AS
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Falanx Microsystems AS nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//
*/
/*
//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//
*/
/* Based on
ANSI C grammar, Lex specification

In 1985, Jeff Lee published this Lex specification together with a Yacc 
grammar for the April 30, 1985 ANSI C draft.  Tom Stockfisch reposted 
both to net.sources in 1987; that original, as mentioned in the answer 
to question 17.25 of the comp.lang.c FAQ, can be ftp'ed from ftp.uu.net, 
file usenet/net.sources/ansi.c.grammar.Z. 

I intend to keep this version as close to the current C Standard grammar 
as possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
O           [0-7]

%option nounput 
%{
#include <stdio.h>
#include <stdlib.h>
#include "ParseHelper.h"
#include "glslang_tab.h"

// #define PRAGMA_TABLE

/* windows only pragma */
#ifdef _MSC_VER
#pragma warning(disable : 4102)
#endif

int yy_input(char* buf, int max_size);
TSourceLoc sl_yylineno;

#ifdef _WIN32
    extern int yyparse(TParseContext&);
    #define YY_DECL int yylex(YYSTYPE* pyylval, TParseContext& parseContext)    
#else
    extern int yyparse(void*);
    #define YY_DECL int yylex(YYSTYPE* pyylval, void* parseContextLocal)
    #define parseContext (*((TParseContext*)(parseContextLocal)))		
#endif
 
#define YY_INPUT(buf,result,max_size) (result = yy_input(buf, max_size))

%}

%option noyywrap
%option never-interactive
%option outfile="Gen_glslang.cpp"
%x FIELDS


%%
<*>"//"[^\n]*"\n"     { /* ?? carriage and/or line-feed? */ };

"attribute"    {  pyylval->lex.line = sl_yylineno; return(ATTRIBUTE); }
"const"        {  pyylval->lex.line = sl_yylineno; return(CONST_QUAL); }
"uniform"      {  pyylval->lex.line = sl_yylineno; return(UNIFORM); }
"varying"      {  pyylval->lex.line = sl_yylineno; return(VARYING); }

"break"        {  pyylval->lex.line = sl_yylineno; return(BREAK); }
"continue"     {  pyylval->lex.line = sl_yylineno; return(CONTINUE); }
"do"           {  pyylval->lex.line = sl_yylineno; return(DO); }
"for"          {  pyylval->lex.line = sl_yylineno; return(FOR); }
"while"        {  pyylval->lex.line = sl_yylineno; return(WHILE); }

"if"           {  pyylval->lex.line = sl_yylineno; return(IF); }
"else"         {  pyylval->lex.line = sl_yylineno; return(ELSE); }

"in"           {  pyylval->lex.line = sl_yylineno; return(IN_QUAL); }
"out"          {  pyylval->lex.line = sl_yylineno; return(OUT_QUAL); }
"inout"        {  pyylval->lex.line = sl_yylineno; return(INOUT_QUAL); }

"lowp"         {  pyylval->lex.line = sl_yylineno; return(LOW_PRECISION); }    /* Added for ESSL Support*/
"mediump"      {  pyylval->lex.line = sl_yylineno; return(MEDIUM_PRECISION); } /* Added for ESSL Support*/
"highp"        {  pyylval->lex.line = sl_yylineno; return(HIGH_PRECISION); }   /* Added for ESSL Support*/
"precision"    {  pyylval->lex.line = sl_yylineno; return(PRECISION); }         /* Added for ESSL Support*/
"invariant"    {  pyylval->lex.line = sl_yylineno; return(INVARIANT); }         /* Added for ESSL Support*/


"float"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"int"          {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(INT_TYPE); }
"void"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(VOID_TYPE); }
"bool"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(BOOL_TYPE); }
"true"         {  pyylval->lex.line = sl_yylineno; pyylval->lex.b = true;  return(BOOLCONSTANT); }
"false"        {  pyylval->lex.line = sl_yylineno; pyylval->lex.b = false; return(BOOLCONSTANT); }

"discard"      {  pyylval->lex.line = sl_yylineno; return(DISCARD); }
"return"       {  pyylval->lex.line = sl_yylineno; return(RETURN); }

"mat2"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(MATRIX2); }
"mat3"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(MATRIX3); }
"mat4"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return(MATRIX4); }

"vec2"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (VEC2); }
"vec3"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (VEC3); }
"vec4"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (VEC4); }
"ivec2"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (IVEC2); }
"ivec3"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (IVEC3); }
"ivec4"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (IVEC4); }
"bvec2"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (BVEC2); }
"bvec3"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (BVEC3); }
"bvec4"        {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return (BVEC4); }

%"sampler1D"       {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLER1D; } // Removed for ESSL support
"sampler2D"       {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLER2D; }
"sampler3D"       {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; parseContext.extensionErrorCheck(sl_yylineno, "GL_OES_texture_3D"); return SAMPLER3D; } // Altered for ESSL support
"samplerCube"     {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLERCUBE; }
%"sampler1DShadow" {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLER1DSHADOW; } // Removed for ESSL support
%"sampler2DShadow" {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLER2DSHADOW; } // Removed for ESSL support

%"sampler2DRect"        { pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLERRECTARB; /* ARB_texture_rectangle */ }			// Removed for ESSL support
%"sampler2DRectShadow"  { pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = true; return SAMPLERRECTSHADOWARB; /* ARB_texture_rectangle */ }	// Removed for ESSL support
"sampler2DRect"        {  PaReservedWord(); return 0; }			// Added for ESSL support
"sampler2DRectShadow"  {  PaReservedWord(); return 0; }			// Added for ESSL support

"struct"       {  pyylval->lex.line = sl_yylineno; return(STRUCT); }

"asm"          {  PaReservedWord(); return 0; }

"class"        {  PaReservedWord(); return 0; }
"union"        {  PaReservedWord(); return 0; }
"enum"         {  PaReservedWord(); return 0; }
"typedef"      {  PaReservedWord(); return 0; }
"template"     {  PaReservedWord(); return 0; }
"this"         {  PaReservedWord(); return 0; }
"packed"       {  PaReservedWord(); return 0; }

"goto"         {  PaReservedWord(); return 0; }
"switch"       {  PaReservedWord(); return 0; }
"default"      {  PaReservedWord(); return 0; }

"inline"       {  PaReservedWord(); return 0; }
"noinline"     {  PaReservedWord(); return 0; }
"volatile"     {  PaReservedWord(); return 0; }
"public"       {  PaReservedWord(); return 0; }
"static"       {  PaReservedWord(); return 0; }
"extern"       {  PaReservedWord(); return 0; }
"external"     {  PaReservedWord(); return 0; }
"interface"    {  PaReservedWord(); return 0; }
"flat"		   {  PaReservedWord(); return 0; }			// Added for ESSL support

"long"         {  PaReservedWord(); return 0; }
"short"        {  PaReservedWord(); return 0; }
"double"       {  PaReservedWord(); return 0; }
"half"         {  PaReservedWord(); return 0; }
"fixed"        {  PaReservedWord(); return 0; }
"unsigned"     {  PaReservedWord(); return 0; }
"superb"       {  PaReservedWord(); return 0; }			// Added for ESSL support

"input"        {  PaReservedWord(); return 0; }
"output"       {  PaReservedWord(); return 0; }

"hvec2"        {  PaReservedWord(); return 0; }
"hvec3"        {  PaReservedWord(); return 0; }
"hvec4"        {  PaReservedWord(); return 0; }
"fvec2"        {  PaReservedWord(); return 0; }
"fvec3"        {  PaReservedWord(); return 0; }
"fvec4"        {  PaReservedWord(); return 0; }
"dvec2"        {  PaReservedWord(); return 0; }
"dvec3"        {  PaReservedWord(); return 0; }
"dvec4"        {  PaReservedWord(); return 0; }


"sampler1D"       {  PaReservedWord(); return 0; } // Added for ESSL support
"sampler1DShadow" {  PaReservedWord(); return 0; } // Added for ESSL support
"sampler2DShadow" {  PaReservedWord(); return 0; } // Added for ESSL support
"sampler3DRect"   {  PaReservedWord(); return 0; }

"sizeof"       {  PaReservedWord(); return 0; }
"cast"         {  PaReservedWord(); return 0; }

"namespace"    {  PaReservedWord(); return 0; }
"using"        {  PaReservedWord(); return 0; }

{L}({L}|{D})*       {  
   pyylval->lex.line = sl_yylineno; 
   pyylval->lex.string = NewPoolTString(yytext); 
   return PaIdentOrType(*pyylval->lex.string, parseContext, pyylval->lex.symbol); 
}

0[xX]{H}+         { pyylval->lex.line = sl_yylineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }
0{O}+             { pyylval->lex.line = sl_yylineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }
0{D}+             { pyylval->lex.line = sl_yylineno; parseContext.error(sl_yylineno, "Invalid Octal number.", yytext, "", ""); parseContext.recover(); return 0;}
{D}+              { pyylval->lex.line = sl_yylineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }

{D}+{E}           { pyylval->lex.line = sl_yylineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
{D}+"."{D}*({E})? { pyylval->lex.line = sl_yylineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
"."{D}+({E})?     { pyylval->lex.line = sl_yylineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }

"/*"            {  int ret = PaParseComment(pyylval->lex.line, parseContext); if (!ret) return ret; }   

"+="            {  pyylval->lex.line = sl_yylineno; return(ADD_ASSIGN); }
"-="            {  pyylval->lex.line = sl_yylineno; return(SUB_ASSIGN); }
"*="            {  pyylval->lex.line = sl_yylineno; return(MUL_ASSIGN); }
"/="            {  pyylval->lex.line = sl_yylineno; return(DIV_ASSIGN); }
"%="            {  pyylval->lex.line = sl_yylineno; return(MOD_ASSIGN); }
"<<="           {  pyylval->lex.line = sl_yylineno; return(LEFT_ASSIGN); }
">>="           {  pyylval->lex.line = sl_yylineno; return(RIGHT_ASSIGN); }
"&="            {  pyylval->lex.line = sl_yylineno; return(AND_ASSIGN); }
"^="            {  pyylval->lex.line = sl_yylineno; return(XOR_ASSIGN); }
"|="            {  pyylval->lex.line = sl_yylineno; return(OR_ASSIGN); }

"++"            {  pyylval->lex.line = sl_yylineno; return(INC_OP); }
"--"            {  pyylval->lex.line = sl_yylineno; return(DEC_OP); }
"&&"            {  pyylval->lex.line = sl_yylineno; return(AND_OP); }
"||"            {  pyylval->lex.line = sl_yylineno; return(OR_OP); }
"^^"            {  pyylval->lex.line = sl_yylineno; return(XOR_OP); }
"<="            {  pyylval->lex.line = sl_yylineno; return(LE_OP); }
">="            {  pyylval->lex.line = sl_yylineno; return(GE_OP); }
"=="            {  pyylval->lex.line = sl_yylineno; return(EQ_OP); }
"!="            {  pyylval->lex.line = sl_yylineno; return(NE_OP); }
"<<"            {  pyylval->lex.line = sl_yylineno; return(LEFT_OP); }
">>"            {  pyylval->lex.line = sl_yylineno; return(RIGHT_OP); }
";"         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = false; return(SEMICOLON); }
("{"|"<%")      {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = false; return(LEFT_BRACE); }
("}"|"%>")      {  pyylval->lex.line = sl_yylineno; return(RIGHT_BRACE); }
","         {  pyylval->lex.line = sl_yylineno; if (parseContext.inTypeParen) parseContext.lexAfterType = false; return(COMMA); }
":"         {  pyylval->lex.line = sl_yylineno; return(COLON); }
"="         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = false; return(EQUAL); }
"("         {  pyylval->lex.line = sl_yylineno; parseContext.lexAfterType = false; parseContext.inTypeParen = true; return(LEFT_PAREN); }
")"         {  pyylval->lex.line = sl_yylineno; parseContext.inTypeParen = false; return(RIGHT_PAREN); }
("["|"<:")      {  pyylval->lex.line = sl_yylineno; return(LEFT_BRACKET); }
("]"|":>")      {  pyylval->lex.line = sl_yylineno; return(RIGHT_BRACKET); }
"."         { BEGIN(FIELDS);  return(DOT); }
"!"         {  pyylval->lex.line = sl_yylineno; return(BANG); }
"-"         {  pyylval->lex.line = sl_yylineno; return(DASH); }
"~"         {  pyylval->lex.line = sl_yylineno; return(TILDE); }
"+"         {  pyylval->lex.line = sl_yylineno; return(PLUS); }
"*"         {  pyylval->lex.line = sl_yylineno; return(STAR); }
"/"         {  pyylval->lex.line = sl_yylineno; return(SLASH); }
"%"         {  pyylval->lex.line = sl_yylineno; return(PERCENT); }
"<"         {  pyylval->lex.line = sl_yylineno; return(LEFT_ANGLE); }
">"         {  pyylval->lex.line = sl_yylineno; return(RIGHT_ANGLE); }
"|"         {  pyylval->lex.line = sl_yylineno; return(VERTICAL_BAR); }
"^"         {  pyylval->lex.line = sl_yylineno; return(CARET); }
"&"         {  pyylval->lex.line = sl_yylineno; return(AMPERSAND); }
"?"         {  pyylval->lex.line = sl_yylineno; return(QUESTION); }

<FIELDS>{L}({L}|{D})* { 
BEGIN(INITIAL);      
    pyylval->lex.line = sl_yylineno;     
    pyylval->lex.string = NewPoolTString(yytext); 
    return FIELD_SELECTION; }
<FIELDS>[ \t\v\f\r] {}

[ \t\v\n\f\r]   {  }
<*><<EOF>> { (&parseContext)->AfterEOF = true; yy_delete_buffer(YY_CURRENT_BUFFER); yyterminate();}
<*>.    { parseContext.infoSink.info << "FLEX: Unknown char " << yytext << "\n";
          return 0; }

%%


//Including Pre-processor.
extern "C" {
  #include "./preprocessor/preprocess.h"
} 

//
// The YY_INPUT macro just calls this.  Maybe this could be just put into
// the macro directly.
//

int yy_input(char* buf, int max_size)
{
     char *char_token =NULL;
     int len;

    if ((len = yylex_CPP(buf, max_size)) == 0)
        return 0;
    if (len >= max_size) 
        YY_FATAL_ERROR( "input buffer overflow, can't enlarge buffer because scanner uses REJECT" );

    buf[len] = ' ';
	return len+1;
}


//
// Parse an array of strings using yyparse.  We set up globals used by
// yywrap.
//
// Returns 0 for success, as per yyparse().
//
int PaParseStrings(char* argv[], int strLen[], int argc, TParseContext& parseContextLocal)
{
    int argv0len;
    
    ScanFromString(argv[0]); 
    
    //Storing the Current Compiler Parse context into the cpp structure.
	cpp->pC = (void*)&parseContextLocal;
	
	if (!argv || argc == 0)
        return 1;
    
    for (int i = 0; i < argc; ++i) {
        if (!argv[i]) {
            parseContextLocal.error(0, "Null shader source string", "", "");
            parseContextLocal.recover();
            return 1;
        }
    }
    
    if (!strLen) {
        argv0len = (int) strlen(argv[0]);
        strLen   = &argv0len;
    }
    yyrestart(0);
    (&parseContextLocal)->AfterEOF = false;
    cpp->PaWhichStr = 0;
    cpp->PaArgv     = argv;
    cpp->PaArgc     = argc;
    cpp->PaStrLen   = strLen;
    cpp->notAVersionToken = 0;
    sl_yylineno   = 1;
   
    if (*cpp->PaStrLen >= 0) {    
        int ret;
        #ifdef _WIN32
            ret = yyparse(parseContextLocal);            
        #else
            ret = yyparse((void*)(&parseContextLocal));
        #endif
        if (cpp->CompileError == 1 || parseContextLocal.recoveredFromError || parseContextLocal.numErrors > 0)
             return 1;
        else
             return 0;
    }
    else
        return 0;
}

void yyerror(char *s) 
{
    if (((TParseContext *)cpp->pC)->AfterEOF) {
        if (cpp->tokensBeforeEOF == 1) {
            GlobalParseContext->error(sl_yylineno, "syntax error", "pre-mature EOF", s, "");
            GlobalParseContext->recover();
        }
    } else {
        GlobalParseContext->error(sl_yylineno, "syntax error", yytext, s, "");
        GlobalParseContext->recover();
    }            
}

void PaReservedWord()
{
    GlobalParseContext->error(sl_yylineno, "Reserved word.", yytext, "", "");
    GlobalParseContext->recover();
}

int PaIdentOrType(TString& id, TParseContext& parseContextLocal, TSymbol*& symbol)
{
//    printf("PaIdent - %s\n",id.c_str());
    symbol = parseContextLocal.symbolTable.find(id);
    if (parseContextLocal.lexAfterType == false && symbol && symbol->isVariable()) {
        TVariable* variable = static_cast<TVariable*>(symbol);

//	    printf("  is Variable \n");

        if (variable->isUserType()) {
//		    printf("    is Usertype\n",id.c_str());
				
            parseContextLocal.lexAfterType = true;
            return TYPE_NAME;
        }
    }
    
    return IDENTIFIER;
}

int PaParseComment(int &lineno, TParseContext& parseContextLocal)
{
    int transitionFlag = 0;
    int nextChar;
    
    while (transitionFlag != 2) {
        nextChar = yyinput();
        if (nextChar == '\n')
             lineno++;
        switch (nextChar) {
        case '*' :
            transitionFlag = 1;
            break;
        case '/' :  /* if star is the previous character, then it is the end of comment */
            if (transitionFlag == 1) {
                return 1 ;
            }
            break;
        case EOF :
            /* Raise error message here */
            parseContextLocal.error(sl_yylineno, "End of shader found before end of comment.", "", "", "");
            GlobalParseContext->recover();
            return YY_NULL; 
        default :  /* Any other character will be a part of the comment */
            transitionFlag = 0;
        }
    }
    return 1;
}

extern "C" {

void CPPDebugLogMsg(const char *msg)
{
    ((TParseContext *)cpp->pC)->infoSink.debug.message(EPrefixNone, msg);
}

void CPPWarningToInfoLog(const char *msg)
{
    ((TParseContext *)cpp->pC)->infoSink.info.message(EPrefixWarning, msg, sl_yylineno); 
}

void CPPShInfoLogMsg(const char *msg)
{
    ((TParseContext *)cpp->pC)->error(sl_yylineno,"", "",msg,"");
    GlobalParseContext->recover();
}

void CPPErrorToInfoLog(char *msg)
{
    ((TParseContext *)cpp->pC)->error(sl_yylineno,"syntax error", "",msg,"");
    GlobalParseContext->recover();
}

void SetLineNumber(int line)
{
    sl_yylineno &= ~SourceLocLineMask;
    sl_yylineno |= line;
}

void SetStringNumber(int string)
{
    sl_yylineno = (string << SourceLocStringShift) | (sl_yylineno & SourceLocLineMask);
}

int GetStringNumber(void)
{
    return sl_yylineno >> 16;
}

int GetLineNumber(void)
{
    return sl_yylineno & SourceLocLineMask;
}

void IncLineNumber(void)
{
    if ((sl_yylineno & SourceLocLineMask) <= SourceLocLineMask)
        ++sl_yylineno;
}

void DecLineNumber(void)
{
    if ((sl_yylineno & SourceLocLineMask) > 0)
        --sl_yylineno;
}

void HandlePragma(const char **tokens, int numTokens)
{    
    if (!strcmp(tokens[0], "optimize")) {
        if (numTokens != 4) {
            CPPShInfoLogMsg("optimize pragma syntax is incorrect");
            return;
        }
        
        if (strcmp(tokens[1], "(")) {
            CPPShInfoLogMsg("\"(\" expected after 'optimize' keyword");
            return;
        }
            
        if (!strcmp(tokens[2], "on"))
            ((TParseContext *)cpp->pC)->contextPragma.optimize = true;
        else if (!strcmp(tokens[2], "off"))
            ((TParseContext *)cpp->pC)->contextPragma.optimize = false;
        else {
            CPPShInfoLogMsg("\"on\" or \"off\" expected after '(' for 'optimize' pragma");
            return;
        }
        
        if (strcmp(tokens[3], ")")) {
            CPPShInfoLogMsg("\")\" expected to end 'optimize' pragma");
            return;
        }
    } else if (!strcmp(tokens[0], "debug")) {
        if (numTokens != 4) {
            CPPShInfoLogMsg("debug pragma syntax is incorrect");
            return;
        }
        
        if (strcmp(tokens[1], "(")) {
            CPPShInfoLogMsg("\"(\" expected after 'debug' keyword");
            return;
        }
            
        if (!strcmp(tokens[2], "on"))
            ((TParseContext *)cpp->pC)->contextPragma.debug = true;
        else if (!strcmp(tokens[2], "off"))
            ((TParseContext *)cpp->pC)->contextPragma.debug = false;
        else {
            CPPShInfoLogMsg("\"on\" or \"off\" expected after '(' for 'debug' pragma");
            return;
        }
        
        if (strcmp(tokens[3], ")")) {
            CPPShInfoLogMsg("\")\" expected to end 'debug' pragma");
            return;
        }
	// Added for ESSL support '#pragma STDGL invariant(all)'
	} else if (numTokens==5 && !strcmp(tokens[0], "STDGL") && !strcmp(tokens[1],"invariant") ){
		if (!strcmp(tokens[2],"(") && !strcmp(tokens[4],")") && !strcmp(tokens[3],"all")) {
            ((TParseContext *)cpp->pC)->contextPragma.invariantAll = true;				
		} else {
            CPPShInfoLogMsg("invariant pragma syntax is incorrect");			
		}
    } else {

#ifdef PRAGMA_TABLE
        //
        // implementation specific pragma
        // use ((TParseContext *)cpp->pC)->contextPragma.pragmaTable to store the information about pragma
        // For now, just ignore the pragma that the implementation cannot recognize
        // An Example of one such implementation for a pragma that has a syntax like
        // #pragma pragmaname(pragmavalue)
        // This implementation stores the current pragmavalue against the pragma name in pragmaTable.
        //        
        if (numTokens == 4 && !strcmp(tokens[1], "(") && !strcmp(tokens[3], ")")) {              
            TPragmaTable& pragmaTable = ((TParseContext *)cpp->pC)->contextPragma.pragmaTable;
            TPragmaTable::iterator iter;
            iter = pragmaTable.find(TString(tokens[0]));
            if (iter != pragmaTable.end()) {
                iter->second = tokens[2];
            } else {
                pragmaTable[tokens[0]] = tokens[2];
            }        
        } else if (numTokens >= 2) {
            TPragmaTable& pragmaTable = ((TParseContext *)cpp->pC)->contextPragma.pragmaTable;
            TPragmaTable::iterator iter;
            iter = pragmaTable.find(TString(tokens[0]));
            if (iter != pragmaTable.end()) {
                iter->second = tokens[1];
            } else {
                pragmaTable[tokens[0]] = tokens[1];
            }
        }
#endif // PRAGMA_TABLE
    }
}

void StoreStr(char *string)
{
    TString strSrc;
    strSrc = TString(string);

    ((TParseContext *)cpp->pC)->HashErrMsg = ((TParseContext *)cpp->pC)->HashErrMsg + " " + strSrc;
}

const char* GetStrfromTStr(void)
{
    cpp->ErrMsg = (((TParseContext *)cpp->pC)->HashErrMsg).c_str();
    return cpp->ErrMsg;
}

void ResetTString(void)
{
    ((TParseContext *)cpp->pC)->HashErrMsg = "";
}

TBehavior GetBehavior(const char* behavior)
{
    if (!strcmp("require", behavior))
        return EBhRequire;
    else if (!strcmp("enable", behavior))
        return EBhEnable;
    else if (!strcmp("disable", behavior))
        return EBhDisable;
    else if (!strcmp("warn", behavior))
        return EBhWarn;
    else {
        CPPShInfoLogMsg((TString("behavior '") + behavior + "' is not supported").c_str());
        return EBhDisable;
    }        
}

void  updateExtensionBehavior(const char* extName, const char* behavior)
{
    TBehavior behaviorVal = GetBehavior(behavior);
    TMap<TString, TBehavior>:: iterator iter;
    TString msg;
    
    // special cased for all extension
    if (!strcmp(extName, "all")) {
        if (behaviorVal == EBhRequire || behaviorVal == EBhEnable) {
            CPPShInfoLogMsg("extension 'all' cannot have 'require' or 'enable' behavior");  
            return;
        } else {
            for (iter =  ((TParseContext *)cpp->pC)->extensionBehavior.begin(); iter != ((TParseContext *)cpp->pC)->extensionBehavior.end(); ++iter)
                iter->second = behaviorVal;
        }        
    } else {
        iter = ((TParseContext *)cpp->pC)->extensionBehavior.find(TString(extName));
        if (iter == ((TParseContext *)cpp->pC)->extensionBehavior.end()) {
            switch (behaviorVal) {
            case EBhRequire:
                CPPShInfoLogMsg((TString("extension '") + extName + "' is not supported").c_str());  
                break;
            case EBhEnable:
            case EBhWarn:
            case EBhDisable:
                msg = TString("extension '") + extName + "' is not supported";
                ((TParseContext *)cpp->pC)->infoSink.info.message(EPrefixWarning, msg.c_str(), sl_yylineno); 
                break;
            }
            return;
        } else
            iter->second = behaviorVal;
    }
}
        
}  // extern "C"

void setInitialState()
{
    yy_start = 1;
}
