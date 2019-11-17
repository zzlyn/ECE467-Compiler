#include "ast.h"
#include "symbol.h"
#include  "parser.tab.h"
#include <stdlib.h>
#include <stdio.h> 


#define ERROR  -1
#define ARITHMETIC  0
#define LOGICAL  1
#define COMPARISON  2
#define SCALAR 3
#define VECTOR  4

int checkVectorIndex(int indexValue, int operandType);
int checkPredefinedVectorIndex(int indexValue, char * varname);
int getOpType(int op);
int getBaseType(int op);
int getClassSize(int type);


ExprEval evaluate_unary_expression(AstNode* node) {
    // Check if op and child type match. Class does not matter since
    // unary accepts both vectors and scalars.
    int child_type = node->unary_expr.right->ee.expr_type;
    
    if (node->unary_expr.op == MINUS && child_type != ARITHMETIC_EXPR)
        return ExprError;
    if (node->unary_expr.op == NOT && child_type != LOGICAL_EXPR)
        return ExprError;

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
            return ExprError;
        }
        
        // Left & right base types need to be the same.
        if (left_ee.base_type != right_ee.base_type) {
            return ExprError;
        }
        
        int base_type = left_ee.base_type;

        // Accept ss or vv;
        if (op == PLUS || op == MINUS) {
            if (left_ee.class_size != right_ee.class_size) {
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
                return ExprError;
            }

            // Equal size vectors, inherit from left.
            return (ExprEval) {false, ARITHMETIC, base_type, left_ee.class_size};
        }

        // Accept ss only.
        if (op == DIV || op == POWER) {
            if (left_ee.class_size != 1 || right_ee.class_size != 1)
                return ExprError;
            return (ExprEval) {false, ARITHMETIC, base_type, 1};
        }

        // None reached.
        return ExprError;
    }

    // Case 2: Logical and binary.
    if (op_type == LOGICAL) {
        // Must have boolean logical operands.
        if (left_ee.expr_type != LOGICAL || right_ee.expr_type != LOGICAL)
            return ExprError;

        // ss or vv
        if (left_ee.class_size != right_ee.class_size) 
            return ExprError;

        return (ExprEval) {false, LOGICAL, BOOL_T, left_ee.class_size};      
    }

    // Case 3: Comparisons.
    if (op_type == COMPARISON) {
        // All operands must be arithmetic.
        if (left_ee.expr_type != ARITHMETIC_EXPR || right_ee.expr_type != ARITHMETIC_EXPR)
            return ExprError;
        
        // Base types need to be the same.
        if (left_ee.base_type != right_ee.base_type)
            return ExprError;
        
        if (left_ee.class_size != right_ee.class_size)
            return ExprError;

        // ss or vv accepted.
        if (op == EQ || op == NE)
            return (ExprEval) {false, LOGICAL, left_ee.base_type, left_ee.class_size};    
        
        // For other operators, only ss accepted.
        if (left_ee.class_size != 1)
            return ExprError;

        return (ExprEval) {false, LOGICAL, left_ee.base_type, 1};
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

        case BOOL_NODE:
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
			addToSymbolTable(node->declaration.id, node->declaration.type->type.type, node->declaration.is_const);
		}
		else{
			printf("\nError redecleartion of variable %s in same scope\n",node->declaration.id);
		}
            break;

        case VAR_NODE: {// Needs to set ExprEval.
            // LOok up on symbol table.
            if (!doesVarExist(node->variable.id)) {
                printf("Error: variable %s does not exist in symbol table.\n", node->variable.id);
            }
		else{
			node->ee = typeToEE(getVarType(node->variable.id));
		}
            break;
                       }

        case TYPE_NODE:
            break;

        case IF_STATEMENT_NODE:{
		ExprEval ee = node->if_statement.condition->ee;
		if(ee.expr_type != LOGICAL){
			printf("Error: expression in if statement is not a bool\n");
		}
		break;
	}

        case ASSIGNMENT_NODE:{
		AstNode * variableNode = node->assignment.variable;
                AstNode * exprNode = node->assignment.expression;
                ExprEval ee = exprNode->if_statement.condition->ee;
		
		if(!doesVarExist(variableNode->variable.id)) {
			int varType = variableNode->type.type; 

		}
		else{
			printf("Error: In ASSIGNMENT_NODE variable %s does not exist in symbol table.\n", variableNode->variable.id);
		}
		break;
	}
        case CONSTRUCTOR_NODE: // Needs to set ExprEval.
            node->ee = typeToEE(node->constructor.type->type.type);
            break;

        case FUNCTION_NODE: {// Needs to set ExprEvalal.
            int func_type = funcNameToRetType(node->function.name);
            node->ee = typeToEE(func_type);
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

	return ERROR;

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
        return ERROR;

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

	return ERROR;
}



int getOperandVector(int operand){

	if(operand == INT_T || operand == INT ||operand == FLOAT_T ||operand == FLOAT ||operand == BOOL_T ||operand == BOOL_TRUE ||operand == BOOL_FALSE ){
		return SCALAR;
	}

	if(operand == VEC2_T ||operand == VEC3_T ||operand == VEC4_T ||operand == BVEC2_T ||operand == BVEC3_T ||operand == BVEC4_T ||operand == IVEC2_T ||operand == IVEC3_T ||operand == IVEC4_T){
		return VECTOR;
	}
        return ERROR;

}





int checkOpAndOperandType(int op, int operand){

	int opType = getOpType(op);
	int operandType = getOperandType(operand);  
	if(opType == operandType ||  (opType == COMPARISON && operandType == ARITHMETIC) ){

		int operandVectorType = getOperandVector(operand);
		if ( operandVectorType == VECTOR && (op == DIV || op == POWER || op == LT || op == LE || op == GT || op == GE)){
			printf("\nOperand is a vector to a operator that only takes scalar\n"); 
			return 0;		
		}
		return 1;
	}
	else{
		printf("\nThe type of operation and operand is different\n");
		return 0;
	}
        return ERROR;
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
		printf("\nBase type does not match\n");
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
		printf("\nInavlid index Value\n");
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
	printf("\nThese is a issue yo\n");
	return 0;
}

int checkPredefinedVectorIndex(int indexValue, char * varname){

        if (indexValue < 0){
                printf("\nInavlid indexi Value\n");
		return 0;
        }

	if(strcmp(varname,"gl_FragDepth")){
		return indexValue < 4;
	}
	else{
		return 1;
	}
}



