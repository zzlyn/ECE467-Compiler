%{
/**********************************************************************
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <errno.h>
#include "symbol.h"

/*
Phase 3: Will be needed if some of the types defined in the parser.tab.h's 
yylval union are defined in your AST module.

#include "ast.h"
*/


#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS          [ \t]
DELIM       [ \t\n\(\)\[\]\{\},;\+\-\*\/\^=!\<\>]|&&|"||"|==|!=|"<"=|">"=|EOF
LETTER      [A-Za-z]
LETTERS     {LETTER}+
DIGIT       [0-9]
DIGITS      {DIGIT}+
ID_FORMAT   [A-Za-z_][A-Za-z0-9_]*
NUMBER      {DIGITS}+(\.{DIGITS})?([Ee][+-]?DIGITS)?
VALID_NUMS  (([1-9][0-9]*)|0)
NUMS        [0-9]+
FLOAT_SEGMENT (([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+))

%%

\/[\/]+.*   { /* 1 line comment, no action reqiured */ }   

"/*"        {
            int c1 = 0; int c2 = yyinput(); int curline = yyline;
            for(;;) {
                if (c2 == EOF) {
                    fprintf(errorFile,
                            "\nLEXICAL ERROR, LINE %d: Unmatched /*\n",
                            curline);
                    errorOccurred = TRUE;
                    yyterminate();
                    }
                if (c1 == '*' && c2== '/')
                    break;
                c1 = c2;
                c2 = yyinput();
                if (c1 == '\n' && c2 != EOF) {
                    yyline++;
                    }
                }
            }

\n          {yyline++;}

{WS}        { /* No Action Required */ }

const       { yTRACE(CONST); return CONST; }

if          { yTRACE(IF); return IF; }

then        { yTRACE(THEN); return THEN; }

else        { yTRACE(ELSE); return ELSE; }

while       { yTRACE(WHILE); return WHILE; }

<<EOF>>     {yyterminate();}

true        { yTRACE(BOOL_TRUE); yylval.boolVal = 1; return BOOL_TRUE; }

false       { yTRACE(BOOL_FALSE); yylval.boolVal = 0; return BOOL_FALSE; }

bool        { yTRACE(BOOL_T); return BOOL_T; }

int         { yTRACE(INT_T); return INT_T; }

float       { yTRACE(FLOAT_T); return FLOAT_T; }

vec2        { yTRACE(VEC2_T); return VEC2_T; }

vec3        { yTRACE(VEC3_T); return VEC3_T; }

vec4        { yTRACE(VEC4_T); return VEC4_T; }

bvec2        { yTRACE(BVEC2_T); return BVEC2_T; }

bvec3        { yTRACE(BVEC3_T); return BVEC3_T; }

bvec4        { yTRACE(BVEC4_T); return BVEC4_T; }

ivec2        { yTRACE(IVEC2_T); return IVEC2_T; }

ivec3        { yTRACE(IVEC3_T); return IVEC3_T; }

ivec4        { yTRACE(IVEC4_T); return IVEC4_T; }

dp3          { yTRACE(DP3_F); return DP3_F; }

rsq          { yTRACE(RSQ_F); return RSQ_F; }

lit          { yTRACE(LIT_F); return LIT_F; }

{ID_FORMAT}".".* { yERROR("Unknown token"); }

{ID_FORMAT}  { if(yyleng > 32) {
                  yERROR("Identifier exceeds maximum length");
              } else {
                  // Complete yylval.id
		  yylval.identifier = (char*) malloc(yyleng + 1);
		  strncpy(yylval.identifier, yytext, yyleng + 1);
                  // Tracing & return.
                  yTRACE(ID);
                  //printf("\nYOOOOOOO DAWWWWWWG %s \n",yytext);
                  return ID;
              }
            }

"("         { yTRACE(LBRAC); return LBRAC; }
")"         { yTRACE(RBRAC); return RBRAC; }
"["         { yTRACE(LSQBRAC); return LSQBRAC; }
"]"         { yTRACE(RSQBRAC); return RSQBRAC; }
"{"         { yTRACE(LSCOPE); return LSCOPE; }
"}"         { yTRACE(RSCOPE); return RSCOPE; }

","         { yTRACE(COMMA); return COMMA; }
";"         { yTRACE(SEMICOLON); return SEMICOLON; }

"+"         { yTRACE(PLUS); return PLUS; }
"-"         { yTRACE(MINUS); return MINUS; }
"*"         { yTRACE(MUL); return MUL; }
"/"         { yTRACE(DIV); return DIV; }
"^"         { yTRACE(POWER); return POWER; }

"="         { yTRACE(ASSIGN); return ASSIGN; }
"!"         { yTRACE(NOT); return NOT; }
"&&"        { yTRACE(AND); return AND; }

"||"        { yTRACE(OR); return OR; }
"=="        { yTRACE(EQ); return EQ; }
"!="        { yTRACE(NE); return NE; }
"<"         { yTRACE(LT); return LT; }
"<="        { yTRACE(LE); return LE; }
">"         { yTRACE(GT); return GT; }
">="        { yTRACE(GE); return GE; }

{VALID_NUMS}/{DELIM}	{
					long val = strtol(yytext, 0, 10);
					if (val < INT_MIN || val > INT_MAX) {
						yERROR("Out of bound integer.");
					}
					// Record the integer value.
					yylval.intVal = (int) val;
					yTRACE(INT);
					return INT;	
				}

({NUMS}|{FLOAT_SEGMENT})(("e"|"E")("-"|"+")?{NUMS})/{DELIM}   {
					// Special floating point numbers with scientific notation.
					// E.g. 1.0e4, 12.13E9
					
					// TODO: Implement parsing for scientific notations. (dont think
					// this is required for lab 1)
					yTRACE(FLOAT);
					return FLOAT;
					}

{FLOAT_SEGMENT}/{DELIM}	{
					// Standard floating point numbers, e.g. 1.23, .78, 4. .
					// 
					// This match needs to happen before scientific float match
					// in order to properly detect say 1.23e9.
					float val = strtof(yytext, NULL);
					
					errno = 0;
					if(errno == ERANGE) {
						yERROR("Out of bound float.");
					}
					
					yylval.floatVal = val;
                                        yTRACE(FLOAT); 
					return FLOAT;
                                  	}

.           { yERROR("unknown token"); }

%%
