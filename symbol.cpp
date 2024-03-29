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

using namespace std;

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



int does_var_have_regname(char * var_name){

	if(predefinedVarnameCheck(var_name)){
		return 1;
	}

	else{
		int var_scope = getVarScope(var_name);
		varType thisVarType = symbolTable[var_scope][var_name];
		string reg_name = thisVarType.reg_name;
		if(reg_name.empty()){
			return 0;
		}
		return 1;
	}
}

const char * get_reg_name(char * var_name){
        if(predefinedVarnameCheck(var_name)){
		if (!strcmp(var_name,"gl_FragColor" )){ return "result.color";}
		if (!strcmp(var_name,"gl_FragDepth" )){ return "result.depth";}
		if (!strcmp(var_name,"gl_FragCoord" )){ return "fragment.position";}
		if (!strcmp(var_name,"gl_TexCoord" )){ return "fragment.texcoord";}
		if (!strcmp(var_name,"gl_Color" )){ return "fragment.color";}
		if (!strcmp(var_name,"gl_Secondary" )){ return "fragment.color.secondary";}
		if (!strcmp(var_name,"gl_FogFragCoord" )){ return "fragment.fogcoord";}
		if (!strcmp(var_name,"gl_Light_Half" )){ return "state.light[0].half";}
		if (!strcmp(var_name,"gl_Light_Ambient" )){ return "state.lightmodel.ambient";}
		if (!strcmp(var_name,"gl_Material_Shininess" )){ return "state.material.shininess";}
		if (!strcmp(var_name,"env1" )){ return "program.env[1]";}
		if (!strcmp(var_name,"env2" )){ return "program.env[2]";}
		if (!strcmp(var_name,"env3" )){ return "program.env[3]";}
		return "";
	}

	else{
		int var_scope = getVarScope(var_name);
                varType thisVarType = symbolTable[var_scope][var_name];
                string reg_name = thisVarType.reg_name;
		return reg_name.c_str();
	}
}


void assign_reg_name(char * var_name, string new_reg_name){
	                int var_scope = getVarScope(var_name);
			symbolTable[var_scope][var_name].reg_name = new_reg_name;
}


// Call whenever you enter a new scope  
extern "C" void addScope(){
    my_scope_count += 1; 
    std::tr1::unordered_map<string,varType> scopeTable;
    symbolTable.push_back(scopeTable);
}


// call whenever you exit a scope
extern "C" void subtractScope(){
    my_scope_count -= 1;
    symbolTable.pop_back();
}


// Finds out whether a varibale exists in the program
extern "C" int doesVarExist(char * varname){

    if(varname == NULL){
        return 0;
    }
    if(predefinedVarnameCheck(varname)){
        return 1;
    }
    int currScope = my_scope_count - 1;
    while(currScope >= 0 ){
        auto variableFound = symbolTable[currScope].find(varname);
        if ( variableFound !=  symbolTable[currScope].end()){
            return 1;                        
        }
        else {
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
    printf("Error: Scope is invalid it is %d \n", currScope); 
    return 0;
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


extern "C" bool  getConstType(char * varname){

    if(predefinedVarnameCheck(varname)){
        if(!strcmp(varname,"gl_Light_Half") || !strcmp(varname,"gl_Light_Ambient") ||!strcmp(varname,"gl_Material_Shininess") ||
                !strcmp(varname,"env1") || !strcmp(varname,"env2") || !strcmp(varname,"env3")){
            return 1;
        } 
        return 0;
    }
    int varibaleScope = getVarScope(varname);
    varType thisVarType = symbolTable[varibaleScope][varname];
    bool thisConst = thisVarType.isConst;
    return thisConst;
}


extern "C" void set_initiated(char* varname) {
    if (predefinedVarnameCheck(varname))
        return;
    int variableScope = getVarScope(varname);
    symbolTable[variableScope][varname].initiated = true;
}

extern "C" varType getVarStruct(char * varname){
    int varibaleScope = getVarScope(varname);
    varType thisVarType = symbolTable[varibaleScope][varname];
    return thisVarType;
}



// Provide varibale name first and all the types after. Can take more than one. Fomrat for variable argument list. Number of arguments you want to look at
// then the arguments. Make sure that the remaining number of arguments is atleast the number of variables you specified

extern "C" void addToSymbolTable(char * var_name , int given_varType , bool given_isConst , bool initiated){
    if (my_scope_count > 0){
        auto variableFound = symbolTable[my_scope_count -1].find(var_name);
        if (variableFound == symbolTable[my_scope_count - 1].end()){
            varType thisVarType = varType(given_varType,given_isConst,initiated);
            symbolTable[my_scope_count - 1][var_name] = thisVarType; 

        } 
        else{
            cout << endl << "ERROR : Redecleration of variable!" << endl;
        }
    }

    else{
        printf("Scope Count is less than zero\n");
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







