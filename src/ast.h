#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_COMPLEX, TYPE_CHAR,
    TYPE_STRING, TYPE_VOID, TYPE_UNDEFINED, TYPE_ARRAY, TYPE_MATRIX, TYPE_SET,
    TYPE_GRAPH, TYPE_CUSTOM
} DataType;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_FLOOR_DIV, OP_MOD, OP_POWER, OP_MATMUL,
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_AND, OP_OR, OP_NOT,
    OP_UNARY_PLUS, OP_UNARY_MINUS, OP_PRE_INC, OP_PRE_DEC, OP_POST_INC, OP_POST_DEC,
    OP_ASSIGN, OP_IN
} OperatorType;

typedef enum {
    NODE_INT_LITERAL, NODE_FLOAT_LITERAL, NODE_BOOL_LITERAL,
    NODE_COMPLEX_LITERAL, NODE_STRING_LITERAL, NODE_CHAR_LITERAL,
    NODE_IDENTIFIER, NODE_BINARY_OP, NODE_UNARY_OP, NODE_CALL,
    NODE_ARRAY_ACCESS, NODE_MEMBER_ACCESS, NODE_ARRAY_LITERAL,
    NODE_MATRIX_LITERAL, NODE_SET_LITERAL, NODE_DECLARATION,
    NODE_ASSIGNMENT, NODE_IF_STMT, NODE_WHILE_STMT, NODE_FOR_STMT,
    NODE_RETURN_STMT, NODE_BREAK_STMT, NODE_CONTINUE_STMT,
    NODE_EXPR_STMT, NODE_TRY_STMT, NODE_CATCH_CLAUSE,
    NODE_FUNCTION_DEF, NODE_TYPE_DEF, NODE_IMPORT,
    NODE_PROGRAM
} NodeType;

typedef struct TypeSpec {
    DataType base_type;
    struct TypeSpec* element_type;
    char* type_name;
} TypeSpec;

typedef struct NodeList {
    struct ASTNode* node;
    struct NodeList* next;
} NodeList;

typedef struct ParamList {
    struct TypeSpec* type;
    char* name;
    struct ParamList* next;
} ParamList;

typedef struct ASTNode {
    NodeType type;
    int line;

    TypeSpec* inferred_type;

    union {
        struct { int value; } int_literal;
        struct { double value; } float_literal;
        struct { int value; } bool_literal;
        struct { char* value; } complex_literal;
        struct { char* value; } string_literal;
        struct { char value; } char_literal;
        struct { char* name; } identifier;

        struct {
            OperatorType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        struct {
            OperatorType op;
            struct ASTNode* operand;
        } unary_op;

        struct {
            struct ASTNode* function;
            NodeList* arguments;
        } call;

        struct {
            struct ASTNode* array;
            struct ASTNode* index;
        } array_access;

        struct {
            struct ASTNode* object;
            char* member;
            NodeList* args;
        } member_access;

        struct { NodeList* elements; } array_literal;
        struct { NodeList* rows; } matrix_literal;
        struct { NodeList* elements; } set_literal;

        struct {
            TypeSpec* var_type;
            char* name;
            struct ASTNode* initializer;
            int is_const;
        } declaration;

        struct {
            struct ASTNode* target;
            OperatorType op;
            struct ASTNode* value;
        } assignment;

        struct {
            struct ASTNode* condition;
            NodeList* then_block;
            NodeList* elif_parts;
            NodeList* else_block;
        } if_stmt;

        struct {
            struct ASTNode* condition;
            NodeList* body;
        } while_stmt;

        struct {
            char* iterator;
            char* index_var;
            struct ASTNode* iterable;
            NodeList* body;
        } for_stmt;

        struct {
            struct ASTNode* value;
        } return_stmt;

        struct {
            struct ASTNode* expression;
        } expr_stmt;

        struct {
            NodeList* try_block;
            NodeList* catch_clauses;
            NodeList* finally_block;
        } try_stmt;

        struct {
            TypeSpec* exception_type;
            char* var_name;
            NodeList* body;
        } catch_clause;

        struct {
            TypeSpec* return_type;
            char* name;
            ParamList* parameters;
            NodeList* body;
        } function_def;

        struct {
            char* name;
            NodeList* members;
        } type_def;

        struct {
            char* module_name;
        } import;

        struct {
            NodeList* imports;
            NodeList* definitions;
            struct ASTNode* main_function;
        } program;
    };
} ASTNode;

/* Funções de criação */
ASTNode* create_int_literal(int value, int line);
ASTNode* create_float_literal(double value, int line);
ASTNode* create_bool_literal(int value, int line);
ASTNode* create_complex_literal(char* value, int line);
ASTNode* create_string_literal(char* value, int line);
ASTNode* create_char_literal(char value, int line);
ASTNode* create_identifier(char* name, int line);
ASTNode* create_binary_op(OperatorType op, ASTNode* left, ASTNode* right, int line);
ASTNode* create_unary_op(OperatorType op, ASTNode* operand, int line);
ASTNode* create_call(ASTNode* function, NodeList* arguments, int line);
ASTNode* create_array_access(ASTNode* array, ASTNode* index, int line);
ASTNode* create_member_access(ASTNode* object, char* member, NodeList* args, int line);
ASTNode* create_array_literal(NodeList* elements, int line);
ASTNode* create_matrix_literal(NodeList* rows, int line);
ASTNode* create_set_literal(NodeList* elements, int line);
ASTNode* create_declaration(TypeSpec* type, char* name, ASTNode* initializer, int line);
ASTNode* create_assignment(ASTNode* target, OperatorType op, ASTNode* value, int line);
ASTNode* create_if_stmt(ASTNode* condition, NodeList* then_block,
                       NodeList* elif_parts, NodeList* else_block, int line);
ASTNode* create_while_stmt(ASTNode* condition, NodeList* body, int line);
ASTNode* create_for_stmt(char* iterator, char* index_var,
                        ASTNode* iterable, NodeList* body, int line);
ASTNode* create_return_stmt(ASTNode* value, int line);
ASTNode* create_break_stmt(int line);
ASTNode* create_continue_stmt(int line);
ASTNode* create_expr_stmt(ASTNode* expression, int line);
ASTNode* create_try_stmt(NodeList* try_block, NodeList* catch_clauses,
                        NodeList* finally_block, int line);
ASTNode* create_catch_clause(TypeSpec* exception_type, char* var_name,
                             NodeList* body, int line);
ASTNode* create_function_def(TypeSpec* return_type, char* name,
                            ParamList* parameters, NodeList* body, int line);
ASTNode* create_type_def(char* name, NodeList* members, int line);
ASTNode* create_import(char* module_name, int line);
ASTNode* create_program(NodeList* imports, NodeList* definitions,
                       ASTNode* main_function, int line);

TypeSpec* create_type_spec(DataType base_type, TypeSpec* element_type, char* type_name);
NodeList* create_node_list(ASTNode* node, NodeList* next);
ParamList* create_param_list(TypeSpec* type, char* name, ParamList* next);

void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);
void free_ast_tree(ASTNode** root);
void free_node_list(NodeList* list);
void free_param_list(ParamList* list);
void free_type_spec(TypeSpec* spec);

/* Funções de debug */
int count_ast_nodes(ASTNode* node);

#ifdef DEBUG_MEMORY
void print_memory_stats(void);
#endif

#endif /* AST_H */