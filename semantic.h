#include "ast.h"
#include "symbol.h"

// Check top-down the entire AST tree.
bool semantic_check(AstNode* ast);

// Check a specific AST node, also responsible for 
// symbol creation & lookup.
bool semantic_check_node(AstNode* node);

int ExprNodeToType(AstNode* node);
