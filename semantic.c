#include "ast.h"
#include "symbol.h"
#include  "parser.tab.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h> 

#define ARITHMETIC_OP  556
#define LOGICAL_OP  557
#define COMPARISON_OP  558
#define SCALAR 559
#define VECTOR 560

#define DEBUGGING false

#define DEBUG_PRINT(...) { if(DEBUGGING) printf(__VA_ARGS__); }
#define ERROR(...) { fprintf(errorFile, __VA_ARGS__); errorOccurred = 1; }

int checkPredefinedVectorIndex(int indexValue, char * varname);
int getOpType(int op);
int getBaseType(int op);
int getClassSize(int type);

int ExprNodeToType(AstNode* node);

int isReadOnly(char * varname, int line){

    if(predefinedVarnameCheck(varname)){
        if(!strcmp(varname,"gl_FragColor") || !strcmp(varname,"gl_FragDepth")){
            return 0;
        }
        ERROR("Error(line %i): Assigning a value to a read-only variable %s\n", line,varname);
        return 1;
    }

    if(doesVarExist(varname)){
        int isConst = getConstType(varname);
	if(isConst){
 	       ERROR("Error(line %i): Assigning a value to const variable %s\n", line,varname);
	}
        return isConst;
    }

    ERROR("Varname doesn't exist\n");
    return 0;
}



int isWriteOnly(char * varname){

    if(!strcmp(varname,"gl_FragColor") || !strcmp(varname,"gl_FragDepth")){
        return 1;
    }
    return 0;
}


int writeOnlyCheck(AstNode * node){

	if(node->kind == VAR_NODE){
		if(isWriteOnly(node->variable.id)){
			return 1;
		}
	}
	return 0;	
} 



int getNumArgs(AstNode * node){
    int count = 0;
    while(node != NULL){
        count = count + 1;
        node = node->arguments.arguments;
    }
    return count;
}


int argTypeCheck(int typeToCheck, AstNode * givenNode){
    AstNode * node = givenNode;        
    while(node != NULL){
        ExprEval ee = node->arguments.expression->ee;
	if(node->arguments.expression->kind  == VAR_NODE){
		if(writeOnlyCheck(node->arguments.expression) && errorOccurred == 0 ){
			ERROR("Error: Reading from write only variable on line %d\n",node->arguments.expression->line);
		}
		
	}

        if(typeToCheck != ee.base_type){
            return 0;
        }

        node = node->arguments.arguments;
    }
    return 1;
}

int argSizeCheck(int sizeToCheck, AstNode * givenNode){
    AstNode * node = givenNode;
    while(node != NULL){
        ExprEval ee = node->arguments.expression->ee;
        int classSize = ee.class_size;
        if(node->arguments.expression->kind == VAR_NODE && node->arguments.expression->variable.deref){
            classSize = 1;
        }
        if(sizeToCheck != classSize){
            return 0;
        }

        node = node->arguments.arguments;
    }
    return 1;
}

int numArgsConstruct(int type){

    if(type == BVEC4_T || type == IVEC4_T || type == VEC4_T){
        return 4;
    }

    if(type == BVEC3_T || type == IVEC3_T || type == VEC3_T){
        return 3;
    }

    if(type == BVEC2_T || type == IVEC2_T || type == VEC2_T){
        return 2;
    }

    if(type == BOOL_T || type == FLOAT_T || type == INT_T){
        return 1;
    }

    return 0;
}

std::string ExprNodeToTypeStr(AstNode* node) {
    return var_type_to_str(ExprNodeToType(node));
}

ExprEval evaluate_unary_expression(AstNode* node) {
    // Check if op and child type match. Class does not matter since
    // unary accepts both vectors and scalars.
    int child_type = node->unary_expr.right->ee.expr_type;

    std::string child_type_str = ExprNodeToTypeStr(node->unary_expr.right);

    if (node->unary_expr.op == MINUS && child_type != ARITHMETIC_EXPR) {
        ERROR("Error(line %i): Expression after unary \"-\" needs to be arithmetic but is %s.\n", node->line, child_type_str.c_str());
        return ExprError;
    }

    if (node->unary_expr.op == NOT && child_type != LOGICAL_EXPR) {
        ERROR("Error(line %i): Expression after unary \"!\" needs to be logical but is %s.\n", node->line, child_type_str.c_str());
        return ExprError;
    }

	if(writeOnlyCheck(node->unary_expr.right)){
		ERROR("Error: Reading from write only varibale\n");
	}



    // Type and class simply inherit from right expr.
    return (ExprEval) {.has_error = false, .expr_type = child_type, .base_type = node->unary_expr.right->ee.base_type, .class_size=node->unary_expr.right->ee.class_size};
}

int max(int a, int b) {
    return a > b ? a : b;
}

ExprEval evaluate_binary_expression(AstNode* node) {
    ExprEval left_ee = node->binary_expr.left->ee;
    ExprEval right_ee = node->binary_expr.right->ee;

    // Propagate any error in children right away.
    if (left_ee.has_error || right_ee.has_error) {
        return ExprError;
    }

    int op = node->binary_expr.op;
    int op_type = getOpType(op);


       if(writeOnlyCheck(node->binary_expr.left)){
                ERROR("Error: Reading from write only variable. Left operand is write only. Line %d\n",node->binary_expr.left->line);
                return ExprError;
        }

       if(writeOnlyCheck(node->binary_expr.right)){
                ERROR("Error: Reading from write only variable. Right operand is write only.Line %d\n",node->binary_expr.right->line);
                return ExprError;
        }



    // Case 1: Arithmetic.
    if (op_type == ARITHMETIC_OP) {
        // Arithemtic operands also need to be arithemetic.
        if (left_ee.expr_type != ARITHMETIC_EXPR ||
                right_ee.expr_type != ARITHMETIC_EXPR) {
            ERROR("Error(line %i): Both operands of arithmetic operator needs to be arithmetic. (left: %s, right: %s)\n", node->line, 
                    ExprNodeToTypeStr(node->binary_expr.left).c_str(), ExprNodeToTypeStr(node->binary_expr.right).c_str());
            return ExprError;
        }

        // Left & right base types need to be the same.
        if (left_ee.base_type != right_ee.base_type) {
            ERROR("Error(line %i): Operands of arithmetic operator needs to be of the same base type. (left base: %s, right base: %s)\n", node->line,
                    var_type_to_str(left_ee.base_type).c_str(), var_type_to_str(right_ee.base_type).c_str());
            return ExprError;
        }

        int base_type = left_ee.base_type;

        // Accept ss or vv;
        if (op == PLUS || op == MINUS) {
            if (left_ee.class_size != right_ee.class_size) {
                return ExprError;
            }

            return (ExprEval) {.has_error = false, .expr_type=ARITHMETIC_EXPR, .base_type=base_type, .class_size=left_ee.class_size};
        }

        // Accept ss, vv, vs or sv;
        if (op == MUL) {
            if (left_ee.class_size == 1 || right_ee.class_size == 1) {
                // One of the sub expr is a scalar, we accept the max 
                // class size regardless of what the other expr is.
                return (ExprEval) {false, ARITHMETIC_EXPR, base_type, max(left_ee.class_size, right_ee.class_size)};
            }

            // None of the class_sizes are 1. Means we have 2 vectors
            // and their sizes need to match in this case.
            if (left_ee.class_size != right_ee.class_size) {
                ERROR("Error(line %i): Operands of \"*\" are vectors of different sizes. (left size: %i, right size: %i)\n", node->line, left_ee.class_size, right_ee.class_size);
                return ExprError;
            }

            // Equal size vectors, inherit from left.
            return (ExprEval) {false, ARITHMETIC_EXPR, base_type, left_ee.class_size};
        }

        // Accept ss only.
        if (op == DIV || op == POWER) {
            if (left_ee.class_size != 1 || right_ee.class_size != 1) {
                ERROR("Error(line %i): Vectors are not accepted for \"/\" or \"^\".\n", node->line);
                return ExprError;
            }
            return (ExprEval) {false, ARITHMETIC_EXPR, base_type, 1};
        }

        // None reached.
        return ExprError;
    }

    // Case 2: Logical and binary.
    if (op_type == LOGICAL_OP) {
        // Must have boolean logical operands.
        if (left_ee.expr_type != LOGICAL_EXPR || right_ee.expr_type != LOGICAL_EXPR) {
            ERROR("Error(line %i): Operands of a logical operator needs to be of logical types. (left: %s, right: %s)\n", node->line,
                    ExprNodeToTypeStr(node->binary_expr.left).c_str(), ExprNodeToTypeStr(node->binary_expr.right).c_str());
            return ExprError;
        }

        // ss or vv
        if (left_ee.class_size != right_ee.class_size) {
            ERROR("Error(line %i): Operands of a logical operator needs to be scalar-scalar of vectors of same size. (left size: %i, right size: %i)\n", node->line,
                    left_ee.class_size, right_ee.class_size);
            return ExprError;
        }

        return (ExprEval) {false, LOGICAL_EXPR, BOOL_T, left_ee.class_size};      
    }

    // Case 3: Comparisons.
    if (op_type == COMPARISON_OP) {
        // All operands must be arithmetic.
        if (left_ee.expr_type != ARITHMETIC_EXPR || right_ee.expr_type != ARITHMETIC_EXPR) {
            ERROR("Error(line %i): Operands of a comparison operator needs to be arithmetic. (left %s, right %s)\n", node->line,
                    ExprNodeToTypeStr(node->binary_expr.left).c_str(), ExprNodeToTypeStr(node->binary_expr.right).c_str());
            return ExprError;
        }

        // Base types need to be the same.
        if (left_ee.base_type != right_ee.base_type) {
            ERROR("Error(line %i): Operands of a comparison operator needs to have the same arithmetic base type. (left base: %s, right base: %s)\n", node->line,
                    var_type_to_str(left_ee.base_type).c_str(), var_type_to_str(right_ee.base_type).c_str());
            return ExprError;
        }

        if (left_ee.class_size != right_ee.class_size) {
            ERROR("Error(line %i): Operands of a comparison operator needs to be scalar-scalar or vectors of same size. (left size: %i, right size: %i)\n", node->line,
                    left_ee.class_size, right_ee.class_size);
            return ExprError;
        }

        // ss or vv accepted.
        if (op == EQ || op == NE)
            return (ExprEval) {false, LOGICAL_EXPR, BOOL_T, left_ee.class_size};    

        // For other operators, only ss accepted.
        if (left_ee.class_size != 1) {
            ERROR("Error(line %i): Operands of comparison operator that is not \"==\" or \"!=\" needs to be scalar-scalar.\n", node->line);
            return ExprError;
        }

        return (ExprEval) {false, LOGICAL_EXPR, BOOL_T, 1};
    }

    return ExprError;
}

ExprEval typeToEE(int type) {
    ExprEval ee;
    ee.base_type = getBaseType(type);
    ee.expr_type = ee.base_type == BOOL_T ? LOGICAL_EXPR : ARITHMETIC_EXPR;
    ee.class_size = getClassSize(type);
    return ee;
}

int funcNameToRetType(char* func_name) {
    std::string name(func_name);
    if (name == "RSQ")
        return FLOAT_T;

    if (name == "DP3")
        return FLOAT_T;

    if (name == "LIT")
        return VEC4_T;

    return -1; // Error.
}

bool IsVector(int t) {
    return t == VEC2_T || t == VEC3_T || t == VEC4_T ||
        t == IVEC2_T || t == IVEC3_T || t == IVEC4_T ||
        t == BVEC2_T || t == BVEC3_T || t == BVEC4_T;
}

void CheckVectorIndex(int type_code, int index, int line){
    if (index < 0) {
        ERROR("Error(line %i): Index can not be < 0.\n", line);
        return;
    }

    int bound;

    if (type_code == VEC2_T || type_code == IVEC2_T || type_code == BVEC2_T)
        bound = 2;

    if (type_code == VEC3_T || type_code == IVEC3_T || type_code == BVEC3_T)
        bound = 3;

    if (type_code == VEC4_T || type_code == IVEC4_T || type_code == BVEC4_T)
        bound = 4;

    // Pass.
    if (index < bound)
        return;

    ERROR("Error(line %i): Index out of bound: %i\n", line, index);
}

int ExprEvalToType(ExprEval ee, int deref) {
    int base = ee.base_type;
    int size = ee.class_size;

    if (base == INT_T) {
        if (deref || size < 2)
            return INT_T;
        if (size == 2)
            return IVEC2_T;
        if (size == 3)
            return IVEC3_T;
        if (size == 4)
            return IVEC4_T;
    }

    if (base == FLOAT_T) {
        if (deref ||size < 2)
            return FLOAT_T;
        if (size == 2)
            return VEC2_T;
        if (size == 3)
            return VEC3_T;
        if (size == 4)
            return VEC4_T;
    }

    if (base == BOOL_T) {
        if (deref || size < 2)
            return BOOL_T;
        if (size == 2)
            return BVEC2_T;
        if (size == 3)
            return BVEC3_T;
        if (size == 4)
            return BVEC4_T;
    }

    return -1; // Error
}

int ExprNodeToType(AstNode* var_node) {
    int deref;
    if (var_node->kind != VAR_NODE)
        deref = 0;
    else
        deref = var_node->variable.deref;
    return ExprEvalToType(var_node->ee, deref);
}

void semantic_check_node(AstNode* node) {
    if (node == NULL) return;

    // Initialize expr check variables.
    node->ee.has_error = false;
    node->ee.expr_type = -1;
    node->ee.base_type = -1;
    node->ee.class_size = -1;

    int line = node->line;

    switch(node->kind) {
        case INT_NODE: // Needs to set ExprEval.
            node->ee = typeToEE(INT_T);
            break;

        case FLOAT_NODE: // Needs to set ExprEval.
            node->ee = typeToEE(FLOAT_T);
            break;

        case BOOL_NODE: // Needs to set ExprEval.
            node->ee = typeToEE(BOOL_T);
            break;

        case PROGRAM_NODE:
            break;

        case SCOPE_NODE:
            break;

        case DECLARATIONS_NODE:
            break;

        case STATEMENTS_NODE:
            break;

        case UNARY_EXPRESSION_NODE: // Needs to set ExprEval.
            node->ee = evaluate_unary_expression(node);
            break;

        case BINARY_EXPRESSION_NODE: // Needs to set ExprEval.
            node->ee = evaluate_binary_expression(node);
            break;

            // Rupan Start here
        case DECLARATION_NODE: {
                                   // Type coherence check.
                                   if (node->declaration.expression) {
                                       int type_lhs = node->declaration.type->type.type;
                                       int type_rhs = ExprNodeToType(node->declaration.expression);
                                       if (type_lhs != type_rhs)
                                           ERROR("Error(line %i): LHS and RHS of the declaration do not have equivalent types (%s != %s).\n", line, var_type_to_str(type_lhs).c_str(), var_type_to_str(type_rhs).c_str());
                                   }

                                   // Predefined/const checks.
                                   if( varnameCanBeDeclared(node->declaration.id)){
                                       bool initiated = (AstNode*)node->declaration.expression != (AstNode*)NULL;
                                       // Ask on piazza and decide const checks.
                                       addToSymbolTable(node->declaration.id, node->declaration.type->type.type, node->declaration.is_const, initiated);
                                   }
                                   else{
                                       ERROR("Error(line %i): redecleartion of variable %s in same scope\n", line, node->declaration.id);
                                   }
                                   break;
                               }

        case VAR_NODE: {// Needs to set ExprEval.
                           // Check 1: Variable exists on symbol table, fetch its type and populate ExprEval.
                           if (!doesVarExist(node->variable.id)) {
                               ERROR("Error(line %i): variable %s does not exist in symbol table.\n", line, node->variable.id);
                           } else {
                               node->variable.var_type = getVarType(node->variable.id);
                               node->ee = typeToEE(node->variable.var_type);
                               
                           }
                                if(node->variable.deref){
                                        node->ee.class_size = 1;
                                }


                           // Check 2: If variable is a vector, check that index does not go out of bound.
                           if (IsVector(node->variable.var_type)) {
                               CheckVectorIndex(node->variable.var_type, node->variable.index, line);

                           }

                           break;
                       }

        case TYPE_NODE:
                       break;

        case IF_STATEMENT_NODE:
			if(writeOnlyCheck(node->if_statement.condition)){
				ERROR("Error: Reading from write only variable. On line %d\n",node->if_statement.condition->line);
			}
                       if(node->if_statement.condition->ee.has_error)
                           break;
                       // We dont need this in nodes that do not reduce to expressions: ExprEval ee = node->if_statement.condition->ee;
                       if(!(node->if_statement.condition->ee.expr_type == LOGICAL_EXPR && node->if_statement.condition->ee.class_size == 1)){
                           ERROR("Error(line %i): expression in if statement is not a bool type\n", line);
                       }
                       break;

        case ASSIGNMENT_NODE:{
                                 AstNode * variableNode = node->assignment.variable;
                                 if(doesVarExist(variableNode->variable.id)) {
                                     if(!isReadOnly(variableNode->variable.id,variableNode->line)) {
                                         int lhs_type = ExprNodeToType(variableNode);
                                         int rhs_type = ExprNodeToType(node->assignment.expression);
					if(writeOnlyCheck(node->assignment.expression)){
	                	                ERROR("Error: Reading from write only variable. On line %d \n",node->assignment.expression->line);
        		                }

                                         if (lhs_type != rhs_type)
                                             ERROR("Error(line %i): LHS and RHS of the assignment do not have equivalent types (%s != %s).\n", line, var_type_to_str(lhs_type).c_str(), var_type_to_str(rhs_type).c_str());
                                         // Set initiated to true.
                                         if(!predefinedVarnameCheck(variableNode->variable.id))
                                             set_initiated(variableNode->variable.id);
                                     }
                                     

                                     // int varType = variableNode->type.type; 
                                 }
                                 else{
                                     ERROR("Error(line %i): In ASSIGNMENT_NODE variable %s does not exist in symbol table.\n", line, variableNode->variable.id);
                                 }
                                 break;

                             }
        case CONSTRUCTOR_NODE: {// Needs to set ExprEval.
                                   node->ee = typeToEE(node->constructor.type->type.type);
                                   int numArgsGiven = getNumArgs(node->constructor.arguments);
                                   int numNecessaryArgs = numArgsConstruct(node->constructor.type->type.type);
                                   if(numNecessaryArgs > 0 && numNecessaryArgs == numArgsGiven){
                                       if(argTypeCheck(getBaseType(node->constructor.type->type.type), node->function.arguments)){

                                           if(!argSizeCheck(1,node->function.arguments)){
                                               ERROR("Error(line %i): Improper arguments provided to constructor\n", line);
                                           }



                                       } 

                                       else{
                                           ERROR("Error(line %i): Improper arguments to constructor \n", line);
                                       }
                                   }

                                   else{
                                       ERROR("Error(line %i): Improper amount of arguments provided\n", line);
                                   }


                                   break;}

        case FUNCTION_NODE: {// Needs to set ExprEvalal.
                                int func_type = funcNameToRetType(node->function.name);
                                node->ee = typeToEE(func_type);
                                if(!strcmp(node->function.name,"RSQ")){
                                    int numArgs = getNumArgs(node->function.arguments);
                                    if(numArgs == 1){
                                        if(argTypeCheck(INT_T, node->function.arguments) || argTypeCheck(FLOAT_T, node->function.arguments)){
                                            if(!argSizeCheck(1,node->function.arguments)){
                                                ERROR("Error(line %i): Improper arguments to function RSQ\n", line);

                                            }
                                        }
                                        else{
                                            ERROR("Error(line %i): Improper arguments to function RSQ\n", line);
                                        }
                                    }
                                    else{
                                        ERROR("Error(line %i): Improper amount of arguments to function RSQ \n", line);
                                    }
                                }
                                else if(!strcmp(node->function.name,"DP3")){
                                    int numArgs = getNumArgs(node->function.arguments);
                                    if(numArgs == 2){
                                        if(argTypeCheck(INT_T, node->function.arguments) || argTypeCheck(FLOAT_T, node->function.arguments)){
                                            if(!argSizeCheck(3,node->function.arguments) && !argSizeCheck(4,node->function.arguments)){
                                                ERROR("Error(line %i): Improper arguments to function DP3\n", line);
                                            } 
                                        }
                                        else{
                                            ERROR("Error(line %i): Improper arguments to function DP3\n", line);
                                        }
                                    }
                                    else{
                                        ERROR("Error(line %i): Improper amount of arguments to function DP3\n", line);
                                    }
                                }
                                else if(!strcmp(node->function.name,"LIT")){
                                    node->ee.class_size = 4;
                                    int numArgs = getNumArgs(node->function.arguments);
                                    if(numArgs == 1){
                                        if(argTypeCheck(FLOAT_T, node->function.arguments)){

                                            if(!argSizeCheck(4,node->function.arguments)){
                                                ERROR("Error(line %i): Improper arguments to function LIT, argument vector size needs to be 4.\n", line);
                                            }
                                        }
                                        else{
                                            ERROR("Error(line %i): Improper arguments to function LIT, arugment base type incorrect.\n", line);
                                        }
                                    }
                                    else{
                                        ERROR("Error(line %i): Improper amount of arguments to function LIT, require 1 argument.\n", line);
                                    }
                                }

                                else{
                                    ERROR("Error(line %i): Invalid Function\n", line);
                                }
                                break;
                            }

        case ARGUMENTS_NODE:
                            break;

        default:
                            break;
    }

    return;
}

void semantic_check(AstNode* node) {
    ast_traverse_post(node, &semantic_check_node);
    return;
}

int typeCheck(int expectedType, int givenType){

    if(expectedType == givenType){
        return 1;
    } 

    return 0 ;
}



int getOperandType(int operand){


    if(operand == INT_T ||  operand == INT ||operand == FLOAT_T ||operand == FLOAT || operand == VEC2_T || operand == VEC3_T ||operand == VEC4_T || operand == IVEC2_T ||operand == IVEC3_T ||operand == IVEC4_T){
        return ARITHMETIC_EXPR;
    }


    if	(operand == BOOL_T ||operand == BOOL_TRUE ||operand == BOOL_FALSE  || operand == BVEC2_T ||operand == BVEC3_T ||operand == BVEC4_T){
        return LOGICAL_EXPR;
    }

    return -1;

}



int getBaseType(int operand){
    if(operand == INT_T ||  operand == INT || operand == IVEC2_T || operand == IVEC3_T || operand == IVEC4_T ){
        return INT_T;
    }

    if(operand == FLOAT_T ||  operand == FLOAT || operand == VEC2_T || operand == VEC3_T || operand == VEC4_T ){
        return FLOAT_T;
    }

    if(operand == BOOL_T ||  operand == BOOL_TRUE || operand == BOOL_FALSE || operand == BVEC2_T || operand == BVEC3_T || operand == BVEC4_T ){
        return BOOL_T;
    }
    return -1;

}


int getOpType(int op){

    if (op == PLUS || op == MINUS || op == MUL || op == DIV || op == POWER ) {
        return ARITHMETIC_OP;
    }

    if (op == LT || op == LE || op == EQ || op == NE || op == GT || op == GE ){
        return COMPARISON_OP;
    }

    if (op == AND || op == OR ){
        return LOGICAL_OP;
    }

    return -1;
}



int getOperandVector(int operand){

    if(operand == INT_T || operand == INT ||operand == FLOAT_T ||operand == FLOAT ||operand == BOOL_T ||operand == BOOL_TRUE ||operand == BOOL_FALSE ){
        return SCALAR;
    }

    if(operand == VEC2_T ||operand == VEC3_T ||operand == VEC4_T ||operand == BVEC2_T ||operand == BVEC3_T ||operand == BVEC4_T ||operand == IVEC2_T ||operand == IVEC3_T ||operand == IVEC4_T){
        return VECTOR;
    }
    return -1;

}





int checkOpAndOperandType(int op, int operand){

    int opType = getOpType(op);
    int operandType = getOperandType(operand);  
    if(opType == operandType ||  (opType == COMPARISON_OP && operandType == ARITHMETIC_EXPR) ){

        int operandVectorType = getOperandVector(operand);
        if ( operandVectorType == VECTOR && (op == DIV || op == POWER || op == LT || op == LE || op == GT || op == GE)){
            ERROR("\nOperand is a vector to a operator that only takes scalar\n"); 
            return 0;		
        }
        return 1;
    }
    else{
        ERROR("\nThe type of operation and operand is different\n");
        return 0;
    }
    return -1;
}


int checkBinaryOperandTypes(int loperandType, int roperandType){

    int lbaseType = getBaseType(loperandType);
    int rbaseType = getBaseType(roperandType);

    if(lbaseType == rbaseType){

        int lVectorType = getOperandVector(loperandType);
        int rVectorType = getOperandVector(roperandType);
        if(lVectorType == rVectorType && lVectorType == VECTOR){
            return loperandType == roperandType;
        }
        return 1;
    }
    else{
        ERROR("\nBase type does not match\n");
        return 0;
    }
}

int checkBinaryOperatorAllowance(int loperand, int operand, int roperand){
    int lVectorType = getOperandVector(loperand);
    int rVectorType = getOperandVector(roperand);

    if(operand != MUL){
        return lVectorType == rVectorType; 
    }

    return 1;
}

int getClassSize(int var_type) {
    if (var_type == VEC4_T || var_type == IVEC4_T || var_type == BVEC4_T)
        return 4;

    if (var_type == VEC3_T || var_type == IVEC3_T || var_type == BVEC3_T)
        return 3;

    if (var_type == VEC2_T || var_type == IVEC2_T || var_type == BVEC2_T)
        return 2;

    if (var_type == INT_T || var_type == FLOAT_T || var_type == BOOL_T)
        return 1;

    return 0; // Error.
}

int checkPredefinedVectorIndex(int indexValue, char * varname){

    if (indexValue < 0){
        ERROR("Inavlid indexi Value\n");
        return 0;
    }

    if(strcmp(varname,"gl_FragDepth")){
        return indexValue < 4;
    }
    else{
        return 1;
    }
}



