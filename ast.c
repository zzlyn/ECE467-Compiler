#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
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
          ast->declaration.id = va_arg(args, std::string);
          ast->declaration.expression = va_arg(args, node*);
          break;

      case VAR_NODE:
          ast->variable.is_const = va_arg(args, int);
          ast->variable.id = va_arg(args, std::string);
          ast->variable.index = va_arg(args, int);
          break;

      case TYPE_NODE:
          ast->type.type = va_arg(args, int);
          ast->type.to_str = va_arg(args, std::string);
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
          ast->function.name = va_arg(args, std::string);
          ast->function.arguments = va_arg(args, node*);
          break;

      case ARGUMENTS_NODE:
          ast->arguments.arguments = va_arg(args, node*);
          ast->arguments.argument = va_arg(args, node*);
          break;

  // ...

  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {

}
