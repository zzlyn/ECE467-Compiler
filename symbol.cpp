#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>

using namespace std;

vector<std::tr1::unordered_map<string,int>> symbolTable; 

int var_type = -1;
string var_name = "";

int my_scope_count = 0;

extern "C" void addScope(){
	my_scope_count += 1; 
	std::tr1::unordered_map<string,int> scopeTable;
	symbolTable.push_back(scopeTable);
        cout << endl << "Scope count is increased : " << my_scope_count << endl;  
}

extern "C" void subtractScope(){
        my_scope_count -= 1;
	symbolTable.pop_back();
        cout << endl << "Scope count is decreased : " << my_scope_count << endl;
}


extern "C" void updateVarType(int new_var_type){
	var_type = new_var_type;
}

extern "C" void updateVarName ( char * newVarName){
	var_name = newVarName;
}

extern "C" void printVarAndScope(){
	cout << endl << "The var name is " << var_name << " the scope is " << my_scope_count - 1<< endl;
}

extern "C" void addToSymbolTable(){
	if (my_scope_count > 0){
		auto variableFound = symbolTable[my_scope_count -1].find(var_name);
		if (variableFound == symbolTable[my_scope_count - 1].end()){
			cout << endl << "Adding new variable. Var name is " << var_name << " type is " << var_type << " scope is " << my_scope_count - 1 << endl;
			symbolTable[my_scope_count - 1][var_name] = var_type;
		} 
		else{
			cout << endl << "ERROR : Redecleration of variable!" << endl;
		}
	}
}

extern "C" void findUsedSymbol(){
	int currScope = my_scope_count - 1;
	int varFound  = 0;
	while(!varFound && currScope >= 0 ){
		auto variableFound = symbolTable[currScope].find(var_name);
		if ( variableFound !=  symbolTable[currScope].end()){
			varFound = 1;
			cout << endl << "Variable found name is " << var_name << " is in scope " << currScope <<  ". Type is " << symbolTable[currScope][var_name] << endl;
		}
		else {
			currScope-=1;
		}
	} 
	if (!varFound){
		cout << endl << "Error : Variable  " << var_name << " is not declared !" << endl;
	}
}


//umap[1] = 1

//std::cout << "YEEEE" << 1 << std::endl;






