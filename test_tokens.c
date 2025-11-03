#include <stdio.h>
#include <stdlib.h>

#include "parser.tab.h"

extern FILE* yyin;
extern int yylex();
extern int yylineno;
extern char* yytext;

const char* token_name(int token) {
    switch(token) {
        case DEF: return "DEF";
        case END: return "END";
        case FUN: return "FUN";
        case IF: return "IF";
        case ELIF: return "ELIF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case FOR: return "FOR";
        case IN: return "IN";
        case RETURN: return "RETURN";
        case BREAK: return "BREAK";
        case CONTINUE: return "CONTINUE";
        case TRY: return "TRY";
        case CATCH: return "CATCH";
        case FINALLY: return "FINALLY";
        case IMPORT: return "IMPORT";
        case CONST: return "CONST";
        case THIS: return "THIS";
        case NEW: return "NEW";
        case INT_TYPE: return "INT_TYPE";
        case FLOAT_TYPE: return "FLOAT_TYPE";
        case BOOL_TYPE: return "BOOL_TYPE";
        case COMPLEX_TYPE: return "COMPLEX_TYPE";
        case CHAR_TYPE: return "CHAR_TYPE";
        case STRING_TYPE: return "STRING_TYPE";
        case VOID_TYPE: return "VOID_TYPE";
        case MATRIX_TYPE: return "MATRIX_TYPE";
        case SET_TYPE: return "SET_TYPE";
        case GRAPH_TYPE: return "GRAPH_TYPE";
        case TRUE: return "TRUE";
        case FALSE: return "FALSE";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        case PRINT: return "PRINT";
        case RANGE: return "RANGE";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case STAR: return "STAR";
        case SLASH: return "SLASH";
        case FLOOR_DIV: return "FLOOR_DIV";
        case MOD: return "MOD";
        case POWER: return "POWER";
        case MATMUL: return "MATMUL";
        case ASSIGN: return "ASSIGN";
        case PLUS_ASSIGN: return "PLUS_ASSIGN";
        case MINUS_ASSIGN: return "MINUS_ASSIGN";
        case STAR_ASSIGN: return "STAR_ASSIGN";
        case SLASH_ASSIGN: return "SLASH_ASSIGN";
        case MOD_ASSIGN: return "MOD_ASSIGN";
        case INCREMENT: return "INCREMENT";
        case DECREMENT: return "DECREMENT";
        case EQUAL: return "EQUAL";
        case NOT_EQUAL: return "NOT_EQUAL";
        case LESS: return "LESS";
        case GREATER: return "GREATER";
        case LESS_EQUAL: return "LESS_EQUAL";
        case GREATER_EQUAL: return "GREATER_EQUAL";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case COMMA: return "COMMA";
        case COLON: return "COLON";
        case DOT: return "DOT";
        case NEWLINE: return "NEWLINE";
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
        case IDENTIFIER: return "IDENTIFIER";
        case INT_LITERAL: return "INT_LITERAL";
        case FLOAT_LITERAL: return "FLOAT_LITERAL";
        case STRING_LITERAL: return "STRING_LITERAL";
        case CHAR_LITERAL: return "CHAR_LITERAL";
        case COMPLEX_LITERAL: return "COMPLEX_LITERAL";
        default: return "UNKNOWN";
    }
}

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Erro ao abrir arquivo: %s\n", argv[1]);
            return 1;
        }
    }

    printf("=== ANÁLISE LÉXICA (TOKENS) ===\n");
    printf("%-5s %-20s %-30s\n", "Linha", "Token", "Lexema");
    printf("---------------------------------------------------------------\n");

    int token;
    while ((token = yylex()) != 0) {
        printf("%-5d %-20s '%s'\n", yylineno, token_name(token), yytext);
    }

    if (argc > 1) {
        fclose(yyin);
    }

    return 0;
}