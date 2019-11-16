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
  ast = (AstNode *) malloc(sizeof(AstNode));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {
  
      case INT_NODE:
          ast->integer.val = va_arg(args, int);
          //printf("int %i\n", ast->integer.val);
          break;

      case FLOAT_NODE:
          ast->float_num.val = va_arg(args, double);
          //printf("float %f\n", ast->float_num.val);
          break;

      case PROGRAM_NODE:
          ast->program.scope = va_arg(args, node*);
          break;

      case SCOPE_NODE:
          ast->scope.declarations = va_arg(args, node*);
          ast->scope.statements = va_arg(args, node*);
          //printf("scope\n");
          break;

      case DECLARATIONS_NODE:
          ast->declarations.declarations = va_arg(args, node*);
          ast->declarations.declaration = va_arg(args, node*);
          //printf("declarations\n");
          break;

      case STATEMENTS_NODE:
          ast->statements.statements = va_arg(args, node*);
          ast->statements.statement = va_arg(args, node*);
          //printf("statements\n");
          break;

      case UNARY_EXPRESSION_NODE:
          ast->unary_expr.op = va_arg(args, int);
          ast->unary_expr.right = va_arg(args, node*);
          //printf("unary_expression, op %i\n", ast->unary_expr.op);
          break;
    
      case BINARY_EXPRESSION_NODE:
          ast->binary_expr.op = va_arg(args, int);
          ast->binary_expr.left = va_arg(args, node*);
          ast->binary_expr.right = va_arg(args, node*);
          //printf("binary_expression, op %i\n", ast->binary_expr.op);
          break;

      case DECLARATION_NODE:
          ast->declaration.is_const = va_arg(args, int);
          ast->declaration.type = va_arg(args, node*);
          ast->declaration.id = va_arg(args, char*);
          ast->declaration.expression = va_arg(args, node*);
          //printf("declaration, id %s\n", ast->declaration.id);
          break;

      case VAR_NODE:
          ast->variable.is_const = va_arg(args, int);
          ast->variable.id = va_arg(args, char*);
          ast->variable.index = va_arg(args, int);
          printf("var, id %s, index %i\n", ast->variable.id, ast->variable.index);
          break;

      case TYPE_NODE:
          ast->type.type = va_arg(args, int);
          ast->type.to_str = va_arg(args, char*);
          //printf("type, type %i, to_str %s\n", ast->type.type, ast->type.to_str);
          break;

      case IF_STATEMENT_NODE:
          ast->if_statement.condition = va_arg(args, node*);
          ast->if_statement.statement = va_arg(args, node*);
          ast->if_statement.else_statement = va_arg(args, node*);
          //printf("if_statement\n");
          break;

      case ASSIGNMENT_NODE:
          ast->assignment.variable = va_arg(args, node*);
          ast->assignment.expression = va_arg(args, node*);
          //printf("assignment\n");
          break;

      case CONSTRUCTOR_NODE:
          ast->constructor.type = va_arg(args, node*);
          ast->constructor.arguments = va_arg(args, node*);
          //printf("constructor\n");
          break;

      case FUNCTION_NODE:
          ast->function.name = va_arg(args, char*);
          ast->function.arguments = va_arg(args, node*);
          //printf("function %s\n", ast->function.name);
          break;

      case ARGUMENTS_NODE:
          ast->arguments.arguments = va_arg(args, node*);
          ast->arguments.expression = va_arg(args, node*);
          //printf("arguments\n");
          break;

  default: break;
  
  }

  va_end(args);

  return ast;
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

static int indent_count = 0;

void ast_traverse_post(node *ast, NodeFunc post_call) {
    
    if (ast == NULL) {
        return;
    }

    switch(ast->kind) {
      case INT_NODE:
          break;

      case FLOAT_NODE:
          break;

      case PROGRAM_NODE:
          ast_traverse_post(ast->program.scope, post_call);
          break;

      case SCOPE_NODE:
          // TODO: Call enter_scope().
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
          ast_traverse_post(ast->variable.type, post_call);
          break;

      case TYPE_NODE:
          break;

      case IF_STATEMENT_NODE:
          ast_traverse_post(ast->if_statement.condition, post_call);
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
    if (ast->kind == SCOPE_NODE) {/* TODO: Call exitscope(). */}
}


void ast_print(node *ast) {
    
    if (ast == NULL) {
        return;
    }

    indent_count++;

    // Print indentation.
    fprintf(dumpFile, "\n");
    for (int i = 0; i < indent_count; i++) {
        fprintf(dumpFile, "  ");
    }
    fprintf(dumpFile, "(");

    switch(ast->kind) {
      case INT_NODE:
          fprintf(dumpFile, "%i", ast->integer.val);
          break;

      case FLOAT_NODE:
          fprintf(dumpFile, "%f", ast->float_num.val);
          break;

      case PROGRAM_NODE:
          fprintf(dumpFile, "PROGRAM ");
          ast_print(ast->program.scope);
          break;

      case SCOPE_NODE:
          fprintf(dumpFile, "SCOPE ");
          ast_print(ast->scope.declarations);
          ast_print(ast->scope.statements);
          break;

      case DECLARATIONS_NODE:
          //ast->declarations.declarations = va_arg(args, node*);
          //ast->declarations.declaration = va_arg(args, node*);
          fprintf(dumpFile, "DECLARATIONS ");
          ast_print(ast->declarations.declarations);
          ast_print(ast->declarations.declaration);
          break;

      case STATEMENTS_NODE:
          //ast->statements.statements = va_arg(args, node*);
          //ast->statements.statement = va_arg(args, node*);
          fprintf(dumpFile, "STATEMENTS ");
          ast_print(ast->statements.statements);
          ast_print(ast->statements.statement);
          break;

      case UNARY_EXPRESSION_NODE: {
          //ast->unary_expr.op = va_arg(args, int);
          //ast->unary_expr.right = va_arg(args, node*);
          fprintf(dumpFile, "UNARY ");
          // TODO: Type missing.
          std::string symbol = ast->unary_expr.op == MINUS ? "- " : "! ";
          fprintf(dumpFile, "%s", symbol.c_str());
          ast_print(ast->unary_expr.right);
          break;
                                  }
    
      case BINARY_EXPRESSION_NODE:
          // TODO: Result type missing.
          fprintf(dumpFile, "BINARY ");
          // Print operation.
          fprintf(dumpFile, "%s", binary_op_to_str(ast->binary_expr.op).c_str());
          // Print left and right.
          ast_print(ast->binary_expr.left);
          ast_print(ast->binary_expr.right);
          break;

      case DECLARATION_NODE:
          //ast->declaration.is_const = va_arg(args, int);
          //ast->declaration.type = va_arg(args, node*);
          //ast->declaration.id = va_arg(args, std::string);
          //ast->declaration.expression = va_arg(args, node*);
          fprintf(dumpFile, "DECLARATION ");
          // variable_name type_name initial_value
          fprintf(dumpFile, "%s %s ", ast->declaration.id, ast->declaration.type->type.to_str);
          ast_print(ast->declaration.expression);
          break;

      case VAR_NODE:
          //ast->variable.is_const = va_arg(args, int);
          //ast->variable.id = va_arg(args, std::string);
          //ast->variable.index = va_arg(args, int);
          if (ast->variable.index == -1) {
            fprintf(dumpFile, "%s", ast->variable.id);
          } else {
            // INDEX type id index.
            fprintf(dumpFile, "INDEX fake_type %s %i", ast->variable.id, ast->variable.index);
          }
          break;

      case TYPE_NODE:
          //ast->type.type = va_arg(args, int);
          //ast->type.to_str = va_arg(args, std::string);
          fprintf(dumpFile, "TYPE %s", ast->type.to_str);
          break;

      case IF_STATEMENT_NODE:
          //ast->if_statement.condition = va_arg(args, node*);
          //ast->if_statement.statement = va_arg(args, node*);
          //ast->if_statement.else_statement = va_arg(args, node*);
          // IF COND THEN_STATEMENT ELSE_STATEMENT
          fprintf(dumpFile, "IF ");
          ast_print(ast->if_statement.condition);
          ast_print(ast->if_statement.statement);
          ast_print(ast->if_statement.else_statement);
          break;

      case ASSIGNMENT_NODE:
          //ast->assignment.variable = va_arg(args, node*);
          //ast->assignment.expression = va_arg(args, node*);
          // ASSIGN type variable_name new_value
          fprintf(dumpFile, "ASSIGN fake_type %s ", ast->assignment.variable->variable.id);
          ast_print(ast->assignment.expression);
          break;

      case CONSTRUCTOR_NODE:
          //ast->constructor.type = va_arg(args, node*);
          //ast->constructor.arguments = va_arg(args, node*);
          fprintf(dumpFile, "CALL %s ", ast->constructor.type->type.to_str);
          ast_print(ast->constructor.arguments);
          break;

      case FUNCTION_NODE:
          //ast->function.name = va_arg(args, std::string);
          //ast->function.arguments = va_arg(args, node*);
          fprintf(dumpFile, "CALL %s ", ast->function.name);
          ast_print(ast->function.arguments);
          break;

      case ARGUMENTS_NODE:
          ast_print(ast->arguments.expression);
          break;

  // ...

  default: break;
  }

    // Print exit indentation.
    printf("\n");
    for (int i = 0; i<indent_count; i++) {
        printf("  ");
    }
    printf(")\n");

    indent_count--;

}

void ast_free(node * ast) {

}
