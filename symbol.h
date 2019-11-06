#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>


#ifndef SCOPE_CPP
#define SCOPE_CPP


extern "C"  void print_func();
extern "C"  void addScope();
extern "C"  void subtractScope();


extern "C" void updateVarType(int new_var_type);
extern "C" void updateVarName ( char * newVarName);
extern "C" void printVarAndScope();
extern "C" void addToSymbolTable();
extern "C" void updateVarType(int new_var_type);
extern "C" void findUsedSymbol();



#endif





