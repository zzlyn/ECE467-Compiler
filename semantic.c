#include "ast.h"
#include "symbol.h"
#include  "parser.tab.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h> 

#define ARITHMETIC  0
#define LOGICAL  1
#define COMPARISON  2
#define SCALAR 3
#define VECTOR  4

#define FIRST_ERROR_ONLY true

#define ERROR(...) if(!(FIRST_ERROR_ONLY && errorOccurred)) { fprintf(errorFile, __VA_ARGS__); errorOccurred = 1; }

int checkVectorIndex(int indexValue, int operandType);
int checkPredefinedVectorIndex(int indexValue, char * varname);
int getOpType(int op);
int getBaseType(int op);
int getClassSize(int type);


int isReadOnly(char * varname){

	if(predefinedVarnameCheck(varname)){
		if(!strcmp(varname,"gl_FragColor") || !strcmp(varname,"gl_FragDepth")){
			return 0;
		}
		return 1;
	}

	if(doesVarExist(varname)){
		int isConst = getConstType(varname);
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
                //printf("Type to check is %d . Arg type is %d\n",typeToCheck,ee.base_type);

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
		//printf("Variable index is %d\n", node->arguments.expression->variable.index);
		if(node->arguments.expression->variable.deref){
			classSize = 1;
		}
		//printf("Given Size is %d. Calculated size is %d\n",sizeToCheck, classSize);
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


ExprEval evaluate_unary_expression(AstNode* node) {
    // Check if op and child type match. Class does not matter since
    // unary accepts both vectors and scalars.
    int child_type = node->unary_expr.right->ee.expr_type;
    
    if (node->unary_expr.op == MINUS && child_type != ARITHMETIC_EXPR) {
        ERROR("Error: Expression after unary \"-\" needs to be arithmetic.\n");
        return ExprError;
    }

    if (node->unary_expr.op == NOT && child_type != LOGICAL_EXPR) {
        ERROR("Error: Expression after unary \"!\" needs to be logical.\n");
        return ExprError;
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

    // Case 1: Arithmetic.
    if (op_type == ARITHMETIC) {
        // Arithemtic operands also need to be arithemetic.
        if (left_ee.expr_type != ARITHMETIC ||
            right_ee.expr_type != ARITHMETIC) {
            ERROR("Error: Both operands of arithmetic operator needs to be arithmetic.\n");
            return ExprError;
        }
        
        // Left & right base types need to be the same.
        if (left_ee.base_type != right_ee.base_type) {
            ERROR("Error: Operands of arithmetic operator needs to be of the same base type.\n");
            return ExprError;
        }
        
        int base_type = left_ee.base_type;

        // Accept ss or vv;
        if (op == PLUS || op == MINUS) {
            if (left_ee.class_size != right_ee.class_size) {
                ERROR("Error: Operands of \"+\" or \"-\" needs to be scalar-scalar or vectors of same size.\n");
                return ExprError;
            }

            return (ExprEval) {.has_error = false, .expr_type=ARITHMETIC, .base_type=base_type, .class_size=left_ee.class_size};
        }

        // Accept ss, vv, vs or sv;
        if (op == MUL) {
            if (left_ee.class_size == 1 || right_ee.class_size == 1) {
                // One of the sub expr is a scalar, we accept the max 
                // class size regardless of what the other expr is.
                return (ExprEval) {false, ARITHMETIC, base_type, max(left_ee.class_size, right_ee.class_size)};
            }
            
            // None of the class_sizes are 1. Means we have 2 vectors
            // and their sizes need to match in this case.
            if (left_ee.class_size != right_ee.class_size) {
                ERROR("Error: Operands of \"*\" are vectors of different sizes.\n");
                return ExprError;
            }

            // Equal size vectors, inherit from left.
            return (ExprEval) {false, ARITHMETIC, base_type, left_ee.class_size};
        }

        // Accept ss only.
        if (op == DIV || op == POWER) {
            if (left_ee.class_size != 1 || right_ee.class_size != 1) {
                ERROR("Error: Vectors are not accepted for \"/\" or \"^\".\n");
                return ExprError;
            }
            return (ExprEval) {false, ARITHMETIC, base_type, 1};
        }

        // None reached.
        return ExprError;
    }

    // Case 2: Logical and binary.
    if (op_type == LOGICAL) {
        // Must have boolean logical operands.
        if (left_ee.expr_type != LOGICAL || right_ee.expr_type != LOGICAL) {
            ERROR("Error: Operands of a logical operator needs to be of logical types.\n");
            return ExprError;
        }

        // ss or vv
        if (left_ee.class_size != right_ee.class_size) {
            ERROR("Error: Operands of a logical operator needs to be scalar-scalar of vectors of same size\n");
            return ExprError;
        }

        return (ExprEval) {false, LOGICAL, BOOL_T, left_ee.class_size};      
    }

    // Case 3: Comparisons.
    if (op_type == COMPARISON) {
        // All operands must be arithmetic.
        if (left_ee.expr_type != ARITHMETIC_EXPR || right_ee.expr_type != ARITHMETIC_EXPR) {
            ERROR("Error: Operands of a comparison operator needs to be arithmetic.\n");
            return ExprError;
        }
        
        // Base types need to be the same.
        if (left_ee.base_type != right_ee.base_type) {
            ERROR("Error: Operands of a comparison operator needs to have the same arithmetic base type.\n");
            return ExprError;
        }
        
        if (left_ee.class_size != right_ee.class_size) {
            ERROR("Error: Operands of a comparison operator needs to be scalar-scalar or vectors of same size.\n");
            return ExprError;
        }

        // ss or vv accepted.
        if (op == EQ || op == NE)
            return (ExprEval) {false, LOGICAL, BOOL_T, left_ee.class_size};    
        
        // For other operators, only ss accepted.
        if (left_ee.class_size != 1) {
            ERROR("Error: Operands of comparison operator that is not \"==\" or \"!=\" needs to be scalar-scalar.\n");
            return ExprError;
        }

        return (ExprEval) {false, LOGICAL, BOOL_T, 1};
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
    
    if (name == "rsq")
        return FLOAT_T;
    
    if (name == "dp3")
        return FLOAT_T;

    if (name == "lit")
        return VEC4_T;

    return -1; // Error.
}

bool IsVector(int t) {
    return t == VEC2_T || t == VEC3_T || t == VEC4_T ||
           t == IVEC2_T || t == IVEC3_T || t == IVEC4_T ||
           t == BVEC2_T || t == BVEC3_T || t == BVEC4_T;
}

void CheckVectorIndex(int type_code, int index) {
    if (index < 0) {
        ERROR("Error: Index can not be < 0.\n");
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

    ERROR("Error: Index out of bound: %i\n", index);
}

void semantic_check_node(AstNode* node) {
    if (node == NULL) return;
    
    // Initialize expr check variables.
    node->ee.has_error = false;
    node->ee.expr_type = -1;
    node->ee.base_type = -1;
    node->ee.class_size = -1;

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
        case DECLARATION_NODE:
		if( varnameCanBeDeclared(node->declaration.id)){
            bool initiated = (AstNode*)node->declaration.expression != (AstNode*)NULL;
            // Ask on piazza and decide const checks.
			addToSymbolTable(node->declaration.id, node->declaration.type->type.type, node->declaration.is_const, initiated);
		}
		else{
			ERROR("\nError redecleartion of variable %s in same scope\n",node->declaration.id);
		}
            break;

        case VAR_NODE: {// Needs to set ExprEval.
            // Check 1: Variable exists on symbol table, fetch its type and populate ExprEval.
            if (!doesVarExist(node->variable.id)) {
                ERROR("Error: variable %s does not exist in symbol table.\n", node->variable.id);
            } else {
                node->variable.var_type = getVarType(node->variable.id);
                node->ee = typeToEE(node->variable.var_type);
	    	}

            // Check 2: If variable is a vector, check that index does not go out of bound.
            if (IsVector(node->variable.var_type)) {
                CheckVectorIndex(node->variable.var_type, node->variable.index);
            }

            break;
                       }

        case TYPE_NODE:
            break;

        case IF_STATEMENT_NODE:
		// We dont need this in nodes that do not reduce to expressions: ExprEval ee = node->if_statement.condition->ee;
		if(node->if_statement.condition->ee.expr_type != LOGICAL){
		    ERROR("Error: expression in if statement is not a bool type\n");
		}
		break;

        case ASSIGNMENT_NODE:{
		AstNode * variableNode = node->assignment.variable;
		if(doesVarExist(variableNode->variable.id)) {
			if(!isReadOnly(variableNode->variable.id)) {
                // Set initiated to true.
                if(!predefinedVarnameCheck(variableNode->variable.id))
                    set_initiated(variableNode->variable.id);
			}
			else{
				ERROR("Error: Assigning a value to a read-only variable\n");
			}

			// int varType = variableNode->type.type; 
		}
		else{
			ERROR("Error: In ASSIGNMENT_NODE variable %s does not exist in symbol table.\n", variableNode->variable.id);
		}
		break;

	}
        case CONSTRUCTOR_NODE: {// Needs to set ExprEval.
		node->ee = typeToEE(node->constructor.type->type.type);
		int numArgsGiven = getNumArgs(node->constructor.arguments);
		int numNecessaryArgs = numArgsConstruct(node->constructor.type->type.type);
		//printf("COnstrucitng node\n");
		if(numNecessaryArgs > 0 && numNecessaryArgs == numArgsGiven){
			if(argTypeCheck(getBaseType(node->constructor.type->type.type), node->function.arguments)){

			                        if(!argSizeCheck(1,node->function.arguments)){
			                                ERROR("Error: Improper arguments provided to constructor\n");
						}


				
			} 

			else{
				ERROR("Error: Improper arguments to constructor \n");
			}
		}

		else{
			ERROR("Error: Improper amount of arguments provided\n");
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
						ERROR("Error: Improper arguments to function RSQ\n");

					}
				}
				else{
					ERROR("Error: Improper arguments to function RSQ\n");
				}
                        }
                        else{
                                ERROR("Error: Improper amount of arguments to function RSQ \n");
                        }
		}
               else if(!strcmp(node->function.name,"DP3")){
			int numArgs = getNumArgs(node->function.arguments);
                        if(numArgs == 2){
                                if(argTypeCheck(INT_T, node->function.arguments) || argTypeCheck(FLOAT_T, node->function.arguments)){
					if(!argSizeCheck(3,node->function.arguments) && !argSizeCheck(4,node->function.arguments)){
	                                        ERROR("Error: Improper arguments to function DP3\n");
					} 
                                }
                                else{
                                        ERROR("Error: Improper arguments to function DP3\n");
                                }
                        }
                        else{
                                ERROR("Error: Improper amount of arguments to function DP3\n");
                        }
                }
               else if(!strcmp(node->function.name,"LIT")){

			int numArgs = getNumArgs(node->function.arguments);
			if(numArgs == 1){
				if(argTypeCheck(FLOAT_T, node->function.arguments)){

                                        if(!argSizeCheck(4,node->function.arguments)){
                                                ERROR("Error: Improper arguments to function LIT\n");
					}
				}
				else{
                                	ERROR("Error: Improper arguments to function LIT\n");
				}
			}
			else{
				ERROR("Error: Improper amount of arguments to function LIT \n");
			}
                }

		else{
			ERROR("Error: Invalid Function\n");
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
		return ARITHMETIC;
	}


	if	(operand == BOOL_T ||operand == BOOL_TRUE ||operand == BOOL_FALSE  || operand == BVEC2_T ||operand == BVEC3_T ||operand == BVEC4_T){
		return LOGICAL;
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
		return ARITHMETIC;
	}

	if (op == LT || op == LE || op == EQ || op == NE || op == GT || op == GE ){
                return COMPARISON;
        }

	if (op == NOT || op == AND || op == OR ){
                return LOGICAL;
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
	if(opType == operandType ||  (opType == COMPARISON && operandType == ARITHMETIC) ){

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

int checkVectorIndex(int indexValue, int operandType){

	if (indexValue < 0){
		ERROR("\nInavlid index Value\n");
		return 0;
	}


	if(operandType == VEC4_T || operandType == IVEC4_T ||operandType == BVEC4_T ){
		return indexValue < 4;
	}

	if(operandType == VEC3_T || operandType == IVEC3_T ||operandType == BVEC3_T ){
                return indexValue < 3;
        }

	if(operandType == VEC2_T || operandType == IVEC2_T ||operandType == BVEC2_T ){
                return indexValue < 2;
        }
	ERROR("\nThese is a issue yo\n");
	return 0;
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



