%{
#include <stdio.h>

int line_number = 1;
int column_number = 1;

void update_column() {
    column_number += yyleng;
}

void report_token(const char* token_type, const char* lexeme) {
    printf("%-20s | %-30s | Line: %d, Col: %d\n",
           token_type, lexeme, line_number, column_number);
    update_column();
}
%}

%%

    /* ======================================================================== */
    /* SEÇÃO 1: WHITESPACE E NEWLINES */
    /* ======================================================================== */

\n {
    line_number++;
    column_number = 1;
}

[ \t]+ {
    update_column();
}

    /* ======================================================================== */
    /* SEÇÃO 2: COMENTÁRIOS */
    /* ======================================================================== */

"//".*         { update_column(); }

"#".*          { update_column(); }

"/*"([^*]|"*"+[^*/])*"*"+"/" {
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') {
            line_number++;
            column_number = 1;
        } else {
            column_number++;
        }
    }
}

    /* ======================================================================== */
    /* SEÇÃO 3: PALAVRAS-CHAVE */
    /* ======================================================================== */

"def"      { report_token("DEF", yytext); }
"end"      { report_token("END", yytext); }
"fun"      { report_token("FUN", yytext); }
"if"       { report_token("IF", yytext); }
"elif"     { report_token("ELIF", yytext); }
"else"     { report_token("ELSE", yytext); }
"while"    { report_token("WHILE", yytext); }
"for"      { report_token("FOR", yytext); }
"in"       { report_token("IN", yytext); }
"return"   { report_token("RETURN", yytext); }
"break"    { report_token("BREAK", yytext); }
"continue" { report_token("CONTINUE", yytext); }
"try"      { report_token("TRY", yytext); }
"catch"    { report_token("CATCH", yytext); }
"finally"  { report_token("FINALLY", yytext); }
"import"   { report_token("IMPORT", yytext); }
"const"    { report_token("CONST", yytext); }
"var"      { report_token("VAR", yytext); }
"this"     { report_token("THIS", yytext); }
"new"      { report_token("NEW", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 4: TIPOS PRIMITIVOS */
    /* ======================================================================== */

"int"      { report_token("INT_TYPE", yytext); }
"float"    { report_token("FLOAT_TYPE", yytext); }
"bool"     { report_token("BOOL_TYPE", yytext); }
"complex"  { report_token("COMPLEX_TYPE", yytext); }
"char"     { report_token("CHAR_TYPE", yytext); }
"string"   { report_token("STRING_TYPE", yytext); }
"void"     { report_token("VOID_TYPE", yytext); }
"matrix"   { report_token("MATRIX_TYPE", yytext); }
"set"      { report_token("SET_TYPE", yytext); }
"graph"    { report_token("GRAPH_TYPE", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 5: LITERAIS BOOLEANOS E NULL */
    /* ======================================================================== */

"true"     { report_token("TRUE", yytext); }
"false"    { report_token("FALSE", yytext); }
"null"     { report_token("NULL", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 6: OPERADORES LÓGICOS */
    /* ======================================================================== */

"and"      { report_token("AND", yytext); }
"or"       { report_token("OR", yytext); }
"not"      { report_token("NOT", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 7: FUNÇÕES MATEMÁTICAS */
    /* ======================================================================== */

"sin"|"sen"     { report_token("SIN", yytext); }
"cos"           { report_token("COS", yytext); }
"tan"           { report_token("TAN", yytext); }
"sec"           { report_token("SEC", yytext); }
"csc"|"cosec"   { report_token("CSC", yytext); }
"cot"|"cotan"   { report_token("COT", yytext); }
"asin"|"arcsin" { report_token("ASIN", yytext); }
"acos"|"arccos" { report_token("ACOS", yytext); }
"atan"|"arctan" { report_token("ATAN", yytext); }
"sqrt"          { report_token("SQRT", yytext); }
"abs"           { report_token("ABS", yytext); }
"log"           { report_token("LOG", yytext); }
"ln"            { report_token("LN", yytext); }
"exp"           { report_token("EXP", yytext); }

    /* Funções de arrays/listas */
"length"   { report_token("LENGTH", yytext); }
"add"      { report_token("ADD", yytext); }
"push"     { report_token("PUSH", yytext); }
"pop"      { report_token("POP", yytext); }
"remove"   { report_token("REMOVE", yytext); }
"insert"   { report_token("INSERT", yytext); }
"clear"    { report_token("CLEAR", yytext); }
"isEmpty"  { report_token("IS_EMPTY", yytext); }

    /* Funções de matrizes */
"transpose"   { report_token("TRANSPOSE", yytext); }
"determinant" { report_token("DETERMINANT", yytext); }
"inverse"     { report_token("INVERSE", yytext); }
"rows"        { report_token("ROWS", yytext); }
"cols"        { report_token("COLS", yytext); }

    /* Funções de grafos */
"nodeCount"  { report_token("NODE_COUNT", yytext); }
"neighbors"  { report_token("NEIGHBORS", yytext); }
"weight"     { report_token("WEIGHT", yytext); }

    /* Funções de I/O */
"print"    { report_token("PRINT", yytext); }
"println"  { report_token("PRINTLN", yytext); }
"read"     { report_token("READ", yytext); }
"range"    { report_token("RANGE", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 8: LITERAIS NUMÉRICOS */
    /* ======================================================================== */

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)? {
    report_token("FLOAT_LITERAL", yytext);
}

[0-9]+[eE][+-]?[0-9]+ {
    report_token("FLOAT_LITERAL", yytext);
}

[0-9]+ {
    report_token("INT_LITERAL", yytext);
}

([0-9]+|[0-9]+\.[0-9]+)[+-][0-9]+[ij] {
    report_token("COMPLEX_LITERAL", yytext);
}

([0-9]+|[0-9]+\.[0-9]+)[ij] {
    report_token("COMPLEX_LITERAL", yytext);
}

    /* ======================================================================== */
    /* SEÇÃO 9: LITERAIS DE STRING E CHAR */
    /* ======================================================================== */

\"([^\"\\]|\\.)*\" {
    report_token("STRING_LITERAL", yytext);
}

\'([^\'\\]|\\.)\' {
    report_token("CHAR_LITERAL", yytext);
}

    /* ======================================================================== */
    /* SEÇÃO 10: IDENTIFICADORES */
    /* ======================================================================== */

[a-zA-Z_][a-zA-Z0-9_]* {
    report_token("IDENTIFIER", yytext);
}

    /* ======================================================================== */
    /* SEÇÃO 11: OPERADORES */
    /* ======================================================================== */

    /* Operadores aritméticos */
"+"  { report_token("PLUS", yytext); }
"-"  { report_token("MINUS", yytext); }
"*"  { report_token("STAR", yytext); }
"/"  { report_token("SLASH", yytext); }
"//" { report_token("FLOOR_DIV", yytext); }
"%"  { report_token("MOD", yytext); }
"^"  { report_token("POWER", yytext); }
"@"  { report_token("MATMUL", yytext); }

    /* Operadores de atribuição */
"="  { report_token("ASSIGN", yytext); }
"+=" { report_token("PLUS_ASSIGN", yytext); }
"-=" { report_token("MINUS_ASSIGN", yytext); }
"*=" { report_token("STAR_ASSIGN", yytext); }
"/=" { report_token("SLASH_ASSIGN", yytext); }
"%=" { report_token("MOD_ASSIGN", yytext); }

    /* Operadores de incremento/decremento */
"++" { report_token("INCREMENT", yytext); }
"--" { report_token("DECREMENT", yytext); }

    /* Operadores relacionais */
"==" { report_token("EQUAL", yytext); }
"!=" { report_token("NOT_EQUAL", yytext); }
"<"  { report_token("LESS", yytext); }
">"  { report_token("GREATER", yytext); }
"<=" { report_token("LESS_EQUAL", yytext); }
">=" { report_token("GREATER_EQUAL", yytext); }

    /* Operadores lógicos (símbolos) */
"&&" { report_token("LOGICAL_AND", yytext); }
"||" { report_token("LOGICAL_OR", yytext); }
"!"  { report_token("LOGICAL_NOT", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 12: DELIMITADORES E PONTUAÇÃO */
    /* ======================================================================== */

"("  { report_token("LPAREN", yytext); }
")"  { report_token("RPAREN", yytext); }
"["  { report_token("LBRACKET", yytext); }
"]"  { report_token("RBRACKET", yytext); }
"{"  { report_token("LBRACE", yytext); }
"}"  { report_token("RBRACE", yytext); }
","  { report_token("COMMA", yytext); }
";"  { report_token("SEMICOLON", yytext); }
":"  { report_token("COLON", yytext); }
"."  { report_token("DOT", yytext); }

    /* ======================================================================== */
    /* SEÇÃO 13: TRATAMENTO DE ERROS */
    /* ======================================================================== */

. {
    fprintf(stderr, "ERRO LÉXICO: Caractere inválido '%s' na linha %d, coluna %d\n",
            yytext, line_number, column_number);
    update_column();
}

<<EOF>> {
    return 0;
}

%%

int yywrap(void) {
    return 1;
}

int main(int argc, char** argv) {
    FILE* input_file = NULL;

    printf("============================================================================\n");
    printf("ANALISADOR LÉXICO - LINGUAGEM MATEMÁTICA\n");
    printf("============================================================================\n");
    printf("%-20s | %-30s | Posição\n", "TOKEN", "LEXEMA");
    printf("----------------------------------------------------------------------------\n");

    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            fprintf(stderr, "ERRO: Não foi possível abrir o arquivo '%s'\n", argv[1]);
            return 1;
        }
        yyin = input_file;
    } else {
        printf("Lendo da entrada padrão...\n\n");
    }

    yylex();

    printf("============================================================================\n");
    printf("Análise léxica concluída com sucesso!\n");
    printf("Total de linhas processadas: %d\n", line_number);
    printf("============================================================================\n");

    if (input_file) {
        fclose(input_file);
    }

    return 0;
}