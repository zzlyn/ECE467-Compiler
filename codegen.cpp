#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdarg>
#include "ast.h"
#include "parser.tab.h"
#include "symbol.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>


#include "ast.h"
#include "symbol.h"
#include "common.h"
#include "semantic.h"
#include "parser.tab.h"
#define DEBUG_PRINT_TREE 0
#define PRINT_DUMP(...) fprintf(dumpFile, __VA_ARGS__);

using namespace std; 

static const std::string ASB_FILE_NAME = "asb.txt";
extern "C" void assembly_print(node *n);



std::vector<std::vector<std::string>> used_reg_names;
std::vector<std::string> free_reg_names;
int max_register = 0;

// Creates a new register name if you need it 
std::string gen_new_reg_name(){
	string new_reg_name =  "temp_" + to_string(max_register);
	int scope_depth = used_reg_names.size() - 1;
	if(scope_depth >= 0){	
		used_reg_names[scope_depth].push_back(new_reg_name);
	}
	else{
		printf("ISSUE WITH gen_new_reg_name FUNCTION \n");
	}
	max_register = max_register + 1;
	return new_reg_name;
}

// gets a register for someone to use 
// If null it is a temporary register
// If not null get the reg name from symbol table
std::string  get_reg_name(char * var_name){


	if(var_name != NULL){


	}

	else{



	}
} 




extern "C" void genCode(AstNode* ast) {
    printf("genCode start\n");    
//	assembly_print(ast);
}



std::string  get_assembly_line(node * n){

	std::string to_return = ""; 


	//printf("Instruction is %d \n", instruction);
	if( n->instruction  != NULL){
	        std::string instruction (n->instruction);
		to_return.append(instruction);
	}
	else{
		to_return.append("No instruction");
	}


	return to_return; 
}




static int indentation = 0;



void assembly_print(node *n) {

    if (n == NULL) {
        return;
    }

    switch(n->kind) {
        case INT_NODE:
            PRINT_DUMP("%i", n->integer.val);
            break;

        case FLOAT_NODE:
            PRINT_DUMP("%f", n->float_num.val);
            break;

        case BOOL_NODE:
            if (n->boolean.val) {
                PRINT_DUMP("true");
            } else {
                PRINT_DUMP("false");
            }
            break;

        case PROGRAM_NODE:
            PRINT_DUMP("(PROGRAM \n");
            indentation++;
	    n->instruction = (char * ) malloc(sizeof(char) * 1 );
            assembly_print(n->program.scope);
            indentation--;
            PRINT_DUMP(")\n");
            break;

        case SCOPE_NODE:
            PRINT_DUMP("(SCOPE \n");

            indentation++;

            assembly_print(n->scope.declarations);
            // One empty line between to make things look nicer.
            if (n->scope.declarations)
                PRINT_DUMP("\n");
            assembly_print(n->scope.statements);

            indentation--;

            PRINT_DUMP(")\n");
            break;

        case DECLARATIONS_NODE: {
                                    // Reverse declarations order from bottom up to top down.

					n->instruction = (char*) malloc(2 * sizeof(char));
					*(n->instruction) = 'c';
					*(n->instruction  + 1) = '\0';
					printf("Value is %s\n", n->instruction);


					std::string decl_string = get_assembly_line(n);
					std::cout << "What ya got :" <<  decl_string.c_str() << std::endl;
                                    AstNode* prev = NULL;
                                    while(n != NULL) {
                                        AstNode* next = n->declarations.declarations;
                                        n->declarations.declarations = prev;
                                        prev = n;
                                        n = next;
                                    }
                                    n = prev;

                                    PRINT_DUMP("(DECLARATIONS \n");
                                    indentation++;

                                    AstNode* tmp = n;
                                    while(tmp != NULL) {
                                        assembly_print(tmp->declarations.declaration);
                                        tmp = tmp->declarations.declarations;
                                    }

                                    indentation--;
                                    PRINT_DUMP(")\n");
                                    break;
                                }

        case STATEMENTS_NODE: {
                                  // Reverse statements order from bottom up to top down.
                                  AstNode* prev = NULL;
                                  while(n != NULL) {
                                      AstNode* next = n->statements.statements;
                                      n->statements.statements = prev;
                                      prev = n;
                                      n = next;
                                  }
                                  n = prev;

                                  PRINT_DUMP("(STATEMENTS \n");
                                  indentation++;

                                  AstNode* tmp = n;
                                  while(tmp != NULL) {
                                      assembly_print(tmp->statements.statement);
                                      tmp = tmp->statements.statements;
                                  }

                                  indentation--;
                                  PRINT_DUMP(")\n");
                                  break;
                              }

        case UNARY_EXPRESSION_NODE: {
                                        PRINT_DUMP("(UNARY ");

                                        std::string type = var_type_to_str(ExprNodeToType(n->unary_expr.right));
                                        std::string symbol = n->unary_expr.op == MINUS ? "- " : "! ";

                                        PRINT_DUMP("%s %s", type.c_str(), symbol.c_str());
                                        assembly_print(n->unary_expr.right);
                                        PRINT_DUMP(")");
                                        break;
                                    }

        case BINARY_EXPRESSION_NODE: {
                                         PRINT_DUMP("(BINARY ");
                                         // Print type.
                                         PRINT_DUMP("%s ", var_type_to_str(ExprNodeToType(n)).c_str());
                                         // Print operation.
                                         //PRINT_DUMP("%s ", binary_op_to_str(n->binary_expr.op).c_str());
                                         // Print left and right.
                                         assembly_print(n->binary_expr.left);
                                         PRINT_DUMP(" ");
                                         assembly_print(n->binary_expr.right);

                                         PRINT_DUMP(")");
                                         break;
                                     }

        case DECLARATION_NODE:
                                     PRINT_DUMP("(DECLARATION ");
                                     // variable_name type_name initial_value
                                     assembly_print(n->declaration.type);
                                     PRINT_DUMP("%s %s ", n->declaration.id, n->declaration.type->type.to_str);
                                     assembly_print(n->declaration.expression);
                                     PRINT_DUMP(")\n");
                                     break;

        case VAR_NODE:
                                     if (!n->variable.deref) {
                                         PRINT_DUMP("%s", n->variable.id);
                                     } else {
                                         // INDEX type id index.
                                         PRINT_DUMP("(INDEX %s %s %i)", var_type_to_str(n->variable.var_type).c_str(), n->variable.id, n->variable.index);
                                     }
                                     break;

        case TYPE_NODE:
                                     break;

        case IF_STATEMENT_NODE:
                                     PRINT_DUMP("(IF ");
                                     indentation++;
                                     assembly_print(n->if_statement.condition);
                                     PRINT_DUMP("\n");

                                     assembly_print(n->if_statement.statement);

                                     if (n->if_statement.else_statement != NULL) {
                                         indentation--;
                                         PRINT_DUMP(" ELSE\n");
                                         indentation++;
                                         assembly_print(n->if_statement.else_statement);
                                     }

                                     indentation--;
                                     PRINT_DUMP(")\n");
                                     break;

        case ASSIGNMENT_NODE:
                                     // ASSIGN type variable_name new_value
                                     PRINT_DUMP("(ASSIGN %s %s ", var_type_to_str(n->assignment.variable->variable.var_type).c_str(),n->assignment.variable->variable.id);
                                     assembly_print(n->assignment.expression);
                                     PRINT_DUMP(")\n");
                                     break;

        case CONSTRUCTOR_NODE:
                                     PRINT_DUMP("(CALL %s ", n->constructor.type->type.to_str);
                                     assembly_print(n->constructor.arguments);
                                     PRINT_DUMP(")");
                                     break;

        case FUNCTION_NODE:
                                     PRINT_DUMP("(CALL %s ", n->function.name);
                                     assembly_print(n->function.arguments);
                                     PRINT_DUMP(")");
                                     break;

        case ARGUMENTS_NODE: {
                                 AstNode* prev = NULL;
                                 // Reverse arguments.
                                 while(n != NULL) {
                                     AstNode* next = n->arguments.arguments;
                                     n->arguments.arguments = prev;
                                     prev = n;
                                     n = next;
                                 }
                                 n = prev;

                                 AstNode* tmp = n;
                                 while(tmp != NULL) {
                                     assembly_print(tmp->arguments.expression);
                                     std::string suffix = tmp->arguments.arguments != NULL ? ", " : "";
                                     PRINT_DUMP(suffix.c_str());
                                     tmp = tmp->arguments.arguments;
                                 }
                                 break;
                             }

        default: break;
    }
}


