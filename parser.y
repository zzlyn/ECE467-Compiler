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
#include "ast.h"
#include "symbol.h"
//#include "semantic.h"



#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(const char* s);    /* what to do in case of error            */
int yylex();              /* procedure for calling lexical analyzer */
extern int yyline;        /* variable holding current line number   */

char* init_c_str(std::string str) {
    char* ptr = (char*) malloc(str.length() + 1);
    strcpy(ptr, str.c_str());

    // printf("DEBUG: initializing c string %s\n", ptr);

    return ptr;
}

// Translates function code to names for cleaner code.
char* function_code_to_name(int code) {
    if (code == 0) {
        return init_c_str("DP3");
    }

    if (code == 1) {
        return init_c_str("LIT");
    }

    if (code == 2) {
        return init_c_str("RSQ");
    }
    
    // We screwed.
    return NULL;
}


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
  char* identifier;
  int function_code;
  AstNode* ast_node;

  int type_code;
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

%token <identifier> ID

CONST

// Types
INT_T
%token <intVal> INT
FLOAT_T
%token <floatVal> FLOAT

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
%token <boolVal> BOOL_TRUE 
%token <boolVal> BOOL_FALSE 

ASSIGN 

NOT 
AND
OR

// Vector types.
%token <type_code> VOID_T
%token <type_code> VEC2_T
%token <type_code> VEC3_T
%token <type_code> VEC4_T
%token <type_code> BVEC2_T
%token <type_code> BVEC3_T
%token <type_code> BVEC4_T
%token <type_code> IVEC2_T
%token <type_code> IVEC3_T
%token <type_code> IVEC4_T

// Predefined functions.
%token <function_code> DP3_F
%token <function_code> LIT_F
%token <function_code> RSQ_F

%start    program

%left OR
%left AND

%nonassoc LT LE EQ NE GT GE

// '+', '-' lower priority than '*', '/' which is 
// lower than '^'.
%left PLUS MINUS
%left MUL DIV
%right POWER

// Strictly for '!' or '-' before variables.
%left NEGATIVE

// '[*]' and '(*)' are instantly reduced.
%left LSQBRAC RSQBRAC LBRAC RBRAC

%type <ast_node> program
%type <ast_node> scope
%type <ast_node> declarations
%type <ast_node> statements
%type <ast_node> type
%type <ast_node> declaration
%type <ast_node> statement
%type <ast_node> else_statement
%type <ast_node> variable
%type <ast_node> expression
%type <ast_node> constructor
%type <ast_node> function
%type <intVal> predefined_function
%type <ast_node> arguments
%type <ast_node> arguments_opt

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
  :  scope                                                              { yTRACE("program -> scope"); $$=ast_allocate(PROGRAM_NODE, $1); }
  ;
scope
  :  LSCOPE  declarations statements RSCOPE                                { yTRACE("scope -> {declarations statements} "); $$=ast_allocate(SCOPE_NODE, $2, $3); }
  ;
declarations
  :  declarations declaration                                           { yTRACE("declarations -> declarations declaration"); $$=ast_allocate(DECLARATIONS_NODE, $1, $2); }
  |  /* Epsilon */                                                      { yTRACE("declarations -> epsilon"); $$=NULL; }
  ;
statements
  :  statements statement                                               { yTRACE("statements -> statements statement"); $$=ast_allocate(STATEMENTS_NODE, $1, $2); }
  |  /* Epsilon */                                                      { yTRACE("statements -> epsilon"); $$=NULL; }
  ;
declaration
  :  type ID  SEMICOLON                                                  { yTRACE("declaration -> type ID ;"); $$=ast_allocate(DECLARATION_NODE, 0, $1, $2, NULL); }
  |  type ID ASSIGN expression SEMICOLON                                { yTRACE("declaration -> type ID = expression ;"); $$=ast_allocate(DECLARATION_NODE, 0, $1, $2, $4); }
  |  CONST type ID  ASSIGN expression SEMICOLON                          { yTRACE("declaration -> const type ID = expression ;"); $$=ast_allocate(DECLARATION_NODE, 1, $2, $3, $5); }
  ;
statement
  :  variable ASSIGN expression SEMICOLON                               { yTRACE("statement -> variable = expression ;");  $$=ast_allocate(ASSIGNMENT_NODE, $1, $3); }
  |  IF LBRAC expression RBRAC statement else_statement                 { yTRACE("statement -> if ( expression ) statement else_statement"); $$=ast_allocate(IF_STATEMENT_NODE, $3, $5, $6); }
  |  WHILE LBRAC expression RBRAC statement                             { yTRACE("statement -> while ( expression ) statement"); /* AST construction ignored as per lab manual. */}
  |  scope                                                              { yTRACE("statement -> scope"); $$=$1; }
  |  SEMICOLON                                                          { yTRACE("statement -> ;"); $$=NULL; }
  ;
else_statement
  :  ELSE statement                                                     { yTRACE("else_statement -> else statement"); $$=$2; }
  |  /* Epsilon */                                                      { yTRACE("else_statement -> epsilon"); $$=NULL; }
  ;
// Problem case we need to vid the seocnd one
variable
  :  ID                                                                 { yTRACE("variable -> ID"); $$=ast_allocate(VAR_NODE, 0/* is_const: Found in symbol table */, $1, 0,0); }
  |  ID  LSQBRAC INT RSQBRAC                                             { yTRACE("variable -> ID[integer]"); $$=ast_allocate(VAR_NODE, 0, $1, $3,1); }
  ;
type
  :  INT_T                                                              { yTRACE("type -> int"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("int")); }
  |  FLOAT_T                                                            { yTRACE("type -> float"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("float")); }
  |  BOOL_T                                                             { yTRACE("type -> bool"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("bool")); } 
  |  VEC2_T                                                             { yTRACE("type -> vec2"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("vec2")); }
  |  VEC3_T                                                             { yTRACE("type -> vec3"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("vec3")); }
  |  VEC4_T                                                             { yTRACE("type -> vec4"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("vec4")); }
  |  BVEC2_T                                                            { yTRACE("type -> bvec2"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("bvec2")); }
  |  BVEC3_T                                                            { yTRACE("type -> bvec3"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("bvec3")); }
  |  BVEC4_T                                                            { yTRACE("type -> bvec4"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("bvec4")); }
  |  IVEC2_T                                                            { yTRACE("type -> ivec2"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("ivec2")); }
  |  IVEC3_T                                                            { yTRACE("type -> ivec3"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("ivec3")); }
  |  IVEC4_T                                                            { yTRACE("type -> ivec4"); $$=ast_allocate(TYPE_NODE, $1, init_c_str("ivec4")); }
  ;
expression
  :  constructor                                                        { yTRACE("expression -> constructor"); $$=$1; }
  |  function                                                           { yTRACE("expression -> function"); $$=$1; }
  |  INT                                                                { yTRACE("expression -> integer"); $$=ast_allocate(INT_NODE, $1); }
  |  FLOAT                                                              { yTRACE("expression -> floatint point number"); double tmp = (double)$1; $$=ast_allocate(FLOAT_NODE, tmp); }
  |  BOOL_TRUE                                                          { yTRACE("expression -> TRUE"); $$=ast_allocate(BOOL_NODE, 1); }
  |  BOOL_FALSE                                                         { yTRACE("expression -> FALSE"); $$=ast_allocate(BOOL_NODE, 0); }
  |  variable                                                           { yTRACE("expression -> variable"); $$=$1; }
  |  MINUS expression   %prec NEGATIVE                                  { yTRACE("expression -> - expression"); $$=ast_allocate(UNARY_EXPRESSION_NODE, MINUS, $2); }
  |  NOT expression     %prec NEGATIVE                                  { yTRACE("expression -> ! expression"); $$=ast_allocate(UNARY_EXPRESSION_NODE, NOT, $2); }
  |  expression EQ expression                                           { yTRACE("expression -> expression == expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, EQ, $1, $3); }
  |  expression NE expression                                           { yTRACE("expression -> expression != expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, NE, $1, $3); }
  |  expression LT expression                                           { yTRACE("expression -> expression < expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, LT, $1, $3); }
  |  expression LE expression                                           { yTRACE("expression -> expression <= expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, LE, $1, $3); }
  |  expression GT expression                                           { yTRACE("expression -> expression > expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, GT, $1, $3); }
  |  expression GE expression                                           { yTRACE("expression -> expression >= expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, GE, $1, $3); }
  |  expression AND expression                                          { yTRACE("expression -> expression && expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, AND, $1, $3); }
  |  expression OR expression                                           { yTRACE("expression -> expression || expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, OR, $1, $3); }
  |  expression PLUS expression                                         { yTRACE("expression -> expression + expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, PLUS, $1, $3); }
  |  expression MINUS expression                                        { yTRACE("expression -> expression - expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, MINUS, $1, $3); }
  |  expression MUL expression                                          { yTRACE("expression -> expression * expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, MUL, $1, $3); }
  |  expression DIV expression                                          { yTRACE("expression -> expression / expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, DIV, $1, $3); }
  |  expression POWER expression                                        { yTRACE("expression -> expression ^ expression"); $$=ast_allocate(BINARY_EXPRESSION_NODE, POWER, $1, $3); }
  |  LBRAC expression RBRAC                                             { yTRACE("expression -> ( expression )"); $$=$2; }
  ;
constructor
  :  type LBRAC arguments RBRAC                                         { yTRACE("constructor -> type ( arguments )"); $$=ast_allocate(CONSTRUCTOR_NODE, $1, $3); }
  ;
arguments
  :  arguments COMMA expression                                         { yTRACE("arguments -> arguments , expression"); $$=ast_allocate(ARGUMENTS_NODE, $1, $3); }
  |  expression                                                         { yTRACE("arguments -> expression"); $$=ast_allocate(ARGUMENTS_NODE, NULL, $1); }
  ;
arguments_opt
  :  arguments                                                          { yTRACE("arguments_opt -> arguments"); $$=$1; }
  |  /* Epsilon */                                                      { yTRACE("arguments_opt -> epsilon"); $$=NULL; }
  ;
function
  :  predefined_function LBRAC arguments_opt RBRAC                      { yTRACE("function -> predefined_function ( arguments_opt )"); $$=ast_allocate(FUNCTION_NODE, function_code_to_name($1), $3); }
  ;
predefined_function
  : DP3_F                                                               { yTRACE("predefined_function -> dp3"); $$=0; }
  | LIT_F                                                               { yTRACE("predefined_function -> lit"); $$=1; }
  | RSQ_F                                                               { yTRACE("predefined_function -> rsq"); $$=2; }
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

