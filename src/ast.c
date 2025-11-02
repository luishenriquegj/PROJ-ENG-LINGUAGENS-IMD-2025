#define _POSIX_C_SOURCE 200809L
#include "ast.h"
#include <stdarg.h>

/* ========================================================================== */
/* FUNÇÕES AUXILIARES */
/* ========================================================================== */

static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

/* ========================================================================== */
/* CRIAÇÃO DE LITERAIS */
/* ========================================================================== */

ASTNode* create_int_literal(int value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_INT_LITERAL;
    node->line = line;
    node->int_literal.value = value;
    return node;
}

ASTNode* create_float_literal(double value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FLOAT_LITERAL;
    node->line = line;
    node->float_literal.value = value;
    return node;
}

ASTNode* create_bool_literal(int value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_BOOL_LITERAL;
    node->line = line;
    node->bool_literal.value = value;
    return node;
}

ASTNode* create_complex_literal(char* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_COMPLEX_LITERAL;
    node->line = line;
    node->complex_literal.value = strdup(value);
    return node;
}

ASTNode* create_string_literal(char* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_STRING_LITERAL;
    node->line = line;
    node->string_literal.value = strdup(value);
    return node;
}

ASTNode* create_char_literal(char value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_CHAR_LITERAL;
    node->line = line;
    node->char_literal.value = value;
    return node;
}

ASTNode* create_identifier(char* name, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->line = line;
    node->identifier.name = strdup(name);
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DE OPERAÇÕES */
/* ========================================================================== */

ASTNode* create_binary_op(OperatorType op, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_OP;
    node->line = line;
    node->binary_op.op = op;
    node->binary_op.left = left;
    node->binary_op.right = right;
    return node;
}

ASTNode* create_unary_op(OperatorType op, ASTNode* operand, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_OP;
    node->line = line;
    node->unary_op.op = op;
    node->unary_op.operand = operand;
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DE CHAMADAS E ACESSOS */
/* ========================================================================== */

ASTNode* create_call(ASTNode* function, NodeList* arguments, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_CALL;
    node->line = line;
    node->call.function = function;
    node->call.arguments = arguments;
    return node;
}

ASTNode* create_array_access(ASTNode* array, ASTNode* index, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_ACCESS;
    node->line = line;
    node->array_access.array = array;
    node->array_access.index = index;
    return node;
}

ASTNode* create_member_access(ASTNode* object, char* member, NodeList* args, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_MEMBER_ACCESS;
    node->line = line;
    node->member_access.object = object;
    node->member_access.member = strdup(member);
    node->member_access.args = args;
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DE LITERAIS COMPOSTOS */
/* ========================================================================== */

ASTNode* create_array_literal(NodeList* elements, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_ARRAY_LITERAL;
    node->line = line;
    node->array_literal.elements = elements;
    return node;
}

ASTNode* create_matrix_literal(NodeList* rows, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_MATRIX_LITERAL;
    node->line = line;
    node->matrix_literal.rows = rows;
    return node;
}

ASTNode* create_set_literal(NodeList* elements, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_SET_LITERAL;
    node->line = line;
    node->set_literal.elements = elements;
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DE STATEMENTS */
/* ========================================================================== */

ASTNode* create_declaration(TypeSpec* type, char* name, ASTNode* initializer, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_DECLARATION;
    node->line = line;
    node->declaration.var_type = type;
    node->declaration.name = strdup(name);
    node->declaration.initializer = initializer;
    return node;
}

ASTNode* create_assignment(ASTNode* target, OperatorType op, ASTNode* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;
    node->line = line;
    node->assignment.target = target;
    node->assignment.op = op;
    node->assignment.value = value;
    return node;
}

ASTNode* create_if_stmt(ASTNode* condition, NodeList* then_block,
                       NodeList* elif_parts, NodeList* else_block, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_IF_STMT;
    node->line = line;
    node->if_stmt.condition = condition;
    node->if_stmt.then_block = then_block;
    node->if_stmt.elif_parts = elif_parts;
    node->if_stmt.else_block = else_block;
    return node;
}

ASTNode* create_while_stmt(ASTNode* condition, NodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_WHILE_STMT;
    node->line = line;
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;
    return node;
}

ASTNode* create_for_stmt(char* iterator, char* index_var,
                        ASTNode* iterable, NodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FOR_STMT;
    node->line = line;
    node->for_stmt.iterator = strdup(iterator);
    node->for_stmt.index_var = index_var ? strdup(index_var) : NULL;
    node->for_stmt.iterable = iterable;
    node->for_stmt.body = body;
    return node;
}

ASTNode* create_return_stmt(ASTNode* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_RETURN_STMT;
    node->line = line;
    node->return_stmt.value = value;
    return node;
}

ASTNode* create_break_stmt(int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_BREAK_STMT;
    node->line = line;
    return node;
}

ASTNode* create_continue_stmt(int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_CONTINUE_STMT;
    node->line = line;
    return node;
}

ASTNode* create_expr_stmt(ASTNode* expression, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_EXPR_STMT;
    node->line = line;
    node->expr_stmt.expression = expression;
    return node;
}

ASTNode* create_try_stmt(NodeList* try_block, NodeList* catch_clauses,
                        NodeList* finally_block, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_TRY_STMT;
    node->line = line;
    node->try_stmt.try_block = try_block;
    node->try_stmt.catch_clauses = catch_clauses;
    node->try_stmt.finally_block = finally_block;
    return node;
}

ASTNode* create_catch_clause(TypeSpec* exception_type, char* var_name,
                             NodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_CATCH_CLAUSE;
    node->line = line;
    node->catch_clause.exception_type = exception_type;
    node->catch_clause.var_name = strdup(var_name);
    node->catch_clause.body = body;
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DE DEFINIÇÕES */
/* ========================================================================== */

ASTNode* create_function_def(TypeSpec* return_type, char* name,
                            ParamList* parameters, NodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_DEF;
    node->line = line;
    node->function_def.return_type = return_type;
    node->function_def.name = strdup(name);
    node->function_def.parameters = parameters;
    node->function_def.body = body;
    return node;
}

ASTNode* create_type_def(char* name, NodeList* members, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_TYPE_DEF;
    node->line = line;
    node->type_def.name = strdup(name);
    node->type_def.members = members;
    return node;
}

ASTNode* create_import(char* module_name, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_IMPORT;
    node->line = line;
    node->import.module_name = strdup(module_name);
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DO PROGRAMA */
/* ========================================================================== */

ASTNode* create_program(NodeList* imports, NodeList* definitions,
                       ASTNode* main_function, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_PROGRAM;
    node->line = line;
    node->program.imports = imports;
    node->program.definitions = definitions;
    node->program.main_function = main_function;
    return node;
}

/* ========================================================================== */
/* CRIAÇÃO DE TIPOS E LISTAS */
/* ========================================================================== */

TypeSpec* create_type_spec(DataType base_type, TypeSpec* element_type, char* type_name) {
    TypeSpec* spec = (TypeSpec*)malloc(sizeof(TypeSpec));
    spec->base_type = base_type;
    spec->element_type = element_type;
    spec->type_name = type_name ? strdup(type_name) : NULL;
    return spec;
}

NodeList* create_node_list(ASTNode* node, NodeList* next) {
    NodeList* list = (NodeList*)malloc(sizeof(NodeList));
    list->node = node;
    list->next = next;
    return list;
}

ParamList* create_param_list(TypeSpec* type, char* name, ParamList* next) {
    ParamList* list = (ParamList*)malloc(sizeof(ParamList));
    list->type = type;
    list->name = strdup(name);
    list->next = next;
    return list;
}

/* ========================================================================== */
/* IMPRESSÃO DA AST */
/* ========================================================================== */

char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_INT_LITERAL: return "INT_LITERAL";
        case NODE_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case NODE_BOOL_LITERAL: return "BOOL_LITERAL";
        case NODE_COMPLEX_LITERAL: return "COMPLEX_LITERAL";
        case NODE_STRING_LITERAL: return "STRING_LITERAL";
        case NODE_CHAR_LITERAL: return "CHAR_LITERAL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_BINARY_OP: return "BINARY_OP";
        case NODE_UNARY_OP: return "UNARY_OP";
        case NODE_CALL: return "CALL";
        case NODE_ARRAY_ACCESS: return "ARRAY_ACCESS";
        case NODE_MEMBER_ACCESS: return "MEMBER_ACCESS";
        case NODE_ARRAY_LITERAL: return "ARRAY_LITERAL";
        case NODE_MATRIX_LITERAL: return "MATRIX_LITERAL";
        case NODE_SET_LITERAL: return "SET_LITERAL";
        case NODE_DECLARATION: return "DECLARATION";
        case NODE_ASSIGNMENT: return "ASSIGNMENT";
        case NODE_IF_STMT: return "IF_STMT";
        case NODE_WHILE_STMT: return "WHILE_STMT";
        case NODE_FOR_STMT: return "FOR_STMT";
        case NODE_RETURN_STMT: return "RETURN_STMT";
        case NODE_BREAK_STMT: return "BREAK_STMT";
        case NODE_CONTINUE_STMT: return "CONTINUE_STMT";
        case NODE_EXPR_STMT: return "EXPR_STMT";
        case NODE_TRY_STMT: return "TRY_STMT";
        case NODE_CATCH_CLAUSE: return "CATCH_CLAUSE";
        case NODE_FUNCTION_DEF: return "FUNCTION_DEF";
        case NODE_TYPE_DEF: return "TYPE_DEF";
        case NODE_IMPORT: return "IMPORT";
        case NODE_PROGRAM: return "PROGRAM";
        default: return "UNKNOWN";
    }
}

char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_COMPLEX: return "complex";
        case TYPE_CHAR: return "char";
        case TYPE_STRING: return "string";
        case TYPE_VOID: return "void";
        case TYPE_ARRAY: return "array";
        case TYPE_MATRIX: return "matrix";
        case TYPE_SET: return "set";
        case TYPE_GRAPH: return "graph";
        case TYPE_CUSTOM: return "custom";
        default: return "unknown";
    }
}

static const char* op_to_string(OperatorType op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_FLOOR_DIV: return "//";
        case OP_MOD: return "%";
        case OP_POWER: return "^";
        case OP_MATMUL: return "@";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LE: return "<=";
        case OP_GE: return ">=";
        case OP_AND: return "and";
        case OP_OR: return "or";
        case OP_NOT: return "not";
        case OP_UNARY_PLUS: return "+";
        case OP_UNARY_MINUS: return "-";
        case OP_PRE_INC: return "++";
        case OP_PRE_DEC: return "--";
        case OP_POST_INC: return "++";
        case OP_POST_DEC: return "--";
        default: return "?";
    }
}

void print_type_spec(TypeSpec* type, int indent) {
    if (!type) {
        print_indent(indent);
        printf("(TYPE null)\n");
        return;
    }

    print_indent(indent);
    printf("(TYPE %s", data_type_to_string(type->base_type));

    if (type->element_type) {
        printf("\n");
        print_type_spec(type->element_type, indent + 1);
        print_indent(indent);
        printf(")");
    } else if (type->type_name) {
        printf(" %s)", type->type_name);
    } else {
        printf(")");
    }
    printf("\n");
}

void print_param_list(ParamList* params, int indent) {
    while (params) {
        print_indent(indent);
        printf("(PARAM %s\n", params->name);
        print_type_spec(params->type, indent + 1);
        print_indent(indent);
        printf(")\n");
        params = params->next;
    }
}

void print_node_list(NodeList* list, int indent) {
    while (list) {
        print_ast(list->node, indent);
        list = list->next;
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        /* ================================================================ */
        /* LITERAIS */
        /* ================================================================ */
        case NODE_INT_LITERAL:
            printf("(INT %d)\n", node->int_literal.value);
            break;

        case NODE_FLOAT_LITERAL:
            printf("(FLOAT %.6f)\n", node->float_literal.value);
            break;

        case NODE_BOOL_LITERAL:
            printf("(BOOL %s)\n", node->bool_literal.value ? "true" : "false");
            break;

        case NODE_COMPLEX_LITERAL:
            printf("(COMPLEX %s)\n", node->complex_literal.value);
            break;

        case NODE_STRING_LITERAL:
            printf("(STRING \"%s\")\n", node->string_literal.value);
            break;

        case NODE_CHAR_LITERAL:
            printf("(CHAR '%c')\n", node->char_literal.value);
            break;

        /* ================================================================ */
        /* IDENTIFICADOR */
        /* ================================================================ */
        case NODE_IDENTIFIER:
            printf("(ID %s)\n", node->identifier.name);
            break;

        /* ================================================================ */
        /* OPERAÇÕES */
        /* ================================================================ */
        case NODE_BINARY_OP:
            printf("(BINOP %s\n", op_to_string(node->binary_op.op));
            print_ast(node->binary_op.left, indent + 1);
            print_ast(node->binary_op.right, indent + 1);
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_UNARY_OP:
            printf("(UNOP %s\n", op_to_string(node->unary_op.op));
            print_ast(node->unary_op.operand, indent + 1);
            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* CHAMADAS E ACESSOS */
        /* ================================================================ */
        case NODE_CALL:
            printf("(CALL\n");
            print_ast(node->call.function, indent + 1);
            if (node->call.arguments) {
                print_indent(indent + 1);
                printf("(ARGS\n");
                print_node_list(node->call.arguments, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_ARRAY_ACCESS:
            printf("(ARRAY_ACCESS\n");
            print_ast(node->array_access.array, indent + 1);
            print_ast(node->array_access.index, indent + 1);
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_MEMBER_ACCESS:
            printf("(MEMBER_ACCESS\n");
            print_ast(node->member_access.object, indent + 1);
            print_indent(indent + 1);
            printf("(MEMBER %s)\n", node->member_access.member);
            if (node->member_access.args) {
                print_indent(indent + 1);
                printf("(ARGS\n");
                print_node_list(node->member_access.args, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }
            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* LITERAIS COMPOSTOS */
        /* ================================================================ */
        case NODE_ARRAY_LITERAL:
            printf("(ARRAY\n");
            if (node->array_literal.elements) {
                print_node_list(node->array_literal.elements, indent + 1);
            }
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_MATRIX_LITERAL:
            printf("(MATRIX\n");
            if (node->matrix_literal.rows) {
                print_node_list(node->matrix_literal.rows, indent + 1);
            }
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_SET_LITERAL:
            printf("(SET\n");
            if (node->set_literal.elements) {
                print_node_list(node->set_literal.elements, indent + 1);
            }
            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* DECLARAÇÕES */
        /* ================================================================ */
        case NODE_DECLARATION:
            printf("(DECL %s\n", node->declaration.name);
            print_type_spec(node->declaration.var_type, indent + 1);
            if (node->declaration.initializer) {
                print_indent(indent + 1);
                printf("(INIT\n");
                print_ast(node->declaration.initializer, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_ASSIGNMENT:
            printf("(ASSIGN %s\n", op_to_string(node->assignment.op));
            print_ast(node->assignment.target, indent + 1);
            print_ast(node->assignment.value, indent + 1);
            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* ESTRUTURAS DE CONTROLE */
        /* ================================================================ */
        case NODE_IF_STMT:
            printf("(IF\n");
            print_indent(indent + 1);
            printf("(COND\n");
            print_ast(node->if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent + 1);
            printf("(THEN\n");
            print_node_list(node->if_stmt.then_block, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            if (node->if_stmt.elif_parts) {
                print_indent(indent + 1);
                printf("(ELIF\n");
                print_node_list(node->if_stmt.elif_parts, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            if (node->if_stmt.else_block) {
                print_indent(indent + 1);
                printf("(ELSE\n");
                print_node_list(node->if_stmt.else_block, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            print_indent(indent);
            printf(")\n");
            break;

        case NODE_WHILE_STMT:
            printf("(WHILE\n");
            print_indent(indent + 1);
            printf("(COND\n");
            print_ast(node->while_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent + 1);
            printf("(BODY\n");
            print_node_list(node->while_stmt.body, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent);
            printf(")\n");
            break;

        case NODE_FOR_STMT:
            printf("(FOR\n");
            print_indent(indent + 1);
            printf("(ITER %s", node->for_stmt.iterator);
            if (node->for_stmt.index_var) {
                printf(", %s", node->for_stmt.index_var);
            }
            printf(")\n");

            print_indent(indent + 1);
            printf("(IN\n");
            print_ast(node->for_stmt.iterable, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent + 1);
            printf("(BODY\n");
            print_node_list(node->for_stmt.body, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* STATEMENTS SIMPLES */
        /* ================================================================ */
        case NODE_RETURN_STMT:
            printf("(RETURN\n");
            if (node->return_stmt.value) {
                print_ast(node->return_stmt.value, indent + 1);
            }
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_BREAK_STMT:
            printf("(BREAK)\n");
            break;

        case NODE_CONTINUE_STMT:
            printf("(CONTINUE)\n");
            break;

        case NODE_EXPR_STMT:
            printf("(EXPR_STMT\n");
            print_ast(node->expr_stmt.expression, indent + 1);
            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* TRY/CATCH */
        /* ================================================================ */
        case NODE_TRY_STMT:
            printf("(TRY\n");
            print_indent(indent + 1);
            printf("(TRY_BLOCK\n");
            print_node_list(node->try_stmt.try_block, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            if (node->try_stmt.catch_clauses) {
                print_indent(indent + 1);
                printf("(CATCHES\n");
                print_node_list(node->try_stmt.catch_clauses, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            if (node->try_stmt.finally_block) {
                print_indent(indent + 1);
                printf("(FINALLY\n");
                print_node_list(node->try_stmt.finally_block, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            print_indent(indent);
            printf(")\n");
            break;

        case NODE_CATCH_CLAUSE:
            printf("(CATCH %s\n", node->catch_clause.var_name);
            print_type_spec(node->catch_clause.exception_type, indent + 1);
            print_indent(indent + 1);
            printf("(BODY\n");
            print_node_list(node->catch_clause.body, indent + 2);
            print_indent(indent + 1);
            printf(")\n");
            print_indent(indent);
            printf(")\n");
            break;

        /* ================================================================ */
        /* DEFINIÇÕES */
        /* ================================================================ */
        case NODE_FUNCTION_DEF:
            printf("(FUNCTION %s\n", node->function_def.name);

            print_indent(indent + 1);
            printf("(RETURN_TYPE\n");
            print_type_spec(node->function_def.return_type, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            if (node->function_def.parameters) {
                print_indent(indent + 1);
                printf("(PARAMS\n");
                print_param_list(node->function_def.parameters, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            print_indent(indent + 1);
            printf("(BODY\n");
            print_node_list(node->function_def.body, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent);
            printf(")\n");
            break;

        case NODE_TYPE_DEF:
            printf("(TYPE_DEF %s\n", node->type_def.name);
            print_indent(indent + 1);
            printf("(MEMBERS\n");
            print_node_list(node->type_def.members, indent + 2);
            print_indent(indent + 1);
            printf(")\n");
            print_indent(indent);
            printf(")\n");
            break;

        case NODE_IMPORT:
            printf("(IMPORT %s)\n", node->import.module_name);
            break;

        /* ================================================================ */
        /* PROGRAMA */
        /* ================================================================ */
        case NODE_PROGRAM:
            printf("(PROGRAM\n");

            if (node->program.imports) {
                print_indent(indent + 1);
                printf("(IMPORTS\n");
                print_node_list(node->program.imports, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            if (node->program.definitions) {
                print_indent(indent + 1);
                printf("(DEFINITIONS\n");
                print_node_list(node->program.definitions, indent + 2);
                print_indent(indent + 1);
                printf(")\n");
            }

            print_indent(indent + 1);
            printf("(MAIN\n");
            print_ast(node->program.main_function, indent + 2);
            print_indent(indent + 1);
            printf(")\n");

            print_indent(indent);
            printf(")\n");
            break;

        default:
            printf("(UNKNOWN_NODE_TYPE %d)\n", node->type);
            break;
    }
}

void free_node_list(NodeList* list) {
    while (list != NULL) {
        NodeList* next = list->next;
        if (list->node != NULL) {
            free_ast(list->node);
        }
        free(list);
        list = next;
    }
}

/* Liberar lista de parâmetros */
void free_param_list(ParamList* list) {
    while (list != NULL) {
        ParamList* next = list->next;
        if (list->type != NULL) {
            free_type_spec(list->type);
        }
        if (list->name != NULL) {
            free(list->name);
        }
        free(list);
        list = next;
    }
}

/* Liberar especificação de tipo */
void free_type_spec(TypeSpec* spec) {
    if (spec == NULL) return;

    if (spec->element_type != NULL) {
        free_type_spec(spec->element_type);
    }
    if (spec->type_name != NULL) {
        free(spec->type_name);
    }
    free(spec);
}

/* Função principal de liberação da AST */
void free_ast(ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        /* ================================================================ */
        /* LITERAIS */
        /* ================================================================ */
        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
        case NODE_BOOL_LITERAL:
        case NODE_CHAR_LITERAL:
            // Tipos primitivos, sem alocação dinâmica adicional
            break;

        case NODE_COMPLEX_LITERAL:
            if (node->complex_literal.value != NULL) {
                free(node->complex_literal.value);
            }
            break;

        case NODE_STRING_LITERAL:
            if (node->string_literal.value != NULL) {
                free(node->string_literal.value);
            }
            break;

        /* ================================================================ */
        /* IDENTIFICADOR */
        /* ================================================================ */
        case NODE_IDENTIFIER:
            if (node->identifier.name != NULL) {
                free(node->identifier.name);
            }
            break;

        /* ================================================================ */
        /* OPERAÇÕES */
        /* ================================================================ */
        case NODE_BINARY_OP:
            free_ast(node->binary_op.left);
            free_ast(node->binary_op.right);
            break;

        case NODE_UNARY_OP:
            free_ast(node->unary_op.operand);
            break;

        /* ================================================================ */
        /* CHAMADAS E ACESSOS */
        /* ================================================================ */
        case NODE_CALL:
            free_ast(node->call.function);
            free_node_list(node->call.arguments);
            break;

        case NODE_ARRAY_ACCESS:
            free_ast(node->array_access.array);
            free_ast(node->array_access.index);
            break;

        case NODE_MEMBER_ACCESS:
            free_ast(node->member_access.object);
            if (node->member_access.member != NULL) {
                free(node->member_access.member);
            }
            free_node_list(node->member_access.args);
            break;

        /* ================================================================ */
        /* LITERAIS COMPOSTOS */
        /* ================================================================ */
        case NODE_ARRAY_LITERAL:
            free_node_list(node->array_literal.elements);
            break;

        case NODE_MATRIX_LITERAL:
            free_node_list(node->matrix_literal.rows);
            break;

        case NODE_SET_LITERAL:
            free_node_list(node->set_literal.elements);
            break;

        /* ================================================================ */
        /* DECLARAÇÕES E ATRIBUIÇÕES */
        /* ================================================================ */
        case NODE_DECLARATION:
            free_type_spec(node->declaration.var_type);
            if (node->declaration.name != NULL) {
                free(node->declaration.name);
            }
            free_ast(node->declaration.initializer);
            break;

        case NODE_ASSIGNMENT:
            free_ast(node->assignment.target);
            free_ast(node->assignment.value);
            break;

        /* ================================================================ */
        /* ESTRUTURAS DE CONTROLE */
        /* ================================================================ */
        case NODE_IF_STMT:
            free_ast(node->if_stmt.condition);
            free_node_list(node->if_stmt.then_block);
            free_node_list(node->if_stmt.elif_parts);
            free_node_list(node->if_stmt.else_block);
            break;

        case NODE_WHILE_STMT:
            free_ast(node->while_stmt.condition);
            free_node_list(node->while_stmt.body);
            break;

        case NODE_FOR_STMT:
            if (node->for_stmt.iterator != NULL) {
                free(node->for_stmt.iterator);
            }
            if (node->for_stmt.index_var != NULL) {
                free(node->for_stmt.index_var);
            }
            free_ast(node->for_stmt.iterable);
            free_node_list(node->for_stmt.body);
            break;

        /* ================================================================ */
        /* STATEMENTS SIMPLES */
        /* ================================================================ */
        case NODE_RETURN_STMT:
            free_ast(node->return_stmt.value);
            break;

        case NODE_BREAK_STMT:
        case NODE_CONTINUE_STMT:
            // Sem alocação dinâmica adicional
            break;

        case NODE_EXPR_STMT:
            free_ast(node->expr_stmt.expression);
            break;

        /* ================================================================ */
        /* TRATAMENTO DE EXCEÇÕES */
        /* ================================================================ */
        case NODE_TRY_STMT:
            free_node_list(node->try_stmt.try_block);
            free_node_list(node->try_stmt.catch_clauses);
            free_node_list(node->try_stmt.finally_block);
            break;

        case NODE_CATCH_CLAUSE:
            free_type_spec(node->catch_clause.exception_type);
            if (node->catch_clause.var_name != NULL) {
                free(node->catch_clause.var_name);
            }
            free_node_list(node->catch_clause.body);
            break;

        /* ================================================================ */
        /* DEFINIÇÕES */
        /* ================================================================ */
        case NODE_FUNCTION_DEF:
            free_type_spec(node->function_def.return_type);
            if (node->function_def.name != NULL) {
                free(node->function_def.name);
            }
            free_param_list(node->function_def.parameters);
            free_node_list(node->function_def.body);
            break;

        case NODE_TYPE_DEF:
            if (node->type_def.name != NULL) {
                free(node->type_def.name);
            }
            free_node_list(node->type_def.members);
            break;

        case NODE_IMPORT:
            if (node->import.module_name != NULL) {
                free(node->import.module_name);
            }
            break;

        /* ================================================================ */
        /* PROGRAMA PRINCIPAL */
        /* ================================================================ */
        case NODE_PROGRAM:
            free_node_list(node->program.imports);
            free_node_list(node->program.definitions);
            free_ast(node->program.main_function);
            break;


        default:
            break;
    }

    // Liberar o próprio nó
    free(node);
}

/* ========================================================================== */
/* FUNÇÃO AUXILIAR PARA LIBERAR TODA A AST E RESETAR O PONTEIRO */
/* ========================================================================== */

void free_ast_tree(ASTNode** root) {
    if (root != NULL && *root != NULL) {
        free_ast(*root);
        *root = NULL;
    }
}

/* ========================================================================== */
/* FUNÇÃO PARA CONTAR NÓS NA AST (ÚTIL PARA DEBUG) */
/* ========================================================================== */

int count_ast_nodes(ASTNode* node) {
    if (node == NULL) return 0;

    int count = 1;  // Contar o nó atual

    switch (node->type) {
        case NODE_BINARY_OP:
            count += count_ast_nodes(node->binary_op.left);
            count += count_ast_nodes(node->binary_op.right);
            break;

        case NODE_UNARY_OP:
            count += count_ast_nodes(node->unary_op.operand);
            break;

        case NODE_CALL:
            count += count_ast_nodes(node->call.function);
            {
                NodeList* args = node->call.arguments;
                while (args) {
                    count += count_ast_nodes(args->node);
                    args = args->next;
                }
            }
            break;

        case NODE_ARRAY_ACCESS:
            count += count_ast_nodes(node->array_access.array);
            count += count_ast_nodes(node->array_access.index);
            break;

        case NODE_MEMBER_ACCESS:
            count += count_ast_nodes(node->member_access.object);
            {
                NodeList* args = node->member_access.args;
                while (args) {
                    count += count_ast_nodes(args->node);
                    args = args->next;
                }
            }
            break;

        case NODE_ARRAY_LITERAL:
        case NODE_SET_LITERAL:
            {
                NodeList* elems = node->array_literal.elements;
                while (elems) {
                    count += count_ast_nodes(elems->node);
                    elems = elems->next;
                }
            }
            break;

        case NODE_MATRIX_LITERAL:
            {
                NodeList* rows = node->matrix_literal.rows;
                while (rows) {
                    count += count_ast_nodes(rows->node);
                    rows = rows->next;
                }
            }
            break;

        case NODE_DECLARATION:
            count += count_ast_nodes(node->declaration.initializer);
            break;

        case NODE_ASSIGNMENT:
            count += count_ast_nodes(node->assignment.target);
            count += count_ast_nodes(node->assignment.value);
            break;

        case NODE_IF_STMT:
            count += count_ast_nodes(node->if_stmt.condition);
            {
                NodeList* stmts = node->if_stmt.then_block;
                while (stmts) {
                    count += count_ast_nodes(stmts->node);
                    stmts = stmts->next;
                }
                stmts = node->if_stmt.elif_parts;
                while (stmts) {
                    count += count_ast_nodes(stmts->node);
                    stmts = stmts->next;
                }
                stmts = node->if_stmt.else_block;
                while (stmts) {
                    count += count_ast_nodes(stmts->node);
                    stmts = stmts->next;
                }
            }
            break;

        case NODE_WHILE_STMT:
            count += count_ast_nodes(node->while_stmt.condition);
            {
                NodeList* body = node->while_stmt.body;
                while (body) {
                    count += count_ast_nodes(body->node);
                    body = body->next;
                }
            }
            break;

        case NODE_FOR_STMT:
            count += count_ast_nodes(node->for_stmt.iterable);
            {
                NodeList* body = node->for_stmt.body;
                while (body) {
                    count += count_ast_nodes(body->node);
                    body = body->next;
                }
            }
            break;

        case NODE_RETURN_STMT:
            count += count_ast_nodes(node->return_stmt.value);
            break;

        case NODE_EXPR_STMT:
            count += count_ast_nodes(node->expr_stmt.expression);
            break;

        case NODE_TRY_STMT:
            {
                NodeList* list = node->try_stmt.try_block;
                while (list) {
                    count += count_ast_nodes(list->node);
                    list = list->next;
                }
                list = node->try_stmt.catch_clauses;
                while (list) {
                    count += count_ast_nodes(list->node);
                    list = list->next;
                }
                list = node->try_stmt.finally_block;
                while (list) {
                    count += count_ast_nodes(list->node);
                    list = list->next;
                }
            }
            break;

        case NODE_CATCH_CLAUSE:
            {
                NodeList* body = node->catch_clause.body;
                while (body) {
                    count += count_ast_nodes(body->node);
                    body = body->next;
                }
            }
            break;

        case NODE_FUNCTION_DEF:
            {
                NodeList* body = node->function_def.body;
                while (body) {
                    count += count_ast_nodes(body->node);
                    body = body->next;
                }
            }
            break;

        case NODE_TYPE_DEF:
            {
                NodeList* members = node->type_def.members;
                while (members) {
                    count += count_ast_nodes(members->node);
                    members = members->next;
                }
            }
            break;

        case NODE_PROGRAM:
            {
                NodeList* list = node->program.imports;
                while (list) {
                    count += count_ast_nodes(list->node);
                    list = list->next;
                }
                list = node->program.definitions;
                while (list) {
                    count += count_ast_nodes(list->node);
                    list = list->next;
                }
                count += count_ast_nodes(node->program.main_function);
            }
            break;

        default:
            // Outros tipos de nós (literais, etc.) não têm filhos
            break;
    }

    return count;
}