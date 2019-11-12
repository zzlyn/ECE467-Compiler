#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>


#ifndef SCOPE_CPP
#define SCOPE_CPP


class varType {
        public :
                int typeOfVariable;
                bool isConst;
                varType(int given_type, bool given_const){typeOfVariable = given_type; isConst = given_const; }
                varType(){typeOfVariable = -1; isConst = false;}

};



// Call this when you enter a new scope
extern "C"  void addScope();

// Call this when you exit a scope
extern "C"  void subtractScope();

// Call this when you add a new symbol. Arguments are variable name and types. Types are integers
extern "C" void addToSymbolTable(char * var_name , int given_varType , bool given_isConst );

//  Call this to check if a var exists. 
extern "C" int doesVarExist(char * varname);




// The following functions assume the variable name exists

// Call this to check if variable type matches or not
extern "C" int  varTypeMatch(char * varname, int varType);

// Call this to get the varibale type of some var name
extern "C" int  getVarType(char * varname);

// Call this to get if a variable is constant or not
extern "C" bool  getConstType(char * varname);

// Call this function to get the struct vartype
extern "C" varType getVarStruct(char * varname);


#endif





