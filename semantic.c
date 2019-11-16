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


void semantic_check_node(AstNode* node) {
    switch(node->kind) {
        case INT_NODE:
            break;
            
        case FLOAT_NODE:
            break;

        case PROGRAM_NODE:
            break;
            
        case SCOPE_NODE:
            break;

        case DECLARATIONS_NODE:
            break;

        case STATEMENTS_NODE:
            break;

        case UNARY_EXPRESSION_NODE:
            break;

        case BINARY_EXPRESSION_NODE:
            break;

// Rupan Start here
        case DECLARATION_NODE:
            printf("semantic_check: declaration node, inserting variable %s\n", node->declaration.id);
            break;

        case VAR_NODE:
        	printf("SWAAAAAAGGGGGGvar, id %s, index %i\n", node->variable.id, node->variable.index);
        	printf("semantic_check: var node, looking up variable %s\n", node->variable.id);
		if( doesVarExist(node->variable.id)){
			if(node->variable.index != -1){
                                AstNode * typeNode = node->variable.type;

				if(predefinedVarnameCheck(node->variable.id)){
					if(!checkPredefinedVectorIndex(node->variable.index,node->variable.id)){
						printf("\nError incorrect Indexing\n");
					}
				}

				else{
					if(!checkVectorIndex(node->variable.index, typeNode->type.type)){
                                                printf("\nError Incorrect indexing\n");
					}
				}
			}
		}
		else{
			printf("\nError doesn't exist yet\n");
		}
		printf("\nDone Checking\n");
            break;

        case TYPE_NODE:
            break;

        case IF_STATEMENT_NODE:
            break;

        case ASSIGNMENT_NODE:
            break;

        case CONSTRUCTOR_NODE:
            break;

        case FUNCTION_NODE:
            break;

        case ARGUMENTS_NODE:
            break;

        default:
            break;
    }

    return;
}

void semantic_check(AstNode* ast) {
    ast_traverse_post(ast, &semantic_check_node);
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



