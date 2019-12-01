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
//http://www.eecg.toronto.edu/~jzhu/csc467/readings/csc467_finalVersion.pdf?fbclid=IwAR1jw3sOREgfYkC337wigX_8yIfPZlfUpH8kBLUJ8CMIxF6NaCQ9QzKoX3E





void assembly_check(AstNode* node);

std::vector<std::vector<std::string>> used_reg_names;
std::vector<std::string> free_reg_names;
int max_register = 0;

void add_reg_scope(){
	vector<std::string> new_scope;
	used_reg_names.push_back(new_scope);
}

void subtract_reg_scope(){
        used_reg_names.pop_back();
}


void add_used_name( string reg_name){
        int scope_depth = used_reg_names.size() - 1;
        if(scope_depth >= 0){
                used_reg_names[scope_depth].push_back(reg_name);
        }
        else{
                printf("ISSUE WITH add_used_name FUNCTION \n");
        }
}

// Creates a new register name if you need it 
std::string gen_new_reg_name(){
	std::string new_reg_name =  "temp_" + to_string(max_register);
	add_used_name(new_reg_name);
	max_register = max_register + 1;
	//td::cout << "Generated name is "<< new_reg_name << std::endl;
	return new_reg_name;
}

// gets a register for someone to use 
// If null it is a temporary register and you need to create a name
// If not null get the reg name from symbol table
std::string  get_new_reg_name(char * var_name){


	if(var_name != NULL){
		// this checks synbol table
		if(does_var_have_regname(var_name)){
			return get_reg_name(var_name);
		}
	}

	int num_free_names = free_reg_names.size() ;
	std::string return_name;	
	if(num_free_names >  0){
		return_name = free_reg_names[num_free_names - 1];
		free_reg_names.pop_back();
		add_used_name( return_name); 
	}

	else{
		return_name = gen_new_reg_name();
	}


	if(var_name != NULL){
		assign_reg_name(var_name,return_name);
	}

	//std::cout << "Return name is "<< return_name << std::endl;
	return return_name;
} 




extern "C" void genCode(AstNode* ast) {
    printf("genCode start\n");    
	assembly_check(ast);

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



	assembly_arg * assembly_args = n->assembly_args;

	if(assembly_args == NULL){
		to_return.append(" No args");
	}


	else {
		while(assembly_args != NULL){
			to_return.append(assembly_args->reg_name);
			to_return.append(" ");
			assembly_args = assembly_args-> next_arg; 
		}
	}

	return to_return; 
}



void assembly_check_node(AstNode* node) {
    if (node == NULL) return;


    switch(node->kind) {
        case INT_NODE: // Needs to set ExprEval.
            break;

        case FLOAT_NODE: // Needs to set ExprEval.
            break;

        case BOOL_NODE: // Needs to set ExprEval.
            break;

        case PROGRAM_NODE:
            break;

        case SCOPE_NODE:
            break;

        case DECLARATIONS_NODE:{
			}

        case STATEMENTS_NODE:	break;
        case UNARY_EXPRESSION_NODE:	break;// Needs to set ExprEval.
        case BINARY_EXPRESSION_NODE:	break; // Needs to set ExprEval.
        case DECLARATION_NODE:{	
                                //printf("In the declaration node \n");
                                char * id = node->declaration.id;
				addToSymbolTable(id, 1 , 1 , 1);
				//node->instruction = "TEMP";
				//std::string to_print = get_assembly_line(node);
				//std::cout << "Assemnbl;y line is " << to_print << std::endl;

				//std::cout <<"Node is " << id << std::endl;
				std::string reg_name = get_new_reg_name(id);
				//std::cout << "Reg name is " << reg_name << std::endl ;
				//std::cout << "Reg name is " << get_reg_name(id) << std::endl ;
				std::cout << "TEMP " << reg_name << ";" << std::endl;

				break;
			}
        case VAR_NODE:	break;
        case TYPE_NODE:	break;
        case IF_STATEMENT_NODE:	break;
        case ASSIGNMENT_NODE:	break;
        case CONSTRUCTOR_NODE:	break;
        case FUNCTION_NODE:	break;
        case ARGUMENTS_NODE:	break;
	}

}	
                                                                                                                                    
void assembly_check(AstNode* node){
    ast_traverse_post(node, &assembly_check_node);
}



