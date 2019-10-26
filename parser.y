%{
/***********************************************************************
 *   Interface to the parser module for CSC467 course project.
 * 
 *   Phase 2: Implement context free grammar for source language, and
 *            parse tracing functionality.
 *   Phase 3: Construct the AST for the source language program.
 ***********************************************************************/

/***********************************************************************
 *  C Definitions and external declarations for this module.
 *
 *  Phase 3: Include ast.h if needed, and declarations for other global or
 *           external vars, functions etc. as needed.
 ***********************************************************************/

#include <string.h>
#include "common.h"
//#include "ast.h"
//#include "symbol.h"
//#include "semantic.h"
#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(const char* s);    /* what to do in case of error            */
int yylex();              /* procedure for calling lexical analyzer */
extern int yyline;        /* variable holding current line number   */

%}

/***********************************************************************
 *  Yacc/Bison declarations.
 *  Phase 2:
 *    1. Add precedence declarations for operators (after %start declaration)
 *    2. If necessary, add %type declarations for some nonterminals
 *  Phase 3:
 *    1. Add fields to the union below to facilitate the construction of the
 *       AST (the two existing fields allow the lexical analyzer to pass back
 *       semantic info, so they shouldn't be touched).
 *    2. Add <type> modifiers to appropriate %token declarations (using the
 *       fields of the union) so that semantic information can by passed back
 *       by the scanner.
 *    3. Make the %type declarations for the language non-terminals, utilizing
 *       the fields of the union as well.
 ***********************************************************************/

%{
#define YYDEBUG 1
%}

%union {
  int intVal;
  float floatVal;
  bool boolVal;
  char *identifier;
}

%token 

// Relational operations
LT
LE
EQ
NE
GT
GE

// Basic system keywords.
IF
THEN
ELSE
WHILE

ID

CONST

// Types
INT_T
INT
FLOAT_T
FLOAT

// Symbols
LBRAC
RBRAC
LSQBRAC
RSQBRAC 
LSCOPE 
RSCOPE 

COMMA 
SEMICOLON 

// Math ops.
PLUS 
MINUS 
MUL 
DIV 
POWER 

// Boolean stuff.
BOOL_T 
BOOL_TRUE 
BOOL_FALSE 

ASSIGN 

NOT 
AND
OR

// Vector types.
VOID_T
VEC2_T
VEC3_T
VEC4_T
BVEC2_T
BVEC3_T
BVEC4_T
IVEC2_T
IVEC3_T
IVEC4_T

// Predefined functions.
DP3_F
LIT_F
RSQ_F

%start    program

%left OR
%left AND

%left LT LE EQ NE GT GE

// '+', '-' lower priority than '*', '/' which is 
// lower than '^'.
%left PLUS MINUS
%left MUL DIV
%right POWER

// Strictly for '!' or '-' before variables.
%left NEGATIVE

// '[*]' and '(*)' are instantly reduced.
%left LSQBRAC RSQBRAC LBRAC RBRAC

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 *  Phase 3:
 *    1. Add code to rules for construction of AST.
 ***********************************************************************/
program
  :   tokens
  ;
tokens
  :  tokens token  
  |      
  ;
token
  :	LT {}
  |	LE {}
  |	EQ {}
  |	NE {}
  |	GT {}
  |	GE {}
  |     IF {}
  |     THEN {}
  |     ELSE {}
  |     WHILE {}
  |     ID {}
  |     CONST {}
  |     INT_T {}
  |     INT {}
  |     FLOAT_T {}
  |     FLOAT {}
  |     LBRAC {}
  |     RBRAC {}
  |     LSQBRAC {}
  |     RSQBRAC {}
  |     LSCOPE {}
  |     RSCOPE {}
  |     COMMA {}
  |     SEMICOLON {}
  |     PLUS {}
  |     MINUS {}
  |     MUL {}
  |     DIV {}
  |     POWER {}
  |     BOOL_T {}
  |     BOOL_TRUE {}
  |     BOOL_FALSE {}
  |     ASSIGN {}
  |     NOT {}
  |     AND {}
  |     OR {}
  |     VOID_T {}
  |     VEC2_T {}
  |     VEC3_T {}
  |     VEC4_T {}
  |     BVEC2_T {}
  |     BVEC3_T {}
  |     BVEC4_T {}
  |     IVEC2_T {}
  |     IVEC3_T {}
  |     IVEC4_T {}
  |     DP3_F {}
  |     LIT_F {}
  |     RSQ_F {}    
  ;	


%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(const char* s) {
  if (errorOccurred)
    return;    /* Error has already been reported by scanner */
  else
    errorOccurred = 1;
        
  fprintf(errorFile, "\nPARSER ERROR, LINE %d",yyline);
  if (strcmp(s, "parse error")) {
    if (strncmp(s, "parse error, ", 13))
      fprintf(errorFile, ": %s\n", s);
    else
      fprintf(errorFile, ": %s\n", s+13);
  } else
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
}

