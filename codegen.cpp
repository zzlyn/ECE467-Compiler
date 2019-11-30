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
#include "codegen.h"



static const std::string ASB_FILE_NAME = "asb.txt";

extern "C" void genCode(AstNode* ast) {
    printf("genCode start\n");    
}
