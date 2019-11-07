#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>
#include <cstdarg>

using namespace std;

vector<std::tr1::unordered_map<string,vector<int>>> symbolTable; 

int my_scope_count = 0;

extern "C" void addScope(){
	my_scope_count += 1; 
	std::tr1::unordered_map<string,vector<int>> scopeTable;
	symbolTable.push_back(scopeTable);
        cout << endl << "Scope count is increased : " << my_scope_count << endl;  
}

extern "C" void subtractScope(){
        my_scope_count -= 1;
	symbolTable.pop_back();
        cout << endl << "Scope count is decreased : " << my_scope_count << endl;
}


// Provide varibale name first and all the types after. Can take more than one
extern "C" void addToSymbolTable(char * var_name , ... ){
	if (my_scope_count > 0){
		auto variableFound = symbolTable[my_scope_count -1].find(var_name);
		if (variableFound == symbolTable[my_scope_count - 1].end()){

			char * varName = var_name;
                        cout <<endl << "YOOOOOOOOOVar name is " << varName << endl;

                        vector <int> emptyVector; 
			symbolTable[my_scope_count - 1][varName] =  emptyVector;

			va_list args;
			va_start(args, var_name);

			while (*var_name != '\0') {
				int typeValue =  va_arg(args, int);
				cout << endl << "    The type is " << typeValue << endl;
				symbolTable[my_scope_count - 1][var_name].push_back(typeValue);
				++var_name;
			}
			va_end(args);

		} 
		else{
			cout << endl << "ERROR : Redecleration of variable!" << endl;
		}
	}
}


// This will find the user symbol from the symbol table. Doesn't return anything for now. Not sure what ast implementation will look like. 
extern "C" void findUsedSymbol(char * var_name){
	int currScope = my_scope_count - 1;
	int varFound  = 0;
	while(!varFound && currScope >= 0 ){
		auto variableFound = symbolTable[currScope].find(var_name);
		if ( variableFound !=  symbolTable[currScope].end()){
			varFound = 1;
			cout << endl << "Variable found name is " << var_name << " " <<endl; 

			for (int i = 0; i <  symbolTable[currScope][var_name].size(); i ++){
				cout << endl << "    Type is " +  symbolTable[currScope][var_name][i] << endl;
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








