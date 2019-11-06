#include <stdio.h>
#include <stdlib.h>
#include<tr1/unordered_map>
#include <vector> 
#include <iostream>
#include <cstring>



using namespace std;


std::tr1::unordered_map<int, int> assass; 
vector<std::tr1::unordered_map<string,int>> symbolTable; 

int my_scope_count = 0;

extern "C"  void print_func(){ 
	assass[0] = my_scope_count;
        cout << endl << "The value is "<< assass[0] << endl;
        my_scope_count +=  1;
}


extern "C" void addScope(){

	my_scope_count += 1; 
        cout << endl << "Scope count is increasedn : " << my_scope_count << endl;  
}

extern "C" void subtractScope(){

        my_scope_count -= 1;
        cout << endl << "Scope count is decreased : " << my_scope_count << endl;
}




//umap[1] = 1

//std::cout << "YEEEE" << 1 << std::endl;






