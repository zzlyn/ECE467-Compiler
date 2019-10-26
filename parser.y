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
  :  scope                                                              { yTRACE("program -> scope"); }
  ;
scope
  :  LSCOPE declarations statements RSCOPE                                { yTRACE("scope -> {declarations statements} "); }
  ;
declarations
  :  declarations declaration                                           { yTRACE("declarations -> declarations declaration"); }
  |  /* Epsilon */                                                      { yTRACE("declarations -> epsilon"); }
  ;
statements
  :  statements statement                                               { yTRACE("statements -> statements statement"); }
  |  /* Epsilon */                                                      { yTRACE("statements -> epsilon"); }
  ;
declaration
  :  type ID SEMICOLON                                                  { yTRACE("declaration -> type ID ;"); }
  |  type ID ASSIGN expression SEMICOLON                                { yTRACE("declaration -> type ID = expression ;"); }
  |  CONST type ID ASSIGN expression SEMICOLON                          { yTRACE("declaration -> const type ID = expression ;"); }
  ;
statement
  :  variable ASSIGN expression SEMICOLON                               { yTRACE("statement -> variable = expression ;"); }
  |  IF LBRAC expression RBRAC statement else_statement                 { yTRACE("statement -> if ( expression ) statement else_statement"); }
  |  WHILE LBRAC expression RBRAC statement                             { yTRACE("statement -> while ( expression ) statement"); }
  |  scope                                                              { yTRACE("statement -> scope"); }
  |  SEMICOLON                                                          { yTRACE("statement -> ;"); }
  ;
else_statement
  :  ELSE statement                                                     { yTRACE("else_statement -> else statement"); }
  |  /* Epsilon */                                                      { yTRACE("else_statement -> epsilon"); }
  ;
variable
  :  ID                                                                 { yTRACE("variable -> ID"); }
  |  ID LSQBRAC INT RSQBRAC                                             { yTRACE("variable -> ID[integer]"); }
  ;
type
  :  INT_T                                                              { yTRACE("type -> int"); }
  |  FLOAT_T                                                            { yTRACE("type -> float"); }
  |  BOOL_T                                                             { yTRACE("type -> bool"); }
  |  VEC2_T                                                             { yTRACE("type -> vec2"); }
  |  VEC3_T                                                             { yTRACE("type -> vec3"); }
  |  VEC4_T                                                             { yTRACE("type -> vec4"); }
  |  BVEC2_T                                                            { yTRACE("type -> bvec2"); }
  |  BVEC3_T                                                            { yTRACE("type -> bvec3"); }
  |  BVEC4_T                                                            { yTRACE("type -> bvec4"); }
  |  IVEC2_T                                                            { yTRACE("type -> ivec2"); }
  |  IVEC3_T                                                            { yTRACE("type -> ivec3"); }                                                      
  |  IVEC4_T                                                            { yTRACE("type -> ivec4"); }
  ;
expression
  :  constructor                                                        { yTRACE("expression -> constructor"); }
  |  function                                                           { yTRACE("expression -> function"); }
  |  INT                                                                { yTRACE("expression -> integer"); }
  |  FLOAT                                                              { yTRACE("expression -> floatint point number"); }
  |  BOOL_TRUE                                                          { yTRACE("expression -> TRUE"); }
  |  BOOL_FALSE                                                         { yTRACE("expression -> FALSE"); }
  |  variable                                                           { yTRACE("expression -> variable"); }
  |  MINUS expression   %prec NEGATIVE                                  { yTRACE("expression -> - expression"); }
  |  NOT expression     %prec NEGATIVE                                  { yTRACE("expression -> ! expression"); }
  |  expression relation_op expression                                  { yTRACE("expression -> expression relation_op expression"); }
  |  expression math_op expression                                      { yTRACE("expression -> expression math_op expression"); }
  |  LBRAC expression RBRAC                                             { yTRACE("expression -> ( expression )"); }
  ;
relation_op
  :  EQ                                                                 { yTRACE("relation_op -> =="); }
  |  NE                                                                 { yTRACE("relation_op -> !="); }
  |  LT                                                                 { yTRACE("relation_op -> <"); }
  |  LE                                                                 { yTRACE("relation_op -> <="); }
  |  GT                                                                 { yTRACE("relation_op -> >"); }
  |  GE                                                                 { yTRACE("relation_op -> >="); }
  |  AND                                                                { yTRACE("relation_op -> &&"); }
  |  OR                                                                 { yTRACE("relation_op -> ||"); }
  ;
math_op
  :  PLUS                                                               { yTRACE("math_op -> +"); }
  |  MINUS                                                              { yTRACE("math_op -> -"); }
  |  MUL                                                                { yTRACE("math_op -> *"); }
  |  DIV                                                                { yTRACE("math_op -> /"); }
  |  POWER                                                              { yTRACE("math_op -> ^"); }
  ;
constructor
  :  type LBRAC arguments RBRAC                                         { yTRACE("constructor -> type ( arguments )"); }
  ;
arguments
  :  arguments COMMA expression                                         { yTRACE("arguments -> arguments , expression"); }
  |  expression                                                         { yTRACE("arguments -> expression"); }
  ;
function
  :  predefined_function LBRAC arguments RBRAC                          { yTRACE("function -> predefined_function ( arguments )"); }
  |  predefined_function LBRAC RBRAC                                    { yTRACE("function -> predefined_function ( )"); }
  ;
predefined_function
  : DP3_F                                                               { yTRACE("predefined_function -> dp3"); }
  | LIT_F                                                               { yTRACE("predefined_function -> lit"); }
  | RSQ_F                                                               { yTRACE("predefined_function -> rsq"); }
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

