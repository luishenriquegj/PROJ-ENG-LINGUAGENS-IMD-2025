%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

#define MAX_INDENT 100
static int indent_stack[MAX_INDENT];
static int indent_top = 0;
static int pending_dedents = 0;
static int at_bol = 1;
static int inside_brackets = 0;

void push_indent(int level);
int pop_indent(void);
int current_indent(void);
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT}|_)*
INT         {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
SCIENTIFIC  ({INT}|{FLOAT})[eE][+-]?{DIGIT}+
COMPLEX     ({INT}|{FLOAT})[+-]({INT}|{FLOAT})[ij]
COMPLEX_I   ({INT}|{FLOAT})[ij]

%%



^[ \t]+ {
    if (at_bol && inside_brackets == 0) {
        int spaces = 0;
        for (int i = 0; yytext[i]; i++) {
            spaces += (yytext[i] == '\t') ? 4 : 1;
        }
        int level = spaces / 4;

        if (level > current_indent()) {
            push_indent(level);
            at_bol = 0;
            return INDENT;
        } else if (level < current_indent()) {
            while (level < current_indent() && indent_top > 0) {
                pop_indent();
                pending_dedents++;
            }
            if (pending_dedents > 0) {
                pending_dedents--;
                yyless(0);
                at_bol = 1;
                return DEDENT;
            }
        }
        at_bol = 0;
    }
}

^[^ \t\n] {
    if (at_bol && inside_brackets == 0) {
        int level = 0;
        if (level < current_indent()) {
            while (level < current_indent() && indent_top > 0) {
                pop_indent();
                pending_dedents++;
            }
            if (pending_dedents > 0) {
                pending_dedents--;
                yyless(0);
                at_bol = 1;
                return DEDENT;
            }
        }
    }
    yyless(0);
    at_bol = 0;
}

\n {
    at_bol = 1;
    if (inside_brackets > 0) {
    }
    return NEWLINE;
}


"#".*           { }

"def"       { return DEF; }
"end"       {
    if (at_bol && inside_brackets == 0) {
        if (current_indent() > 0) {
            while (current_indent() > 0) {
                pop_indent();
                pending_dedents++;
            }
        }
        if (pending_dedents > 0) {
            pending_dedents--;
            yyless(0);
            at_bol = 1;
            return DEDENT;
        }
    }
    return END;
}
"fun"       { return FUN; }
"if"        { return IF; }
"elif"      { return ELIF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"for"       { return FOR; }
"in"        { return IN; }
"return"    { return RETURN; }
"break"     { return BREAK; }
"continue"  { return CONTINUE; }
"try"       { return TRY; }
"catch"     { return CATCH; }
"finally"   { return FINALLY; }
"import"    { return IMPORT; }
"const"     { return CONST; }
"this"      { return THIS; }
"new"       { return NEW; }


"int"       { return INT_TYPE; }
"float"     { return FLOAT_TYPE; }
"bool"      { return BOOL_TYPE; }
"complex"   { return COMPLEX_TYPE; }
"char"      { return CHAR_TYPE; }
"string"    { return STRING_TYPE; }
"void"      { return VOID_TYPE; }
"matrix"    { return MATRIX_TYPE; }
"set"       { return SET_TYPE; }
"graph"     { return GRAPH_TYPE; }


"true"      { yylval.bval = 1; return TRUE; }
"false"     { yylval.bval = 0; return FALSE; }


"and"       { return AND; }
"or"        { return OR; }
"not"       { return NOT; }


"print"     { return PRINT; }
"range"     { return RANGE; }
"sin"|"sen" { yylval.sval = strdup(yytext); return IDENTIFIER; }
"cos"       { yylval.sval = strdup(yytext); return IDENTIFIER; }
"tan"       { yylval.sval = strdup(yytext); return IDENTIFIER; }
"sqrt"      { yylval.sval = strdup(yytext); return IDENTIFIER; }
"abs"       { yylval.sval = strdup(yytext); return IDENTIFIER; }
"log"       { yylval.sval = strdup(yytext); return IDENTIFIER; }
"ln"        { yylval.sval = strdup(yytext); return IDENTIFIER; }
"exp"       { yylval.sval = strdup(yytext); return IDENTIFIER; }


{COMPLEX}   { yylval.sval = strdup(yytext); return COMPLEX_LITERAL; }
{COMPLEX_I} { yylval.sval = strdup(yytext); return COMPLEX_LITERAL; }
{SCIENTIFIC} { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
{FLOAT}     { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
{INT}       { yylval.ival = atoi(yytext); return INT_LITERAL; }


\"([^\"\\]|\\.)*\" {
    yylval.sval = strndup(yytext + 1, yyleng - 2);
    return STRING_LITERAL;
}

\'([^\'\\]|\\.)?\' {
    if (yyleng == 3) {
        yylval.cval = yytext[1];
    } else if (yyleng == 4 && yytext[1] == '\\') {
        switch (yytext[2]) {
            case 'n': yylval.cval = '\n'; break;
            case 't': yylval.cval = '\t'; break;
            case 'r': yylval.cval = '\r'; break;
            case '\\': yylval.cval = '\\'; break;
            case '\'': yylval.cval = '\''; break;
            case '0': yylval.cval = '\0'; break;
            default: yylval.cval = yytext[2];
        }
    } else {
        yylval.cval = '\0';
    }
    return CHAR_LITERAL;
}


{ID}        { yylval.sval = strdup(yytext); return IDENTIFIER; }


"++"        { return INCREMENT; }
"--"        { return DECREMENT; }
"+="        { return PLUS_ASSIGN; }
"-="        { return MINUS_ASSIGN; }
"*="        { return STAR_ASSIGN; }
"/="        { return SLASH_ASSIGN; }
"%="        { return MOD_ASSIGN; }
"=="        { return EQUAL; }
"!="        { return NOT_EQUAL; }
"<="        { return LESS_EQUAL; }
">="        { return GREATER_EQUAL; }
"&&"        { return AND; }
"||"        { return OR; }
"//"        { return FLOOR_DIV; }

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return STAR; }
"/"         { return SLASH; }
"%"         { return MOD; }
"^"         { return POWER; }
"@"         { return MATMUL; }
"<"         { return LESS; }
">"         { return GREATER; }
"="         { return ASSIGN; }
"!"         { return NOT; }


"("         { inside_brackets++; return LPAREN; }
")"         { inside_brackets--; return RPAREN; }
"["         { inside_brackets++; return LBRACKET; }
"]"         { inside_brackets--; return RBRACKET; }
"{"         { inside_brackets++; return LBRACE; }
"}"         { inside_brackets--; return RBRACE; }
","         { return COMMA; }
":"         { return COLON; }
"."         { return DOT; }



[ \t]+      {  }


. {
    fprintf(stderr, "Erro léxico: caractere inválido '%c' (ASCII %d) na linha %d\n",
            yytext[0], yytext[0], yylineno);
}


<<EOF>> {
    if (!at_bol) {
        at_bol = 1;
        yyless(0);
        return NEWLINE;
    }
    while (indent_top > 0) {
        pop_indent();
        return DEDENT;
    }
    return 0;
}

%%

void push_indent(int level) {
    if (indent_top < MAX_INDENT - 1) {
        indent_stack[++indent_top] = level;
    }
}

int pop_indent(void) {
    if (indent_top > 0) {
        return indent_stack[indent_top--];
    }
    return 0;
}

int current_indent(void) {
    return (indent_top > 0) ? indent_stack[indent_top] : 0;
}