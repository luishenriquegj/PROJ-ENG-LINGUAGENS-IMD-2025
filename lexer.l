%{
/*
 * ============================================================================
 * ANALISADOR LÉXICO - LINGUAGEM MATEMÁTICA
 * ============================================================================
 * Disciplina: DIM0548 - Engenharia de Linguagens
 * Grupo: ALLANE EMANOELLE, FRANKLIN OLIVEIRA, LUIS HENRIQUE, MATEUS SANTOS, MARCOS MARTINS
 *
 * Descrição:
 *   Analisador léxico para linguagem de programação voltada ao domínio
 *   matemático. Reconhece tokens definidos na gramática BNF especificada
 *   nos documentos do projeto.
 *
 * Características:
 *   - Tipagem forte com inferência
 *   - Suporte a tipos matemáticos (complex, matrix, set, graph)
 *   - Indentação como delimitador de blocos
 *   - Operadores matemáticos especializados
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Contador de linha para mensagens de erro */
int line_number = 1;
int column_number = 1;

/* Stack para gerenciar níveis de indentação */
#define MAX_INDENT_LEVELS 100
int indent_stack[MAX_INDENT_LEVELS];
int indent_stack_ptr = 0;
int current_indent = 0;
int at_line_start = 1;

/* Função para imprimir os tokens */
void report_token(const char* token_type, const char* lexeme) {
    printf("%-20s | %-30s | Line: %d, Col: %d\n",
           token_type, lexeme, line_number, column_number);
}

/* Atualizar posição da coluna */
void update_column() {
    column_number += yyleng;
}

%}

/* Definições de padrões reutilizáveis */
DIGIT       [0-9]
LETTER      [a-zA-Z]
UNDERSCORE  _
ID_START    {LETTER}
ID_CHAR     ({LETTER}|{DIGIT}|{UNDERSCORE})

%%

    /* ========================================================================
     * SEÇÃO 1: WHITESPACE E NEWLINES (Processamento de Indentação)
     * ======================================================================== */

^[ \t]*\n   {
    /* Linha vazia - ignora mas incrementa contador */
    line_number++;
    column_number = 1;
    at_line_start = 1;
}

^[ \t]*     {
    /* Início de linha - processa indentação */
    if (at_line_start) {
        int spaces = 0;
        for (int i = 0; i < yyleng; i++) {
            if (yytext[i] == ' ') spaces++;
            else if (yytext[i] == '\t') spaces += 4; /* Tab = 4 espaços */
        }

        current_indent = spaces / 4; /* Cada nível = 4 espaços */

        /* Emite INDENT ou DEDENT conforme necessário */
        if (indent_stack_ptr == 0) {
            indent_stack[indent_stack_ptr++] = 0;
        }

        if (current_indent > indent_stack[indent_stack_ptr - 1]) {
            /* Aumentou indentação */
            indent_stack[indent_stack_ptr++] = current_indent;
            report_token("INDENT", "");
        } else if (current_indent < indent_stack[indent_stack_ptr - 1]) {
            /* Diminuiu indentação */
            while (indent_stack_ptr > 0 &&
                   indent_stack[indent_stack_ptr - 1] > current_indent) {
                indent_stack_ptr--;
                report_token("DEDENT", "");
            }
        }

        at_line_start = 0;
        column_number += yyleng;
    }
}

\n          {
    line_number++;
    column_number = 1;
    at_line_start = 1;
    report_token("NEWLINE", "\\n");
}

[ \t]+      {
    /* Whitespace no meio da linha - ignora */
    update_column();
}

    /* ========================================================================
     * SEÇÃO 2: COMENTÁRIOS
     * ======================================================================== */

"//".*              { update_column(); /* Comentário de linha - ignora */ }
"#".*               { update_column(); /* Comentário estilo Python - ignora */ }
"/*"([^*]|"*"+[^*/])*"*"+"/"  {
    /* Comentário de bloco - conta linhas internas */
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') {
            line_number++;
            column_number = 1;
        } else {
            column_number++;
        }
    }
}

    /* ========================================================================
     * SEÇÃO 3: PALAVRAS-CHAVE (Keywords)
     * ======================================================================== */

"def"       { report_token("DEF", yytext); update_column(); }
"end"       { report_token("END", yytext); update_column(); }
"fun"       { report_token("FUN", yytext); update_column(); }
"if"        { report_token("IF", yytext); update_column(); }
"elif"      { report_token("ELIF", yytext); update_column(); }
"else"      { report_token("ELSE", yytext); update_column(); }
"while"     { report_token("WHILE", yytext); update_column(); }
"for"       { report_token("FOR", yytext); update_column(); }
"in"        { report_token("IN", yytext); update_column(); }
"return"    { report_token("RETURN", yytext); update_column(); }
"break"     { report_token("BREAK", yytext); update_column(); }
"continue"  { report_token("CONTINUE", yytext); update_column(); }
"try"       { report_token("TRY", yytext); update_column(); }
"catch"     { report_token("CATCH", yytext); update_column(); }
"finally"   { report_token("FINALLY", yytext); update_column(); }
"import"    { report_token("IMPORT", yytext); update_column(); }
"const"     { report_token("CONST", yytext); update_column(); }
"var"       { report_token("VAR", yytext); update_column(); }
"this"      { report_token("THIS", yytext); update_column(); }
"new"       { report_token("NEW", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 4: TIPOS PRIMITIVOS
     * ======================================================================== */

"int"       { report_token("INT_TYPE", yytext); update_column(); }
"float"     { report_token("FLOAT_TYPE", yytext); update_column(); }
"bool"      { report_token("BOOL_TYPE", yytext); update_column(); }
"complex"   { report_token("COMPLEX_TYPE", yytext); update_column(); }
"char"      { report_token("CHAR_TYPE", yytext); update_column(); }
"string"    { report_token("STRING_TYPE", yytext); update_column(); }
"void"      { report_token("VOID_TYPE", yytext); update_column(); }
"matrix"    { report_token("MATRIX_TYPE", yytext); update_column(); }
"set"       { report_token("SET_TYPE", yytext); update_column(); }
"graph"     { report_token("GRAPH_TYPE", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 5: LITERAIS BOOLEANOS E NULL
     * ======================================================================== */

"true"      { report_token("TRUE", yytext); update_column(); }
"false"     { report_token("FALSE", yytext); update_column(); }
"null"      { report_token("NULL", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 6: OPERADORES LÓGICOS
     * ======================================================================== */

"and"       { report_token("AND", yytext); update_column(); }
"or"        { report_token("OR", yytext); update_column(); }
"not"       { report_token("NOT", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 7: FUNÇÕES MATEMÁTICAS EMBUTIDAS
     * ======================================================================== */

"sin"|"sen"     { report_token("SIN", yytext); update_column(); }
"cos"           { report_token("COS", yytext); update_column(); }
"tan"           { report_token("TAN", yytext); update_column(); }
"sec"           { report_token("SEC", yytext); update_column(); }
"csc"|"cosec"   { report_token("CSC", yytext); update_column(); }
"cot"|"cotan"   { report_token("COT", yytext); update_column(); }
"asin"|"arcsin" { report_token("ASIN", yytext); update_column(); }
"acos"|"arccos" { report_token("ACOS", yytext); update_column(); }
"atan"|"arctan" { report_token("ATAN", yytext); update_column(); }
"sqrt"          { report_token("SQRT", yytext); update_column(); }
"abs"           { report_token("ABS", yytext); update_column(); }
"log"           { report_token("LOG", yytext); update_column(); }
"ln"            { report_token("LN", yytext); update_column(); }
"exp"           { report_token("EXP", yytext); update_column(); }

    /* Funções de arrays/listas */
"length"        { report_token("LENGTH", yytext); update_column(); }
"add"           { report_token("ADD", yytext); update_column(); }
"push"          { report_token("PUSH", yytext); update_column(); }
"pop"           { report_token("POP", yytext); update_column(); }
"remove"        { report_token("REMOVE", yytext); update_column(); }
"insert"        { report_token("INSERT", yytext); update_column(); }
"clear"         { report_token("CLEAR", yytext); update_column(); }
"isEmpty"       { report_token("IS_EMPTY", yytext); update_column(); }

    /* Funções de matrizes */
"transpose"     { report_token("TRANSPOSE", yytext); update_column(); }
"determinant"   { report_token("DETERMINANT", yytext); update_column(); }
"inverse"       { report_token("INVERSE", yytext); update_column(); }
"rows"          { report_token("ROWS", yytext); update_column(); }
"cols"          { report_token("COLS", yytext); update_column(); }

    /* Funções de grafos */
"nodeCount"     { report_token("NODE_COUNT", yytext); update_column(); }
"neighbors"     { report_token("NEIGHBORS", yytext); update_column(); }
"weight"        { report_token("WEIGHT", yytext); update_column(); }

    /* Funções de I/O */
"print"         { report_token("PRINT", yytext); update_column(); }
"println"       { report_token("PRINTLN", yytext); update_column(); }
"read"          { report_token("READ", yytext); update_column(); }
"range"         { report_token("RANGE", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 8: LITERAIS NUMÉRICOS
     * ======================================================================== */

{DIGIT}+"."{DIGIT}+([eE][+-]?{DIGIT}+)?  {
    /* Float com notação científica opcional */
    report_token("FLOAT_LITERAL", yytext);
    update_column();
}

{DIGIT}+[eE][+-]?{DIGIT}+  {
    /* Float em notação científica sem ponto */
    report_token("FLOAT_LITERAL", yytext);
    update_column();
}

{DIGIT}+  {
    /* Inteiro */
    report_token("INT_LITERAL", yytext);
    update_column();
}

({DIGIT}+|{DIGIT}+"."{DIGIT}+)[+-]{DIGIT}+("i"|"j")  {
    /* Número complexo: 3+4i ou 2.5-1.2i */
    report_token("COMPLEX_LITERAL", yytext);
    update_column();
}

({DIGIT}+|{DIGIT}+"."{DIGIT}+)("i"|"j")  {
    /* Número complexo imaginário puro: 4i */
    report_token("COMPLEX_LITERAL", yytext);
    update_column();
}

    /* ========================================================================
     * SEÇÃO 9: LITERAIS DE STRING E CHAR
     * ======================================================================== */

\"([^\"\\]|\\.)*\"  {
    /* String com escape sequences */
    report_token("STRING_LITERAL", yytext);
    update_column();
}

'([^'\\]|\\.)' {
    /* Char com escape sequences */
    report_token("CHAR_LITERAL", yytext);
    update_column();
}

    /* ========================================================================
     * SEÇÃO 10: IDENTIFICADORES
     * ======================================================================== */

{ID_START}{ID_CHAR}*  {
    report_token("IDENTIFIER", yytext);
    update_column();
}

    /* ========================================================================
     * SEÇÃO 11: OPERADORES
     * ======================================================================== */

    /* Operadores aritméticos */
"+"         { report_token("PLUS", yytext); update_column(); }
"-"         { report_token("MINUS", yytext); update_column(); }
"*"         { report_token("STAR", yytext); update_column(); }
"/"         { report_token("SLASH", yytext); update_column(); }
"//"        { report_token("FLOOR_DIV", yytext); update_column(); }
"%"         { report_token("MOD", yytext); update_column(); }
"^"         { report_token("POWER", yytext); update_column(); }
"@"         { report_token("MATMUL", yytext); update_column(); }

    /* Operadores de atribuição */
"="         { report_token("ASSIGN", yytext); update_column(); }
"+="        { report_token("PLUS_ASSIGN", yytext); update_column(); }
"-="        { report_token("MINUS_ASSIGN", yytext); update_column(); }
"*="        { report_token("STAR_ASSIGN", yytext); update_column(); }
"/="        { report_token("SLASH_ASSIGN", yytext); update_column(); }
"%="        { report_token("MOD_ASSIGN", yytext); update_column(); }

    /* Operadores de incremento/decremento */
"++"        { report_token("INCREMENT", yytext); update_column(); }
"--"        { report_token("DECREMENT", yytext); update_column(); }

    /* Operadores relacionais */
"=="        { report_token("EQUAL", yytext); update_column(); }
"!="        { report_token("NOT_EQUAL", yytext); update_column(); }
"<"         { report_token("LESS", yytext); update_column(); }
">"         { report_token("GREATER", yytext); update_column(); }
"<="        { report_token("LESS_EQUAL", yytext); update_column(); }
">="        { report_token("GREATER_EQUAL", yytext); update_column(); }

    /* Operadores lógicos (símbolos) */
"&&"        { report_token("LOGICAL_AND", yytext); update_column(); }
"||"        { report_token("LOGICAL_OR", yytext); update_column(); }
"!"         { report_token("LOGICAL_NOT", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 12: DELIMITADORES E PONTUAÇÃO
     * ======================================================================== */

"("         { report_token("LPAREN", yytext); update_column(); }
")"         { report_token("RPAREN", yytext); update_column(); }
"["         { report_token("LBRACKET", yytext); update_column(); }
"]"         { report_token("RBRACKET", yytext); update_column(); }
"{"         { report_token("LBRACE", yytext); update_column(); }
"}"         { report_token("RBRACE", yytext); update_column(); }
","         { report_token("COMMA", yytext); update_column(); }
";"         { report_token("SEMICOLON", yytext); update_column(); }
":"         { report_token("COLON", yytext); update_column(); }
"."         { report_token("DOT", yytext); update_column(); }

    /* ========================================================================
     * SEÇÃO 13: TRATAMENTO DE ERROS
     * ======================================================================== */

.           {
    fprintf(stderr, "ERRO LÉXICO: Caractere inválido '%s' na linha %d, coluna %d\n",
            yytext, line_number, column_number);
    update_column();
}

<<EOF>>     {
    /* Emite DEDENTs pendentes ao final do arquivo */
    while (indent_stack_ptr > 1) {
        indent_stack_ptr--;
        report_token("DEDENT", "");
    }
    return 0;
}

%%

/* ============================================================================
 * CÓDIGO C AUXILIAR
 * ============================================================================ */

int yywrap(void) {
    return 1; /* Indica fim de entrada */
}

int main(int argc, char** argv) {
    FILE* input_file = NULL;

    printf("============================================================================\n");
    printf("ANALISADOR LÉXICO - LINGUAGEM MATEMÁTICA\n");
    printf("============================================================================\n");
    printf("%-20s | %-30s | Posição\n", "TOKEN", "LEXEMA");
    printf("----------------------------------------------------------------------------\n");

    /* Arquivo de entrada */
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

    /* Executa o analisador léxico */
    yylex();

    printf("============================================================================\n");
    printf("Análise léxica concluída com sucesso!\n");
    printf("Total de linhas processadas: %d\n", line_number - 1);
    printf("============================================================================\n");

    if (input_file) {
        fclose(input_file);
    }

    return 0;
}