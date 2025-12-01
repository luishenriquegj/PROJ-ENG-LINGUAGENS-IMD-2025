%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "../libs/symbol-table.h"
#include "../libs/stack.h"

extern int yylex();
extern int yylineno;
extern int yychar;

void yyerror(const char* s);

extern SymbolTable *symbol_table;
extern FILE * yyin, * yyout;

void undeclared_variable_error(const char* var_name, int line);
void already_declared_variable_error(char* var_name, int line);
void type_error(char* t1, char* t2, int line);
char *cat(char *t1, char *t2, char *t3, char *t4, char *t5);
void check_variables(const char* var_name, int line);
void vars_routine(ASTNode* node);
int count_params(ParamList *p);
int count_nodelist(NodeList *n);

const char* datatype_to_string(DataType t);
const char* datatype_to_string_full(TypeSpec* t);

extern const char* lex_current_line(void);
extern int lex_tok_line(void);
extern int lex_tok_col(void);
extern int lex_tok_len(void);
extern const char* lex_tok_text(void);

ASTNode* ast_root = NULL;

static NodeList* reverse_node_list(NodeList* list);
static ParamList* reverse_param_list(ParamList* list);

static char* for_iter_name = NULL;
static char* for_val_name = NULL;
static char* for_idx_name = NULL;
static ASTNode* for_iter_expr = NULL;
static NodeList* for_body = NULL;
static ParamList* current_func_params = NULL;

static void print_syntax_error(const char* msg) {
    const int W = 100;
    const char* raw = lex_current_line();
    int line = lex_tok_line();
    int col  = lex_tok_col();
    int len  = lex_tok_len();
    if (len <= 0) len = 1;

    const char* ln = raw ? raw : "";
    int L = (int)strlen(ln);

    int start = 0;
    if (col > W/2) start = col - 1 - W/2;
    if (start < 0) start = 0;
    if (start > L) start = L;

    int end = start + W;
    if (end > L) end = L;

    fprintf(stderr, "Erro sintático na linha %d, coluna %d: %s\n", line, col, msg);

    if (L == 0) {
        fprintf(stderr, "  <linha vazia>\n");
        return;
    }

    if (start > 0) fputs("  ...", stderr);
    fputs("  ", stderr);
    fwrite(ln + start, 1, end - start, stderr);
    if (end < L) fputs("...", stderr);
    fputc('\n', stderr);

    fputs("  ", stderr);
    if (start > 0) fputs("   ", stderr);
    int caret_pos = col - 1 - start;
    if (caret_pos < 0) caret_pos = 0;
    for (int i = 0; i < caret_pos; i++) fputc(' ', stderr);
    int caret_len = len;
    if (caret_pos + caret_len > end - start) caret_len = (end - start) - caret_pos;
    if (caret_len < 1) caret_len = 1;
    for (int i = 0; i < caret_len; i++) fputc('^', stderr);
    fputc('\n', stderr);
}
%}

%union {
    int ival;
    double fval;
    char cval;
    char* sval;
    int bval;
    struct ASTNode* node;
    struct NodeList* node_list;
    struct ParamList* param_list;
    struct TypeSpec* type_spec;
    int op;
}

%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token <sval> STRING_LITERAL COMPLEX_LITERAL IDENTIFIER
%token <cval> CHAR_LITERAL
%token <bval> TRUE FALSE

%token DEF END FUN IF ELIF ELSE WHILE FOR IN RETURN BREAK CONTINUE
%token TRY CATCH FINALLY IMPORT CONST THIS NEW PRINT RANGE CLASS
%token INT_TYPE FLOAT_TYPE BOOL_TYPE COMPLEX_TYPE CHAR_TYPE STRING_TYPE
%token VOID_TYPE UNDEFINED_TYPE MATRIX_TYPE SET_TYPE GRAPH_TYPE
%token PLUS MINUS STAR SLASH FLOOR_DIV MOD POWER MATMUL
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MOD_ASSIGN
%token INCREMENT DECREMENT
%token EQUAL NOT_EQUAL LESS GREATER LESS_EQUAL GREATER_EQUAL
%token AND OR NOT
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token COMMA COLON DOT
%token NEWLINE INDENT DEDENT

%type <node> program statement declaration assignment if_stmt while_stmt for_stmt
%type <node> for_single for_double for_body_block func_body_block
%type <node> return_stmt break_stmt continue_stmt expr_stmt try_stmt catch_clause
%type <node> expression or_expr and_expr not_expr rel_expr add_expr mult_expr
%type <node> exp_expr unary_expr postfix_expr primary_expr literal
%type <node> array_literal set_literal import definition function_def class_def
%type <node> class_member

%type <node_list> import_list definitions definition_list
%type <node_list> statement_list indented_block expression_list opt_args
%type <node_list> catch_list opt_finally elif_chain opt_else
%type <node_list> class_member_list indented_class_block
%type <node_list> opt_imports_nl opt_newlines

%type <param_list> params param_list

%type <type_spec> type primitive_type array_type matrix_type set_type graph_type

%type <op> assign_op rel_op

%define parse.error verbose

%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MOD_ASSIGN
%left OR
%left AND
%right NOT
%left EQUAL NOT_EQUAL
%left LESS GREATER LESS_EQUAL GREATER_EQUAL IN
%left PLUS MINUS
%left STAR SLASH FLOOR_DIV MOD MATMUL
%right POWER
%left DOT
%left LBRACKET
%left LPAREN
%right UNARY
%left INCREMENT DECREMENT

%%

program
    : opt_newlines opt_imports_nl definitions {
        NodeList* imports = $2 ? reverse_node_list($2) : NULL;
        NodeList* defs = $3 ? reverse_node_list($3) : NULL;
        ast_root = create_program(imports, defs, NULL, yylineno);
        $$ = ast_root;
    }
    ;

opt_newlines
    : %empty { $$ = NULL; }
    | opt_newlines NEWLINE { $$ = NULL; }
    ;

opt_imports_nl
    : %empty { $$ = NULL; }
    | import_list opt_newlines { $$ = $1; }
    ;

import_list
    : import { $$ = create_node_list($1, NULL); }
    | import_list import { $$ = create_node_list($2, $1); }
    ;

import
    : IMPORT IDENTIFIER NEWLINE {
        $$ = create_import($2, yylineno);
        free($2);
    }
    | IMPORT STRING_LITERAL NEWLINE {
        $$ = create_import($2, yylineno);
        free($2);
    }
    ;

definitions
    : definition_list opt_newlines { $$ = $1; }
    ;

definition_list
    : definition { $$ = create_node_list($1, NULL); }
    | definition_list opt_newlines definition { $$ = create_node_list($3, $1); }
    ;

definition
    : function_def
    | class_def
    ;

function_def
    : DEF FUN type IDENTIFIER LPAREN params RPAREN NEWLINE 
        {
            current_func_params = $6 ? reverse_param_list($6) : NULL;
            if (!symbol_table_insert(symbol_table, $4, $3, SYM_FUNC, yylineno, current_func_params)) {
                already_declared_variable_error($4, yylineno);
            }
            symbol_table_enter_scope(symbol_table);
            ParamList* p = current_func_params;
            while (p != NULL) {
                if (!symbol_table_insert(symbol_table, p->name, p->type, SYM_PARAM, yylineno, NULL)) {
                    already_declared_variable_error(p->name, yylineno);
                }
                p = p->next;
            }
        }
    func_body_block END DEF {
        $$ = create_function_def($3, $4, current_func_params, for_body, yylineno);
        free($4);
        symbol_table_leave_scope(symbol_table);
        for_body = NULL;
        current_func_params = NULL;
    }
    ;

func_body_block
    : indented_block DEDENT {
        for_body = $1 ? reverse_node_list($1) : NULL;
        $$ = NULL;
    }
    ;

params
    : %empty { $$ = NULL; }
    | param_list { $$ = $1; }
    ;

param_list
    : type IDENTIFIER {
        $$ = create_param_list($1, $2, NULL);
        free($2);
    }
    | param_list COMMA type IDENTIFIER {
        $$ = create_param_list($3, $4, $1);
        free($4);
    }
    ;

class_def
    : DEF IDENTIFIER NEWLINE
        {
            symbol_table_enter_scope(symbol_table);
        } 
    indented_class_block DEDENT END DEF {
        NodeList* members = $5 ? reverse_node_list($5) : NULL;
        $$ = create_type_def($2, members, yylineno);
        free($2);

        symbol_table_leave_scope(symbol_table);
    }
    ;

indented_class_block
    : INDENT class_member_list { $$ = $2; }
    ;

class_member_list
    : %empty { $$ = NULL; }
    | class_member_list class_member {
        if ($2) {
            $$ = create_node_list($2, $1 ? $1 : NULL);
        } else {
            $$ = $1;
        }
    }
    ;

class_member
    : declaration NEWLINE { $$ = $1; }
    | function_def { $$ = $1; }
    ;

type
    : primitive_type
    | array_type
    | matrix_type
    | set_type
    | graph_type
    | IDENTIFIER { $$ = create_type_spec(TYPE_CUSTOM, NULL, $1); free($1); }
    ;

primitive_type
    : INT_TYPE { $$ = create_type_spec(TYPE_INT, NULL, NULL); }
    | FLOAT_TYPE { $$ = create_type_spec(TYPE_FLOAT, NULL, NULL); }
    | BOOL_TYPE { $$ = create_type_spec(TYPE_BOOL, NULL, NULL); }
    | COMPLEX_TYPE { $$ = create_type_spec(TYPE_COMPLEX, NULL, NULL); }
    | CHAR_TYPE { $$ = create_type_spec(TYPE_CHAR, NULL, NULL); }
    | STRING_TYPE { $$ = create_type_spec(TYPE_STRING, NULL, NULL); }
    | VOID_TYPE { $$ = create_type_spec(TYPE_VOID, NULL, NULL); }
    | UNDEFINED_TYPE { $$ = create_type_spec(TYPE_UNDEFINED, NULL, NULL); }
    ;

array_type
    : primitive_type LBRACKET RBRACKET { $$ = create_type_spec(TYPE_ARRAY, $1, NULL); }
    | array_type LBRACKET RBRACKET { $$ = create_type_spec(TYPE_ARRAY, $1, NULL); }
    ;

matrix_type
    : MATRIX_TYPE LBRACKET primitive_type RBRACKET { $$ = create_type_spec(TYPE_MATRIX, $3, NULL); }
    ;

set_type
    : SET_TYPE LBRACKET primitive_type RBRACKET { $$ = create_type_spec(TYPE_SET, $3, NULL); }
    ;

graph_type
    : GRAPH_TYPE LBRACKET primitive_type RBRACKET { $$ = create_type_spec(TYPE_GRAPH, $3, NULL); }
    ;

indented_block
    : INDENT statement_list { $$ = $2; }
    ;

/* Correção para blocos vazios */
statement_list
    : %empty { $$ = NULL; }
    | statement_list statement {
        if ($2) {
            $$ = create_node_list($2, $1 ? $1 : NULL);
        } else {
            $$ = $1;
        }
    }
    ;

statement
    : declaration NEWLINE { $$ = $1; }
    | assignment NEWLINE { $$ = $1; }
    | if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | for_stmt { $$ = $1; }
    | return_stmt NEWLINE { $$ = $1; }
    | break_stmt NEWLINE { $$ = $1; }
    | continue_stmt NEWLINE { $$ = $1; }
    | expr_stmt NEWLINE { $$ = $1; }
    | try_stmt { $$ = $1; }
    | NEWLINE { $$ = NULL; }
    ;

declaration
    : type IDENTIFIER ASSIGN expression {
        if (!symbol_table_insert(symbol_table, $2, $1, SYM_VAR, yylineno, NULL)) {
            already_declared_variable_error($2, yylineno);
        }
        $$ = create_declaration($1, $2, $4, yylineno);
        free($2);
    }
    | type IDENTIFIER {
        if (!symbol_table_insert(symbol_table, $2, $1, SYM_VAR, yylineno, NULL)) {
            already_declared_variable_error($2, yylineno);
        }
        $$ = create_declaration($1, $2, NULL, yylineno);
        free($2);
    }
    | CONST type IDENTIFIER ASSIGN expression {
        if (!symbol_table_insert(symbol_table, $3, $2, SYM_VAR, yylineno, NULL)) {
            already_declared_variable_error($3, yylineno);
        }
        ASTNode* d = create_declaration($2, $3, $5, yylineno);
        d->declaration.is_const = 1;
        $$ = d;
        free($3);
    }
    ;

assignment
    : postfix_expr assign_op expression {
        /* Valida que target existe se for identificador simples */
        if ($1->type == NODE_IDENTIFIER) {
            Symbol* s = symbol_table_lookup(symbol_table, $1->identifier.name);
            if (!s) {
                undeclared_variable_error($1->identifier.name, yylineno);
                exit(1);
            }
        }
        $$ = create_assignment($1, $2, $3, yylineno);
    }
    ;

assign_op
    : ASSIGN { $$ = OP_ASSIGN; }
    | PLUS_ASSIGN { $$ = OP_ADD; }
    | MINUS_ASSIGN { $$ = OP_SUB; }
    | STAR_ASSIGN { $$ = OP_MUL; }
    | SLASH_ASSIGN { $$ = OP_DIV; }
    | MOD_ASSIGN { $$ = OP_MOD; }
    ;

if_stmt
    : IF LPAREN expression RPAREN NEWLINE indented_block DEDENT elif_chain opt_else {
        NodeList* then_block = $6 ? reverse_node_list($6) : NULL;
        NodeList* elif_parts = $8 ? reverse_node_list($8) : NULL;
        NodeList* else_block = $9 ? reverse_node_list($9) : NULL;
        $$ = create_if_stmt($3, then_block, elif_parts, else_block, yylineno);
    }
    ;

elif_chain
    : %empty { $$ = NULL; }
    | ELIF LPAREN expression RPAREN NEWLINE indented_block DEDENT elif_chain {
        NodeList* body = $6 ? reverse_node_list($6) : NULL;
        ASTNode* e = create_if_stmt($3, body, NULL, NULL, yylineno);
        if ($8) {
            NodeList* current = create_node_list(e, NULL);
            NodeList* last = current;
            while (last->next) last = last->next;
            last->next = $8;
            $$ = current;
        } else {
            $$ = create_node_list(e, NULL);
        }
    }
    ;

opt_else
    : %empty { $$ = NULL; }
    | ELSE NEWLINE indented_block DEDENT {
        $$ = $3 ? reverse_node_list($3) : NULL;
    }
    ;

while_stmt
    : WHILE LPAREN expression RPAREN NEWLINE indented_block DEDENT {
        NodeList* body = $6 ? reverse_node_list($6) : NULL;
        $$ = create_while_stmt($3, body, yylineno);
    }
    ;

for_stmt
    : for_single
    | for_double
    ;

for_single
    : FOR LPAREN IDENTIFIER IN expression RPAREN NEWLINE {
        for_iter_name = $3;
        for_iter_expr = $5;
        symbol_table_enter_scope(symbol_table);
        TypeSpec* iter_type = create_type_spec(TYPE_INT, NULL, NULL);
        symbol_table_insert(symbol_table, for_iter_name, iter_type, SYM_VAR, yylineno, NULL);
    } for_body_block {
        $$ = create_for_stmt(for_iter_name, NULL, for_iter_expr, for_body, yylineno);
        free(for_iter_name);
        symbol_table_leave_scope(symbol_table);
        for_iter_name = NULL;
        for_iter_expr = NULL;
        for_body = NULL;
    }
    ;

for_double
    : FOR LPAREN IDENTIFIER COMMA IDENTIFIER IN expression RPAREN NEWLINE {
        for_val_name = $3;
        for_idx_name = $5;
        for_iter_expr = $7;
        symbol_table_enter_scope(symbol_table);
        TypeSpec* value_type = create_type_spec(TYPE_INT, NULL, NULL);
        TypeSpec* index_type = create_type_spec(TYPE_INT, NULL, NULL);
        symbol_table_insert(symbol_table, for_val_name, value_type, SYM_VAR, yylineno, NULL);
        symbol_table_insert(symbol_table, for_idx_name, index_type, SYM_VAR, yylineno, NULL);
    } for_body_block {
        $$ = create_for_stmt(for_val_name, for_idx_name, for_iter_expr, for_body, yylineno);
        free(for_val_name);
        free(for_idx_name);
        symbol_table_leave_scope(symbol_table);
        for_val_name = NULL;
        for_idx_name = NULL;
        for_iter_expr = NULL;
        for_body = NULL;
    }
    ;

for_body_block
    : indented_block DEDENT {
        for_body = $1 ? reverse_node_list($1) : NULL;
        $$ = NULL; // Not used, just to satisfy type requirement
    }
    ;

return_stmt
    : RETURN expression { $$ = create_return_stmt($2, yylineno); }
    | RETURN { $$ = create_return_stmt(NULL, yylineno); }
    ;

break_stmt
    : BREAK { $$ = create_break_stmt(yylineno); }
    ;

continue_stmt
    : CONTINUE { $$ = create_continue_stmt(yylineno); }
    ;

expr_stmt
    : expression { $$ = create_expr_stmt($1, yylineno); }
    ;

try_stmt
    : TRY NEWLINE indented_block DEDENT catch_list opt_finally {
        NodeList* t = $3 ? reverse_node_list($3) : NULL;
        NodeList* c = $5 ? reverse_node_list($5) : NULL;
        NodeList* f = $6 ? reverse_node_list($6) : NULL;
        $$ = create_try_stmt(t, c, f, yylineno);
    }
    ;

catch_list
    : catch_clause { $$ = create_node_list($1, NULL); }
    | catch_list catch_clause { $$ = create_node_list($2, $1); }
    ;

catch_clause
    : CATCH LPAREN type IDENTIFIER RPAREN NEWLINE indented_block DEDENT {
        NodeList* body = $7 ? reverse_node_list($7) : NULL;
        $$ = create_catch_clause($3, $4, body, yylineno);
        free($4);
    }
    ;

opt_finally
    : %empty { $$ = NULL; }
    | FINALLY NEWLINE indented_block DEDENT { $$ = $3; }
    ;

expression
    : or_expr
    ;

or_expr
    : and_expr
    | or_expr OR and_expr { $$ = create_binary_op(OP_OR, $1, $3, yylineno); }
    ;

and_expr
    : not_expr
    | and_expr AND not_expr { $$ = create_binary_op(OP_AND, $1, $3, yylineno); }
    ;

not_expr
    : rel_expr
    | NOT not_expr { $$ = create_unary_op(OP_NOT, $2, yylineno); }
    ;

rel_expr
    : add_expr
    | rel_expr rel_op add_expr { $$ = create_binary_op($2, $1, $3, yylineno); }
    ;

rel_op
    : EQUAL { $$ = OP_EQ; }
    | NOT_EQUAL { $$ = OP_NEQ; }
    | LESS { $$ = OP_LT; }
    | GREATER { $$ = OP_GT; }
    | LESS_EQUAL { $$ = OP_LE; }
    | GREATER_EQUAL { $$ = OP_GE; }
    | IN { $$ = OP_IN; }
    ;

add_expr
    : mult_expr
    | add_expr PLUS mult_expr { $$ = create_binary_op(OP_ADD, $1, $3, yylineno); }
    | add_expr MINUS mult_expr { $$ = create_binary_op(OP_SUB, $1, $3, yylineno); }
    ;

mult_expr
    : exp_expr
    | mult_expr STAR exp_expr { $$ = create_binary_op(OP_MUL, $1, $3, yylineno); }
    | mult_expr SLASH exp_expr { $$ = create_binary_op(OP_DIV, $1, $3, yylineno); }
    | mult_expr FLOOR_DIV exp_expr { $$ = create_binary_op(OP_FLOOR_DIV, $1, $3, yylineno); }
    | mult_expr MOD exp_expr { $$ = create_binary_op(OP_MOD, $1, $3, yylineno); }
    | mult_expr MATMUL exp_expr { $$ = create_binary_op(OP_MATMUL, $1, $3, yylineno); }
    ;

exp_expr
    : unary_expr
    | unary_expr POWER exp_expr { $$ = create_binary_op(OP_POWER, $1, $3, yylineno); }
    ;

unary_expr
    : postfix_expr
    | PLUS unary_expr %prec UNARY { $$ = create_unary_op(OP_UNARY_PLUS, $2, yylineno); }
    | MINUS unary_expr %prec UNARY { $$ = create_unary_op(OP_UNARY_MINUS, $2, yylineno); }
    | INCREMENT unary_expr { $$ = create_unary_op(OP_PRE_INC, $2, yylineno); }
    | DECREMENT unary_expr { $$ = create_unary_op(OP_PRE_DEC, $2, yylineno); }
    ;

postfix_expr
    : primary_expr
    | postfix_expr LBRACKET expression RBRACKET { $$ = create_array_access($1, $3, yylineno); }
    | postfix_expr LPAREN opt_args RPAREN {
        /* Verifica se o operando é um identificador */
        if ($1->type == NODE_IDENTIFIER) {
            char *func_name = $1->identifier.name;

            Symbol *sym = symbol_table_lookup(symbol_table, func_name);
            /* Relaxa verificação: permite forward references; só valida se houver assinatura (params != NULL) */
            int is_user_func = (sym && sym->kind == SYM_FUNC && sym->params != NULL);

            NodeList* args = $3 ? reverse_node_list($3) : NULL;
            // Apenas checa parâmetros se soubermos a assinatura
            if (is_user_func && sym) {
                int expected_count = count_params(sym->params);
                int actual_count   = count_nodelist(args);

                if (expected_count != actual_count) {
                    fprintf(stderr,
                            "Erro: função '%s' espera %d argumento(s), mas recebeu %d (linha %d)\n",
                            func_name, expected_count, actual_count, yylineno);
                    exit(1);
                }
            }

            $$ = create_call($1, args, yylineno);
            if (is_user_func && sym && sym->type) $$->inferred_type = sym->type;
        } else {
            NodeList* args = $3 ? reverse_node_list($3) : NULL;
            $$ = create_call($1, args, yylineno);
        }

    }
    | postfix_expr DOT IDENTIFIER %prec DOT { 
        $$ = create_member_access($1, $3, NULL, yylineno); 
        free($3); 
    }
    | postfix_expr DOT IDENTIFIER LPAREN opt_args RPAREN { 
        NodeList* args = $5 ? reverse_node_list($5) : NULL; 
        $$ = create_member_access($1, $3, args, yylineno); 
        free($3); 
    }
    | postfix_expr INCREMENT { $$ = create_unary_op(OP_POST_INC, $1, yylineno); }
    | postfix_expr DECREMENT { $$ = create_unary_op(OP_POST_DEC, $1, yylineno); }
    ;

primary_expr
    : literal
    | IDENTIFIER { 
        /* Aceita qualquer identificador - validação será feita no uso (permite forward reference) */
        $$ = create_identifier($1, yylineno);
        free($1);
    }
    | PRINT { $$ = create_identifier("print", yylineno); }
    | RANGE { $$ = create_identifier("range", yylineno); }
    | THIS { $$ = create_identifier("this", yylineno); }
    | LPAREN expression RPAREN { $$ = $2; }
    | array_literal
    | set_literal
    ;

literal
    : INT_LITERAL { $$ = create_int_literal($1, yylineno); }
    | FLOAT_LITERAL { $$ = create_float_literal($1, yylineno); }
    | TRUE { $$ = create_bool_literal(1, yylineno); }
    | FALSE { $$ = create_bool_literal(0, yylineno); }
    | COMPLEX_LITERAL { $$ = create_complex_literal($1, yylineno); free($1); }
    | STRING_LITERAL { $$ = create_string_literal($1, yylineno); free($1); }
    | CHAR_LITERAL { $$ = create_char_literal($1, yylineno); }
    ;

array_literal
    : LBRACKET RBRACKET { $$ = create_array_literal(NULL, yylineno); }
    | LBRACKET expression_list RBRACKET { NodeList* elems = reverse_node_list($2); $$ = create_array_literal(elems, yylineno); }
    ;

set_literal
    : LBRACE RBRACE { $$ = create_set_literal(NULL, yylineno); }
    | LBRACE expression_list RBRACE { NodeList* elems = reverse_node_list($2); $$ = create_set_literal(elems, yylineno); }
    ;

expression_list
    : expression { $$ = create_node_list($1, NULL); }
    | expression_list COMMA expression { $$ = create_node_list($3, $1); }
    ;

opt_args
    : %empty { $$ = NULL; }
    | expression_list { $$ = $1; }
    ;

%%

void yyerror(const char* s) {
    print_syntax_error(s ? s : "erro de sintaxe");
}

static NodeList* reverse_node_list(NodeList* list) {
    NodeList* prev = NULL;
    NodeList* cur = list;
    NodeList* nxt = NULL;
    while (cur) {
        nxt = cur->next;
        cur->next = prev;
        prev = cur;
        cur = nxt;
    }
    return prev;
}

static ParamList* reverse_param_list(ParamList* list) {
    ParamList* prev = NULL;
    ParamList* cur = list;
    ParamList* nxt = NULL;
    while (cur) {
        nxt = cur->next;
        cur->next = prev;
        prev = cur;
        cur = nxt;
    }

    return prev;
}

void undeclared_variable_error(const char* var_name, int line) {
    fprintf(stderr, "Erro semântico na linha %d: variável '%s' não declarada.\n", line, var_name);
}

void already_declared_variable_error(char* var_name, int line) {
    if(symbol_table_lookup(symbol_table, var_name)) {
        char * out = cat(var_name, " already declared!", "", "", "");
        yyerror(out);
        // free(out);
    }
}

void type_error(char* t1, char* t2, int line) {
    if (strcmp(t1, t2) != 0 && !(strcmp(t1, "") == 0 || strcmp(t2, "") == 0)) {
        char *s = cat("[TYPE ERROR]: ", t1, " and ", t2, " are incompatible");
        yyerror(s);
        // free(s);
    }
}

char * cat(char* t1, char* t2, char* t3, char* t4, char* t5) {
    int len = strlen(t1) + strlen(t2) + strlen(t3) + strlen(t4) + strlen(t5) + 1;
    char * result = (char *)malloc(len * sizeof(char));
    if (!result) {
        fprintf(stderr, "Memory allocation failed in cat function.\n");
        exit(1);
    }

    sprintf(result, "%s%s%s%s%s", t1, t2, t3, t4, t5);
    return result;
}

void check_variables(const char* var_name, int line) {
    if (find_variable(var_name) == NULL) {
        undeclared_variable_error(var_name, line);
        exit(1);
    }
}

void vars_routine(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_DECLARATION: {
            check_variables(node->declaration.name, node->line);
            break;
        }
        default:
            break;
    }
}

int count_params(ParamList *p) {
    int count = 0;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}

int count_nodelist(NodeList *n) {
    int count = 0;
    while (n != NULL) {
        /* fprintf(stderr, "Param nodelist: %s\n", n->node->identifier.name); */
        count++;
        n = n->next;
    }
    return count;
}

const char *datatype_to_string(DataType t) {
    switch (t) {
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