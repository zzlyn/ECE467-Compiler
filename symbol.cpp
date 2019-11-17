#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>
#include <cstdarg>
#include "ast.h"
#include "parser.tab.h"
using namespace std;



class varType {
	public :  
		int typeOfVariable;
		bool isConst;
        	varType(int given_type, bool given_const){typeOfVariable = given_type; isConst = given_const; }
                varType(){typeOfVariable = -1; isConst = false;}

};




extern "C" int predefinedVarnameCheck(char * varname){

	if(!strcmp(varname,"gl_FragColor") || !strcmp(varname,"gl_FragDepth") ||!strcmp(varname,"gl_FragCoord") ||!strcmp(varname,"gl_TexCoord") ){
		return 1;
	} 

	if(!strcmp(varname,"gl_Color") || !strcmp(varname,"gl_Secondary") ||!strcmp(varname,"gl_FogFragCoord") || !strcmp(varname,"gl_Light_Half") ){
                return 1;
        }

	if(!strcmp(varname,"gl_Light_Ambient") || !strcmp(varname,"gl_Material_Shininess") || !strcmp(varname,"env1") ||!strcmp(varname,"env2") || !strcmp(varname,"env3")){
                return 1;
        }

	return 0;
}


vector<std::tr1::unordered_map<string,varType>> symbolTable;
extern "C" int getVarScope(char * varname);

int my_scope_count = 0;



// Call whenever you enter a new scope  
extern "C" void addScope(){
	my_scope_count += 1; 
	std::tr1::unordered_map<string,varType> scopeTable;
	symbolTable.push_back(scopeTable);
        cout << endl << "Scope count is increased : " << my_scope_count << endl;  
}


// call whenever you exit a scope
extern "C" void subtractScope(){
        my_scope_count -= 1;
	symbolTable.pop_back();
        cout << endl << "Scope count is decreased : " << my_scope_count << endl;
}


// Finds out whether a varibale exists in the program
extern "C" int doesVarExist(char * varname){
	printf("hi - %s\n", varname);
    if(predefinedVarnameCheck(varname)){
		return 1;
	}
	int currScope = my_scope_count - 1;
        while(currScope >= 0 ){
		printf("In while loop\n");
                auto variableFound = symbolTable[currScope].find(varname);
		printf("Loooking for\n");
                if ( variableFound !=  symbolTable[currScope].end()){
			printf("Found Variable\n");
                	return 1;                        
                }
                else {
			printf("Checking previous scope\n");
                        currScope-=1;
                }
        }
	return 0;
}


extern "C" int varnameCanBeDeclared(char * varname){
	int currScope = my_scope_count - 1;
	if(predefinedVarnameCheck(varname)){
		return 0;
	}
	if(currScope >=0){
                auto variableFound = symbolTable[currScope].find(varname);	
		if ( variableFound !=  symbolTable[currScope].end()){
			// Varname exists in current scope already
                        return 0;
                }
		else{
			return 1;
		}
	} 
}



// Returns which scope the specific varibale name is declared in   
extern "C" int getVarScope(char * varname){
 int currScope = my_scope_count - 1;
        int varFound  = 0; 
        while(!varFound && currScope >= 0 ){
                auto variableFound = symbolTable[currScope].find(varname);
                if ( variableFound !=  symbolTable[currScope].end()){
                        return currScope; 
                }
                else {
                        currScope-=1;
                }
        }
        return -1;
}


// Assumes that the varibale name exists somewhere
extern "C" int  varTypeMatch(char * varname, int varType){

	int varibaleScope = getVarScope(varname);
        auto  thisVarType = symbolTable[varibaleScope][varname];
        int thisType = thisVarType.typeOfVariable;
	if(thisType == varType){
		return 1;
	}
	return 0; 
}



extern "C" int  getVarType(char * varname){

	if(predefinedVarnameCheck(varname)){
		if(!strcmp(varname,"gl_FragDepth")){
			return BOOL_T;
		}	
		return VEC4_T; 
	}


        int varibaleScope = getVarScope(varname);
        auto thisVarType = symbolTable[varibaleScope][varname];
        int thisType = thisVarType.typeOfVariable;
        return thisType;
}


extern "C" int  getConstType(char * varname){
        int varibaleScope = getVarScope(varname);
        varType thisVarType = symbolTable[varibaleScope][varname];
        bool thisConst = thisVarType.isConst;
        return thisConst;
}


extern "C" varType getVarStruct(char * varname){
        int varibaleScope = getVarScope(varname);
        varType thisVarType = symbolTable[varibaleScope][varname];
        return thisVarType;
}



// Provide varibale name first and all the types after. Can take more than one. Fomrat for variable argument list. Number of arguments you want to look at
// then the arguments. Make sure that the remaining number of arguments is atleast the number of variables you specified

extern "C" void addToSymbolTable(char * var_name , int given_varType , bool given_isConst ){
	if (my_scope_count > 0){
		auto variableFound = symbolTable[my_scope_count -1].find(var_name);
		if (variableFound == symbolTable[my_scope_count - 1].end()){
			varType thisVarType = varType(given_varType,given_isConst);
                        symbolTable[my_scope_count - 1][var_name] = thisVarType; 

		} 
		else{
			cout << endl << "ERROR : Redecleration of variable!" << endl;
		}
	}
}


// This will find the user symbol from the symbol table. Doesn't return anything for now. Not sure what ast implementation will look like. 
/*
extern "C" void findUsedSymbol(char * var_name){
	int currScope = my_scope_count - 1;
	int varFound  = 0;
	while(!varFound && currScope >= 0 ){
		auto variableFound = symbolTable[currScope].find(var_name);
		if ( variableFound !=  symbolTable[currScope].end()){
			varFound = 1;
			//cout << endl << "In findUsedSymbol Variable found name is " << var_name << " " <<endl; 
			int numTypes =  symbolTable[currScope][var_name].size();
			//cout << endl << "Num types is " << numTypes << endl;
			for (int i = 0; i <  numTypes ; i ++){
				//cout << endl << "    Type is " <<  symbolTable[currScope][var_name][i] << endl;
			}

		}
		else {
			currScope-=1;
		}
	} 
	if (!varFound){
		cout << endl << "Error : Variable  " << var_name << " is not declared !" << endl;
	}
}
*/







