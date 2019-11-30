
#ifndef AST_H_
#define AST_H_ 1

#include <stdarg.h>
#include <string>

// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
typedef struct node_ node;
typedef struct node_ AstNode;
extern node *ast;

extern bool in_variable_assign;
extern bool in_ifelse_scope;

typedef enum {
    UNKNOWN               = 0,

    PROGRAM_NODE          = (1 << 18),

    SCOPE_NODE            = (1 << 0),

    EXPRESSION_NODE       = (1 << 2),
    UNARY_EXPRESSION_NODE = (1 << 2) | (1 << 3),
    BINARY_EXPRESSION_NODE= (1 << 2) | (1 << 4),
    INT_NODE              = (1 << 2) | (1 << 5), 
    FLOAT_NODE            = (1 << 2) | (1 << 6),
    BOOL_NODE             = (1 << 2) | (1 << 16),
    IDENT_NODE            = (1 << 2) | (1 << 7),
    VAR_NODE              = (1 << 2) | (1 << 8),
    FUNCTION_NODE         = (1 << 2) | (1 << 9),
    CONSTRUCTOR_NODE      = (1 << 2) | (1 << 10),
    TYPE_NODE             = (1 << 2) | (1 << 11),

    STATEMENT_NODE        = (1 << 1),
    STATEMENTS_NODE       = (1 << 1) | (1 << 15),
    IF_STATEMENT_NODE     = (1 << 1) | (1 << 11),
    WHILE_STATEMENT_NODE  = (1 << 1) | (1 << 12),
    ASSIGNMENT_NODE       = (1 << 1) | (1 << 13),
    NESTED_SCOPE_NODE     = (1 << 1) | (1 << 14),

    DECLARATION_NODE      = (1 << 15),
    DECLARATIONS_NODE     = (1 << 16),

    ARGUMENTS_NODE        = (1 << 17),
} node_kind;

#define ARITHMETIC_EXPR 111
#define LOGICAL_EXPR 222

typedef struct ExpressionEvaluation{
    bool has_error;
    int expr_type; // ARITHMETIC/LOGIC
    int base_type; // INT_T/FLOAT_T/BOOL_T
    int class_size; // 1 for SCALAR and >1 for VECTOR types.
} ExprEval;


typedef struct assembly_argument{
	char * reg_name;
	char * index;
	struct assembly_argument * next_arg;
} assembly_arg;


#define ExprError (ExprEval) {.has_error = true, -1, -1}

struct node_ {

    // For semantic checking of expression productions.
    ExprEval ee;

    // an example of tagging each node with a type
    node_kind kind;

    // Line number.
    int line;


    // Instructions for assembly stuff
    char * instruction = NULL;	
    assembly_arg * assembly_args = NULL;

    union {

        struct {
            AstNode* scope;
        } program;

        struct {
            AstNode* declarations;
            AstNode* statements;
        } scope;

        struct {
            int op;
            AstNode *right;
        } unary_expr;

        struct {
            int op;
            AstNode* left;
            AstNode* right;
        } binary_expr;

        struct {
            int val;
        } integer;

        struct {
            float val;
        } float_num;

        struct {
            bool val;
        } boolean;

        // declarations, statements, variable, if_statement, assignment, constructor, arguments, function
        struct {
            AstNode* declarations;
            AstNode* declaration;
        } declarations;

        struct {
            int type; // Refer to parser.y type tokens.
            char* to_str; // String representation.
        } type;

        struct {
            bool is_const;
            AstNode* type; // Refer to parser.y.
            char* id;
            AstNode* expression;
        } declaration;

        struct {
            AstNode* statements;
            AstNode* statement;
        } statements;

        struct {
            bool is_const;
            int var_type; // Assigned during semantic check.
            char* id;
            int index; // Dereference index, set to 0 if id is stand alone.
            bool deref;
        } variable;

        struct {
            AstNode* condition;
            AstNode* statement;
            AstNode* else_statement;
        } if_statement;

        struct {
            AstNode* variable; // Contains type info.
            AstNode* expression;
        } assignment;

        struct {
            AstNode* type; // Refer to parser.y.
            AstNode* arguments;
        } constructor;

        struct {
            char* name;
            AstNode* arguments;
        } function;

        struct {
            AstNode* arguments;
            AstNode* expression;
        } arguments;

    };
};


typedef void(*NodeFunc)(AstNode*);
node *ast_allocate(node_kind type, ...);
void ast_traverse_post(AstNode* root, NodeFunc post_call);
void ast_free(node *ast);
void ast_print(node * ast);

std::string var_type_to_str(int type);

#endif /* AST_H_ */
