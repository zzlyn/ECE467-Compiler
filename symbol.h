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
        bool initiated;
	std::string reg_name;
        varType(int given_type, bool given_const, bool init){typeOfVariable = given_type; isConst = given_const; initiated = init; reg_name = "";}
        varType(){typeOfVariable = -1; isConst = false; initiated = false;}

};


const char * get_reg_name(char * var_name);
int does_var_have_regname(char * var_name);
void assign_reg_name(char * var_name, std::string new_reg_name); 



// Call this when you enter a new scope
extern "C"  void addScope();

// Call this when you exit a scope
extern "C"  void subtractScope();

extern "C" void set_initiated(char* var_name);

// Call this when you add a new symbol. Arguments are variable name and types. Types are integers
extern "C" void addToSymbolTable(char * var_name , int given_varType , bool given_isConst , bool initiated);

//  Call this to check if a var exists. 
extern "C" int doesVarExist(char * varname);

//  Call this to check if a var can be declared.
extern "C" int varnameCanBeDeclared(char * varname);

extern "C" int predefinedVarnameCheck(char * varname);


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





