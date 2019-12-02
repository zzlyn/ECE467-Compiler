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
void assembly_print(node *n);
//http://www.eecg.toronto.edu/~jzhu/csc467/readings/csc467_finalVersion.pdf?fbclid=IwAR1jw3sOREgfYkC337wigX_8yIfPZlfUpH8kBLUJ8CMIxF6NaCQ9QzKoX3E

char * str_to_char (string str ){
    char* ca = new char[str.size()+1];
    std::copy(str.begin(), str.end(), ca);
    ca[str.size()] = '\0';
    return ca;
}



std::string binary_op_assembly_str(int op) {
    std::string str;
    switch (op) {


        case PLUS: {
                       str = "ADD";
                       break;}

        case MINUS:{
                       str = "SUB";
                       break;}

        case MUL: {
                      str = "MUL";
                      break;}

        case DIV: {
                      str = "/";
                      break;}

        case POWER:{
                       str = "POW";
                       break;}

        default: {
                     str = "UNKNOWN YOOOO";
                     break;}
    }
    return str;
}


std::string get_index_string(int deref, int index){
    if( deref == 0){
        return "";
    }


    if(index == 0){
        return ".x";
    }
    if(index == 1){
        return ".y";
    }
    if(index == 2){
        return ".z";
    }
    if(index == 3){
        return ".w";
    }
    return "ERROR in get_index_string";
}

void assembly_check(AstNode* node);

std::vector<std::vector<std::string>> used_reg_names;
std::vector<std::string> free_reg_names;
std::vector<std::string> if_reg_names;


int max_register = 0;
bool created_new_reg = false;

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
    created_new_reg = true;
    std::string new_reg_name =  "temp_" + to_string(max_register);
    add_used_name(new_reg_name);
    max_register = max_register + 1;
    //td::cout << "Generated name is "<< new_reg_name << std::endl;
    std::cout << "TEMP " << new_reg_name << ";" << std::endl;

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




void genCode(AstNode* ast) {
    printf("!!ARBfp1.0\n");    
    assembly_check(ast);
    printf("END\n");

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
        case INT_NODE: {// Needs to set ExprEval.
                           break;


                           node->result_reg = str_to_char(to_string(node->integer.val));
                           //struct {
                           //            int val;
                           //        } integer;



                       }

        case FLOAT_NODE: {// Needs to set ExprEval.
                             node->result_reg = str_to_char(to_string(node->float_num.val));

                             //struct {
                             //            float val;
                             //        } float_num;

                             break;


                         }

        case BOOL_NODE: {// Needs to set ExprEval.

                            node->result_reg = str_to_char(to_string(node->boolean.val));

                            //  struct {
                            //            bool val;
                            //        } boolean

                            break;
                        }

        case PROGRAM_NODE:
                        break;

        case SCOPE_NODE:
                        break;

        case DECLARATIONS_NODE:{
                               }

        case STATEMENTS_NODE:	break;
        case UNARY_EXPRESSION_NODE:	break;// Not necessary for both demos.
        case BINARY_EXPRESSION_NODE:{



                                        string binary_op_inst = binary_op_assembly_str(node->binary_expr.op);
                                        char * right_name = node->binary_expr.right->result_reg;

                                        //if(right_name) std::cout <<" Right name is " << right_name << std::endl;

                                        char *  left_name =  node->binary_expr.left-> result_reg;

                                        //if(left_name) std::cout <<"Left name is " << left_name << std::endl;



                                        //std::cout <<"Op is " << binary_op_inst  << std::endl;
                                        std::string reg_name = get_new_reg_name(NULL);
                                        node->result_reg = str_to_char(reg_name);
                                        std::cout << binary_op_inst << " " << reg_name << ", " ;
                                        std::cout << left_name << ", " << right_name << std::endl;

                                        break; // Needs to set ExprEval.

                                    }

        case DECLARATION_NODE:{	
                                  //printf("In the declaration node \n");
                                  char * id = node->declaration.id;
                                  addToSymbolTable(id, 1 , 1 , 1);
                                  node->instruction = "NO_OP";
                                  //std::string to_print = get_assembly_line(node);
                                  //std::cout << "Assemnbl;y line is " << to_print << std::endl;
                                  //std::cout <<"Node is " << id << std::endl;
                                  std::string reg_name = get_new_reg_name(id);
                                  node->result_reg = str_to_char(reg_name);
                                  //std::cout << "Reg name is " << reg_name << std::endl ;
                                  //std::cout << "Reg name is " << get_reg_name(id) << std::endl ;
                                  //std::cout << "TEMP " << reg_name << ";" << std::endl;
                                  if(node->declaration.expression == NULL){
                                      std::cout << "MOV " << reg_name << " 0.00000;" << std::endl;
                                  }

                                  else {
                                      //DO SOMETHING IF INITILIZED
                                  }
                                  break;
                              }
        case VAR_NODE:	{

                            node->instruction = "NO_OP";
                            //std::cout << "In var node " << std::endl;
                            //std:: cout << "Var name is " << node->variable. id << std::endl;
                            //std::cout << "Index is " << node->variable.index << std::endl;
                            //std::cout <<"Index assembly string is " << get_index_string(node->variable.index) << std::endl;
                            string reg_name  = get_reg_name( node->variable. id);
                            string index_string =  get_index_string(node->variable. deref,node->variable.index);
                            reg_name = reg_name + index_string;
                            node->result_reg = str_to_char(reg_name);
                            //std::cout << "Reg name is " <<  node->result_reg << std::endl;

                            break;
                        }
        case TYPE_NODE:	break;
        case IF_STATEMENT_NODE:{
                            // Do nothing as this is post traversal.
                                   break;


                               }
        case ASSIGNMENT_NODE:{

                                 // struct {
                                 //            AstNode* variable; // Contains type info.
                                 //            AstNode* expression;
                                 //        } assignment;

                                 char *  result_name =  node->assignment.variable->result_reg;
                                 char *  expression_name =  node->assignment.expression->result_reg;


                                 std::cout << "MOV " <<result_name << ", " <<  expression_name <<std::endl;

                                 break;



                             }
        case CONSTRUCTOR_NODE:	break;
        case FUNCTION_NODE:{


                               //  char* name;
                               //  AstNode* arguments;
                               std::string reg_name = get_new_reg_name(NULL);
                               node->result_reg = str_to_char(reg_name);
                               //cout <<  node->function.name << " " << reg_name ;

                               //std::cout << "Function name is " << node->function.name << std::endl;
                               AstNode* func_args =  node->function.arguments; 

                               //AstNode* arguments;
                               //AstNode* expression;
                               std::cout << node->function.name << " " << reg_name;
                               while(func_args != NULL){
                                   cout << ", " << func_args->result_reg;
                                   //cout << "Expression reg is " << func_args->result_reg;
                                   func_args = func_args->arguments.arguments;
                               }
                               std::cout << std::endl;

                               break;


                           }
        case ARGUMENTS_NODE:{

                                node->result_reg = node->arguments.expression->result_reg;
                                break;


                            }




    }

    created_new_reg = false;
}	

void assembly_check(AstNode* node){
    ast_traverse_post_if_optimized(node, &assembly_check_node);
}



