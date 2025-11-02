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
        case VOID_TYPE: return "VOID_TYPE";
        case IMPORT: return "IMPORT";
        case PRINT: return "PRINT";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case NEWLINE: return "NEWLINE";
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
        case IDENTIFIER: return "IDENTIFIER";
        case STRING_LITERAL: return "STRING_LITERAL";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "TOKEN(%d)", token);
            return buf;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    int token;
    printf("%-20s %-30s %s\n", "TOKEN", "LEXEMA", "LINHA");
    printf("================================================================\n");
    
    while ((token = yylex()) != 0) {
        printf("%-20s %-30s %d\n", token_name(token), yytext, yylineno);
    }

    fclose(yyin);
    return 0;
}