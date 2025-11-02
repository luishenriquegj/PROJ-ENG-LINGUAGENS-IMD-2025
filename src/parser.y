%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

ASTNode* ast_root = NULL;
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
%token TRY CATCH FINALLY IMPORT CONST THIS NEW PRINT RANGE
%token INT_TYPE FLOAT_TYPE BOOL_TYPE COMPLEX_TYPE CHAR_TYPE STRING_TYPE
%token VOID_TYPE MATRIX_TYPE SET_TYPE GRAPH_TYPE
%token PLUS MINUS STAR SLASH FLOOR_DIV MOD POWER MATMUL
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MOD_ASSIGN
%token INCREMENT DECREMENT
%token EQUAL NOT_EQUAL LESS GREATER LESS_EQUAL GREATER_EQUAL
%token AND OR NOT
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token COMMA COLON DOT
%token NEWLINE INDENT DEDENT

%type <node> program import definition function_def type_def main_function
%type <node> statement declaration assignment if_stmt while_stmt for_stmt
%type <node> return_stmt break_stmt continue_stmt expr_stmt try_stmt
%type <node> catch_clause expression or_expr and_expr not_expr rel_expr
%type <node> add_expr mult_expr exp_expr unary_expr postfix_expr primary_expr
%type <node> literal array_literal matrix_literal set_literal

%type <node_list> import_list definition_list statement_list statement_block
%type <node_list> elif_list catch_list expression_list array_row_list
%type <node_list> opt_args opt_else opt_finally opt_imports opt_definitions

%type <param_list> param_list params

%type <type_spec> type primitive_type array_type matrix_type set_type graph_type

%type <op> assign_op rel_op

%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN MOD_ASSIGN
%left OR
%left AND
%right NOT
%left EQUAL NOT_EQUAL
%left LESS GREATER LESS_EQUAL GREATER_EQUAL IN
%left PLUS MINUS
%left STAR SLASH FLOOR_DIV MOD MATMUL
%right POWER
%right UNARY
%left DOT LBRACKET LPAREN INCREMENT DECREMENT

%%

program
    : opt_imports opt_definitions main_function {
        ast_root = create_program($1, $2, $3, yylineno);
    }
    ;

opt_imports
    : import_list
        { $$ = $1; }
    | %empty
        { $$ = NULL; }
    ;

opt_definitions
    : definition_list
        { $$ = $1; }
    | %empty
        { $$ = NULL; }
    ;

import_list
    : import
        { $$ = create_node_list($1, NULL); }
    | import import_list
        { $$ = create_node_list($1, $2); }
    ;

import
    : IMPORT IDENTIFIER NEWLINE
        { $$ = create_import($2, yylineno); free($2); }
    ;

definition_list
    : definition
        { $$ = create_node_list($1, NULL); }
    | definition definition_list
        { $$ = create_node_list($1, $2); }
    ;

definition
    : function_def
    | type_def
    ;

main_function
    : DEF FUN VOID_TYPE IDENTIFIER LPAREN RPAREN NEWLINE
      INDENT statement_list DEDENT
      END DEF NEWLINE {
        TypeSpec* void_type = create_type_spec(TYPE_VOID, NULL, NULL);
        $$ = create_function_def(void_type, $4, NULL, $9, yylineno);
        free($4);
    }
    ;

function_def
    : DEF FUN type IDENTIFIER LPAREN params RPAREN NEWLINE
      INDENT statement_list DEDENT
      END DEF NEWLINE {
        $$ = create_function_def($3, $4, $6, $10, yylineno);
        free($4);
    }
    ;

type_def
    : DEF IDENTIFIER NEWLINE
      INDENT statement_list DEDENT
      END DEF NEWLINE {
        $$ = create_type_def($2, $5, yylineno);
        free($2);
    }
    ;

params
    : param_list
        { $$ = $1; }
    | %empty
        { $$ = NULL; }
    ;

param_list
    : type IDENTIFIER
        { $$ = create_param_list($1, $2, NULL); free($2); }
    | type IDENTIFIER COMMA param_list
        { $$ = create_param_list($1, $2, $4); free($2); }
    ;

type
    : primitive_type
    | array_type
    | matrix_type
    | set_type
    | graph_type
    | IDENTIFIER
        { $$ = create_type_spec(TYPE_CUSTOM, NULL, $1); }
    ;

primitive_type
    : INT_TYPE { $$ = create_type_spec(TYPE_INT, NULL, NULL); }
    | FLOAT_TYPE { $$ = create_type_spec(TYPE_FLOAT, NULL, NULL); }
    | BOOL_TYPE { $$ = create_type_spec(TYPE_BOOL, NULL, NULL); }
    | COMPLEX_TYPE { $$ = create_type_spec(TYPE_COMPLEX, NULL, NULL); }
    | CHAR_TYPE { $$ = create_type_spec(TYPE_CHAR, NULL, NULL); }
    | STRING_TYPE { $$ = create_type_spec(TYPE_STRING, NULL, NULL); }
    | VOID_TYPE { $$ = create_type_spec(TYPE_VOID, NULL, NULL); }
    ;

array_type
    : primitive_type LBRACKET RBRACKET
        { $$ = create_type_spec(TYPE_ARRAY, $1, NULL); }
    ;

matrix_type
    : MATRIX_TYPE LBRACKET primitive_type RBRACKET
        { $$ = create_type_spec(TYPE_MATRIX, $3, NULL); }
    ;

set_type
    : SET_TYPE LBRACKET primitive_type RBRACKET
        { $$ = create_type_spec(TYPE_SET, $3, NULL); }
    ;

graph_type
    : GRAPH_TYPE LBRACKET primitive_type RBRACKET
        { $$ = create_type_spec(TYPE_GRAPH, $3, NULL); }
    ;

statement_list
    : statement
        { $$ = $1 ? create_node_list($1, NULL) : NULL; }
    | statement statement_list
        { $$ = $1 ? create_node_list($1, $2) : $2; }
    ;

statement_block
    : INDENT statement_list DEDENT
        { $$ = $2; }
    ;

statement
    : declaration NEWLINE
        { $$ = $1; }
    | assignment NEWLINE
        { $$ = $1; }
    | if_stmt
        { $$ = $1; }
    | while_stmt
        { $$ = $1; }
    | for_stmt
        { $$ = $1; }
    | return_stmt NEWLINE
        { $$ = $1; }
    | break_stmt NEWLINE
        { $$ = $1; }
    | continue_stmt NEWLINE
        { $$ = $1; }
    | expr_stmt NEWLINE
        { $$ = $1; }
    | try_stmt
        { $$ = $1; }
    | NEWLINE
        { $$ = NULL; }
    ;

declaration
    : type IDENTIFIER ASSIGN expression
        { $$ = create_declaration($1, $2, $4, yylineno); free($2); }
    | type IDENTIFIER
        { $$ = create_declaration($1, $2, NULL, yylineno); free($2); }
    | CONST IDENTIFIER ASSIGN expression {
        TypeSpec* type = create_type_spec(TYPE_INT, NULL, NULL);
        $$ = create_declaration(type, $2, $4, yylineno);
        free($2);
    }
    ;

assignment
    : IDENTIFIER assign_op expression {
        ASTNode* id = create_identifier($1, yylineno);
        $$ = create_assignment(id, $2, $3, yylineno);
        free($1);
    }
    | postfix_expr assign_op expression {
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
    : IF LPAREN expression RPAREN NEWLINE
      statement_block
      elif_list
      opt_else {
        $$ = create_if_stmt($3, $6, $7, $8, yylineno);
    }
    ;

elif_list
    : ELIF LPAREN expression RPAREN NEWLINE statement_block elif_list {
        ASTNode* elif = create_if_stmt($3, $6, NULL, NULL, yylineno);
        $$ = create_node_list(elif, $7);
    }
    | %empty
        { $$ = NULL; }
    ;

opt_else
    : ELSE NEWLINE statement_block
        { $$ = $3; }
    | %empty
        { $$ = NULL; }
    ;

while_stmt
    : WHILE LPAREN expression RPAREN NEWLINE statement_block {
        $$ = create_while_stmt($3, $6, yylineno);
    }
    ;

for_stmt
    : FOR LPAREN IDENTIFIER IN expression RPAREN NEWLINE statement_block {
        $$ = create_for_stmt($3, NULL, $5, $8, yylineno);
        free($3);
    }
    | FOR LPAREN IDENTIFIER COMMA IDENTIFIER IN expression RPAREN NEWLINE statement_block {
        $$ = create_for_stmt($3, $5, $7, $10, yylineno);
        free($3);
        free($5);
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
    : TRY NEWLINE statement_block catch_list opt_finally {
        $$ = create_try_stmt($3, $4, $5, yylineno);
    }
    ;

catch_list
    : catch_clause
        { $$ = create_node_list($1, NULL); }
    | catch_clause catch_list
        { $$ = create_node_list($1, $2); }
    ;

catch_clause
    : CATCH LPAREN type IDENTIFIER RPAREN NEWLINE statement_block {
        $$ = create_catch_clause($3, $4, $7, yylineno);
        free($4);
    }
    ;

opt_finally
    : FINALLY NEWLINE statement_block
        { $$ = $3; }
    | %empty
        { $$ = NULL; }
    ;

expression
    : or_expr
        { $$ = $1; }
    ;

or_expr
    : and_expr
    | or_expr OR and_expr
        { $$ = create_binary_op(OP_OR, $1, $3, yylineno); }
    ;

and_expr
    : not_expr
    | and_expr AND not_expr
        { $$ = create_binary_op(OP_AND, $1, $3, yylineno); }
    ;

not_expr
    : rel_expr
    | NOT not_expr
        { $$ = create_unary_op(OP_NOT, $2, yylineno); }
    ;

rel_expr
    : add_expr
    | rel_expr rel_op add_expr
        { $$ = create_binary_op($2, $1, $3, yylineno); }
    ;

rel_op
    : EQUAL { $$ = OP_EQ; }
    | NOT_EQUAL { $$ = OP_NEQ; }
    | LESS { $$ = OP_LT; }
    | GREATER { $$ = OP_GT; }
    | LESS_EQUAL { $$ = OP_LE; }
    | GREATER_EQUAL { $$ = OP_GE; }
    ;

add_expr
    : mult_expr
    | add_expr PLUS mult_expr
        { $$ = create_binary_op(OP_ADD, $1, $3, yylineno); }
    | add_expr MINUS mult_expr
        { $$ = create_binary_op(OP_SUB, $1, $3, yylineno); }
    ;

mult_expr
    : exp_expr
    | mult_expr STAR exp_expr
        { $$ = create_binary_op(OP_MUL, $1, $3, yylineno); }
    | mult_expr SLASH exp_expr
        { $$ = create_binary_op(OP_DIV, $1, $3, yylineno); }
    | mult_expr FLOOR_DIV exp_expr
        { $$ = create_binary_op(OP_FLOOR_DIV, $1, $3, yylineno); }
    | mult_expr MOD exp_expr
        { $$ = create_binary_op(OP_MOD, $1, $3, yylineno); }
    | mult_expr MATMUL exp_expr
        { $$ = create_binary_op(OP_MATMUL, $1, $3, yylineno); }
    ;

exp_expr
    : unary_expr
    | unary_expr POWER exp_expr
        { $$ = create_binary_op(OP_POWER, $1, $3, yylineno); }
    ;

unary_expr
    : postfix_expr
    | PLUS unary_expr %prec UNARY
        { $$ = create_unary_op(OP_UNARY_PLUS, $2, yylineno); }
    | MINUS unary_expr %prec UNARY
        { $$ = create_unary_op(OP_UNARY_MINUS, $2, yylineno); }
    | INCREMENT unary_expr
        { $$ = create_unary_op(OP_PRE_INC, $2, yylineno); }
    | DECREMENT unary_expr
        { $$ = create_unary_op(OP_PRE_DEC, $2, yylineno); }
    ;

postfix_expr
    : primary_expr
    | postfix_expr LBRACKET expression RBRACKET
        { $$ = create_array_access($1, $3, yylineno); }
    | postfix_expr LPAREN opt_args RPAREN
        { $$ = create_call($1, $3, yylineno); }
    | postfix_expr DOT IDENTIFIER
        { $$ = create_member_access($1, $3, NULL, yylineno); free($3); }
    | postfix_expr DOT IDENTIFIER LPAREN opt_args RPAREN
        { $$ = create_member_access($1, $3, $5, yylineno); free($3); }
    | postfix_expr INCREMENT
        { $$ = create_unary_op(OP_POST_INC, $1, yylineno); }
    | postfix_expr DECREMENT
        { $$ = create_unary_op(OP_POST_DEC, $1, yylineno); }
    ;

primary_expr
    : literal
    | IDENTIFIER
        { $$ = create_identifier($1, yylineno); free($1); }
    | THIS
        { $$ = create_identifier("this", yylineno); }
    | PRINT
        { $$ = create_identifier("print", yylineno); }
    | RANGE
        { $$ = create_identifier("range", yylineno); }
    | LPAREN expression RPAREN
        { $$ = $2; }
    | array_literal
    | matrix_literal
    | set_literal
    ;

literal
    : INT_LITERAL
        { $$ = create_int_literal($1, yylineno); }
    | FLOAT_LITERAL
        { $$ = create_float_literal($1, yylineno); }
    | TRUE
        { $$ = create_bool_literal(1, yylineno); }
    | FALSE
        { $$ = create_bool_literal(0, yylineno); }
    | COMPLEX_LITERAL
        { $$ = create_complex_literal($1, yylineno); }
    | STRING_LITERAL
        { $$ = create_string_literal($1, yylineno); }
    | CHAR_LITERAL
        { $$ = create_char_literal($1, yylineno); }
    ;

array_literal
    : LBRACKET expression_list RBRACKET
        { $$ = create_array_literal($2, yylineno); }
    | LBRACKET RBRACKET
        { $$ = create_array_literal(NULL, yylineno); }
    ;

matrix_literal
    : LBRACKET array_row_list RBRACKET
        { $$ = create_matrix_literal($2, yylineno); }
    ;

array_row_list
    : array_literal
        { $$ = create_node_list($1, NULL); }
    | array_literal COMMA array_row_list
        { $$ = create_node_list($1, $3); }
    ;

set_literal
    : LBRACE expression_list RBRACE
        { $$ = create_set_literal($2, yylineno); }
    | LBRACE RBRACE
        { $$ = create_set_literal(NULL, yylineno); }
    ;

expression_list
    : expression
        { $$ = create_node_list($1, NULL); }
    | expression COMMA expression_list
        { $$ = create_node_list($1, $3); }
    ;

opt_args
    : expression_list
        { $$ = $1; }
    | %empty
        { $$ = NULL; }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Erro sintático na linha %d: %s\n", yylineno, s);
}