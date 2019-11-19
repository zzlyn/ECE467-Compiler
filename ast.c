#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "symbol.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

#define PRINT_DUMP(...) fprintf(dumpFile, __VA_ARGS__);

node *ast = NULL;

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  ast = (AstNode *) malloc(sizeof(AstNode));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {
  
      case INT_NODE:
          ast->integer.val = va_arg(args, int);
          break;

      case FLOAT_NODE:
          ast->float_num.val = va_arg(args, double);
          break;

      case BOOL_NODE:
          ast->boolean.val = va_arg(args, int);
          break;

      case PROGRAM_NODE:
          ast->program.scope = va_arg(args, node*);
          break;

      case SCOPE_NODE:
          ast->scope.declarations = va_arg(args, node*);
          ast->scope.statements = va_arg(args, node*);
          break;

      case DECLARATIONS_NODE:
          ast->declarations.declarations = va_arg(args, node*);
          ast->declarations.declaration = va_arg(args, node*);
          break;

      case STATEMENTS_NODE:
          ast->statements.statements = va_arg(args, node*);
          ast->statements.statement = va_arg(args, node*);
          break;

      case UNARY_EXPRESSION_NODE:
          ast->unary_expr.op = va_arg(args, int);
          ast->unary_expr.right = va_arg(args, node*);
          break;
    
      case BINARY_EXPRESSION_NODE:
          ast->binary_expr.op = va_arg(args, int);
          ast->binary_expr.left = va_arg(args, node*);
          ast->binary_expr.right = va_arg(args, node*);
          break;

      case DECLARATION_NODE:
          ast->declaration.is_const = va_arg(args, int);
          ast->declaration.type = va_arg(args, node*);
          ast->declaration.id = va_arg(args, char*);
          ast->declaration.expression = va_arg(args, node*);
          break;

      case VAR_NODE:
          ast->variable.is_const = va_arg(args, int);
          ast->variable.id = va_arg(args, char*);
          ast->variable.index = va_arg(args, int);
          ast->variable.deref = va_arg(args, int);
          break;

      case TYPE_NODE:
          ast->type.type = va_arg(args, int);
          ast->type.to_str = va_arg(args, char*);
          break;

      case IF_STATEMENT_NODE:
          ast->if_statement.condition = va_arg(args, node*);
          ast->if_statement.statement = va_arg(args, node*);
          ast->if_statement.else_statement = va_arg(args, node*);
          break;

      case ASSIGNMENT_NODE:
          ast->assignment.variable = va_arg(args, node*);
          ast->assignment.expression = va_arg(args, node*);
          break;

      case CONSTRUCTOR_NODE:
          ast->constructor.type = va_arg(args, node*);
          ast->constructor.arguments = va_arg(args, node*);
          break;

      case FUNCTION_NODE:
          ast->function.name = va_arg(args, char*);
          ast->function.arguments = va_arg(args, node*);
          break;

      case ARGUMENTS_NODE:
          ast->arguments.arguments = va_arg(args, node*);
          ast->arguments.expression = va_arg(args, node*);
          break;

  default: break;
  
  }

  va_end(args);

  return ast;
}

std::string var_type_to_str(int var_type) {
    std::string str;

    switch(var_type) {
        case INT_T:
            str = "int";
            break;

        case FLOAT_T:
            str = "float";
            break;

        case BOOL_T:
            str = "bool";
            break;

        case VEC2_T:
            str = "vec2";
            break;

        case VEC3_T:
            str = "vec3";
            break;

        case VEC4_T:
            str = "vec4";
            break;

        case IVEC2_T:
            str = "ivec2";
            break;

        case IVEC3_T:
            str = "ivec3";
            break;

        case IVEC4_T:
            str = "ivec4";
            break;

        case BVEC2_T:
            str = "bvec2";
            break;

        case BVEC3_T:
            str = "bvec3";
            break;

        case BVEC4_T:
            str = "bvec4";
            break;

        default:
            str = "unknown";
            break;
    }

    return str;
}

std::string binary_op_to_str(int op) {

    std::string str;

    switch (op) {

        case EQ: {
            str = "==";
            break;
                 }

        case NE: {
            str = "!=";
            break;}

        case LT: {
            str = "<";
            break;}

        case LE: {
            str = "<=";
            break;}

        case GT: {
            str = ">";
            break;}

        case GE: {
            str = ">=";
            break;}

        case AND: {
            str = "&&";
            break;}

        case OR: {
            str = "||";
            break;}

        case PLUS: {
            str = "+";
            break;}

        case MINUS:{ 
            str = "-";
            break;}
            
        case MUL: {
            str = "*";
            break;}

        case DIV: {
            str = "/";
            break;}

        case POWER:{ 
            str = "^";
            break;}

        default: {
            str = "";
            break;}
    }
    return str;
}

void ast_traverse_post(node *ast, NodeFunc post_call) {
    
    if (ast == NULL) {
        return;
    }

    switch(ast->kind) {
      case INT_NODE:
          break;

      case FLOAT_NODE:
          break;
    
      case BOOL_NODE:
          break;

      case PROGRAM_NODE:
          ast_traverse_post(ast->program.scope, post_call);
          break;

      case SCOPE_NODE:
          addScope();
          ast_traverse_post(ast->scope.declarations, post_call);
          ast_traverse_post(ast->scope.statements, post_call);
          break;

      case DECLARATIONS_NODE:
          ast_traverse_post(ast->declarations.declarations, post_call);
          ast_traverse_post(ast->declarations.declaration, post_call);
          break;

      case STATEMENTS_NODE:
          ast_traverse_post(ast->statements.statements, post_call);
          ast_traverse_post(ast->statements.statement, post_call);
          break;

      case UNARY_EXPRESSION_NODE:
          ast_traverse_post(ast->unary_expr.right, post_call);
          break;
    
      case BINARY_EXPRESSION_NODE:
          ast_traverse_post(ast->binary_expr.left, post_call);
          ast_traverse_post(ast->binary_expr.right, post_call);
          break;

      case DECLARATION_NODE:
          ast_traverse_post(ast->declaration.expression, post_call);
          break;

      case VAR_NODE:
          break;

      case TYPE_NODE:
          break;

      case IF_STATEMENT_NODE:
          // printf("1\n");
          ast_traverse_post(ast->if_statement.condition, post_call);
          // printf("2\n");
          ast_traverse_post(ast->if_statement.statement, post_call);
          ast_traverse_post(ast->if_statement.else_statement, post_call);
          break;

      case ASSIGNMENT_NODE:
          ast_traverse_post(ast->assignment.variable, post_call);
          ast_traverse_post(ast->assignment.expression, post_call);
          break;

      case CONSTRUCTOR_NODE:
          ast_traverse_post(ast->constructor.type, post_call);
          ast_traverse_post(ast->constructor.arguments, post_call);
          break;

      case FUNCTION_NODE:
          ast_traverse_post(ast->function.arguments, post_call);
          break;

      case ARGUMENTS_NODE:
          ast_traverse_post(ast->arguments.arguments, post_call);
          ast_traverse_post(ast->arguments.expression, post_call);
          break;

  // ...

  default: break;
  }
    if (post_call) post_call(ast);
    if (ast->kind == SCOPE_NODE) { subtractScope(); }
}

static int indentation = 0;

void print_indentation() {
    for(int i = 0; i < indentation; i++)
        PRINT_DUMP(" ");
}

void ast_print(node *ast) {
    
    if (ast == NULL) {
        return;
    }

    switch(ast->kind) {
      case INT_NODE:
          PRINT_DUMP("%i", ast->integer.val);
          break;

      case FLOAT_NODE:
          PRINT_DUMP("%f", ast->float_num.val);
          break;

      case BOOL_NODE:
          if (ast->boolean.val) {
            PRINT_DUMP("true");
          } else {
            PRINT_DUMP("false");
          }
          break;

      case PROGRAM_NODE:
          PRINT_DUMP("(PROGRAM \n");
          indentation++;
          ast_print(ast->program.scope);
          indentation--;
          PRINT_DUMP(")\n");
          break;

      case SCOPE_NODE:
          print_indentation();
          PRINT_DUMP("(SCOPE \n");

          indentation++;

          ast_print(ast->scope.declarations);
          // One empty line between to make things look nicer.
          PRINT_DUMP("\n");
          ast_print(ast->scope.statements);
          
          indentation--;
          
          print_indentation();
          PRINT_DUMP(")\n");
          break;

      case DECLARATIONS_NODE: {
          // Reverse declarations order from bottom up to top down.
          AstNode* prev = NULL;
          while(ast != NULL) {
            AstNode* next = ast->declarations.declarations;
            ast->declarations.declarations = prev;
            prev = ast;
            ast = next;
          }
          ast = prev;

          print_indentation();
          PRINT_DUMP("(DECLARATIONS \n");
          indentation++;
          
          AstNode* tmp = ast;
          while(tmp != NULL) {
            ast_print(tmp->declarations.declaration);
            tmp = tmp->declarations.declarations;
          }
          
          indentation--;
          print_indentation();
          PRINT_DUMP(")\n");
          break;
                              }

      case STATEMENTS_NODE: {
          // Reverse statements order from bottom up to top down.
          AstNode* prev = NULL;
          while(ast != NULL) {
            AstNode* next = ast->statements.statements;
            ast->statements.statements = prev;
            prev = ast;
            ast = next;
          }
          ast = prev;

          print_indentation();
          PRINT_DUMP("(STATEMENTS \n");
          indentation++;

          AstNode* tmp = ast;
          while(tmp != NULL) {
            ast_print(tmp->statements.statement);
            tmp = tmp->statements.statements;
          }
          
          indentation--;
          print_indentation();
          PRINT_DUMP(")\n");
          break;
                            }

      case UNARY_EXPRESSION_NODE: {
          PRINT_DUMP("(UNARY ");
          
          std::string type = ast->unary_expr.op == MINUS ? "ARITHMETIC" : "LOGICAL";
          std::string symbol = ast->unary_expr.op == MINUS ? "- " : "! ";

          PRINT_DUMP("%s %s", type.c_str(), symbol.c_str());
          ast_print(ast->unary_expr.right);
          PRINT_DUMP(")");
          break;
                                  }
    
      case BINARY_EXPRESSION_NODE: {
          PRINT_DUMP("(BINARY ");
          // Print type.
          int t = ast->binary_expr.op;
          if (t == PLUS || t == MINUS || t == MUL || t == DIV || t == POWER) {
            PRINT_DUMP("ARITHMETIC ");
          } else {
            PRINT_DUMP("LOGICAL ");
          }
          // Print operation.
          PRINT_DUMP("%s ", binary_op_to_str(t).c_str());
          // Print left and right.
          ast_print(ast->binary_expr.left);
          PRINT_DUMP(" ");
          ast_print(ast->binary_expr.right);

          PRINT_DUMP(")");
          break;
                                   }

      case DECLARATION_NODE:
          print_indentation();
          PRINT_DUMP("(DECLARATION ");
          // variable_name type_name initial_value
          PRINT_DUMP("%s %s ", ast->declaration.id, ast->declaration.type->type.to_str);
          ast_print(ast->declaration.expression);
          PRINT_DUMP(")\n");
          break;

      case VAR_NODE:
          if (!ast->variable.deref) {
            PRINT_DUMP("%s", ast->variable.id);
          } else {
            // INDEX type id index.
            PRINT_DUMP("(INDEX %s %s %i)", var_type_to_str(ast->variable.var_type).c_str(), ast->variable.id, ast->variable.index);
          }
          break;

      case TYPE_NODE:
          break;

      case IF_STATEMENT_NODE:
          // IF COND THEN_STATEMENT ELSE_STATEMENT
          print_indentation();
          PRINT_DUMP("(IF ");
          indentation++;
          ast_print(ast->if_statement.condition);
          PRINT_DUMP("\n");
          print_indentation();
          ast_print(ast->if_statement.statement);
          
          if (ast->if_statement.else_statement) {
            indentation--;
            print_indentation();
            PRINT_DUMP(" ELSE\n");
            indentation++;
            
            print_indentation();
            ast_print(ast->if_statement.else_statement);
          }
          
          indentation--;
          print_indentation();
          PRINT_DUMP(")\n");
          break;

      case ASSIGNMENT_NODE:
          // ASSIGN type variable_name new_value
          print_indentation();
          PRINT_DUMP("(ASSIGN %s %s ", var_type_to_str(ast->assignment.variable->variable.var_type).c_str(),ast->assignment.variable->variable.id);
          ast_print(ast->assignment.expression);
          PRINT_DUMP(")\n");
          break;

      case CONSTRUCTOR_NODE:
          PRINT_DUMP("(CALL %s ", ast->constructor.type->type.to_str);
          ast_print(ast->constructor.arguments);
          PRINT_DUMP(")");
          break;

      case FUNCTION_NODE:
          PRINT_DUMP("(CALL %s ", ast->function.name);
          ast_print(ast->function.arguments);
          PRINT_DUMP(")");
          break;

      case ARGUMENTS_NODE:
          ast_print(ast->arguments.expression);
          break;

  // ...

  default: break;
  }
}

void ast_free(node *ast) {
    
    if (ast == NULL) {
        return;
    }

    switch(ast->kind) {
      case INT_NODE:
          break;

      case FLOAT_NODE:
          break;
    
      case BOOL_NODE:
          break;

      case PROGRAM_NODE:
          ast_free(ast->program.scope);
          break;

      case SCOPE_NODE:
          ast_free(ast->scope.declarations);
          ast_free(ast->scope.statements);
          break;

      case DECLARATIONS_NODE:
          ast_free(ast->declarations.declarations);
          ast_free(ast->declarations.declaration);
          break;

      case STATEMENTS_NODE:
          ast_free(ast->statements.statements);
          ast_free(ast->statements.statement);
          break;

      case UNARY_EXPRESSION_NODE:
          ast_free(ast->unary_expr.right);
          break;
    
      case BINARY_EXPRESSION_NODE:
          ast_free(ast->binary_expr.left);
          ast_free(ast->binary_expr.right);
          break;

      case DECLARATION_NODE:
          ast_free(ast->declaration.type);
          free(ast->declaration.id);
          ast_free(ast->declaration.expression);
          break;

      case VAR_NODE:
          free(ast->variable.id);
          break;

      case TYPE_NODE:
          free(ast->type.to_str);
          break;

      case IF_STATEMENT_NODE:
          ast_free(ast->if_statement.condition);
          ast_free(ast->if_statement.statement);
          ast_free(ast->if_statement.else_statement);
          break;

      case ASSIGNMENT_NODE:
          ast_free(ast->assignment.variable);
          ast_free(ast->assignment.expression);
          break;

      case CONSTRUCTOR_NODE:
          ast_free(ast->constructor.type);
          ast_free(ast->constructor.arguments);
          break;

      case FUNCTION_NODE:
          free(ast->function.name);
          ast_free(ast->function.arguments);
          break;

      case ARGUMENTS_NODE:
          ast_free(ast->arguments.arguments);
          ast_free(ast->arguments.expression);
          break;

  // ...

  default: break;
  }
    free(ast);
}

