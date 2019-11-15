#include "ast.h"

#include  "parser.tab.h"
#include <stdlib.h>
#include <stdio.h> 


#define ERROR  -1
#define ARITHMETIC  0
#define LOGICAL  1
#define COMPARISON  2
#define SCALAR 3
#define VECTOR  4





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
		
	}
	else{
		printf("\nThe type of operation and operand is different\n");
		return 0;
	}
        return ERROR;

}





