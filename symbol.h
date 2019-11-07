#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>


#ifndef SCOPE_CPP
#define SCOPE_CPP


extern "C"  void addScope();
extern "C"  void subtractScope();



extern "C" void findUsedSymbol(char * var_name);

extern "C" void addToSymbolTable(char * var_name , ... );

#endif





