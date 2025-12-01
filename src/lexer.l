%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

#define MAX_INDENT 100
#define LINEBUF_MAX 8192
#define TOKBUF_MAX 512

static int indent_stack[MAX_INDENT];
static int indent_top = 0;
static int pending_dedents = 0;
static int at_bol = 1;
static int inside_brackets = 0;

static int curr_col = 0;
static int tok_line = 1;
static int tok_col = 1;
static int tok_len = 0;

static char linebuf[LINEBUF_MAX];
static int  linebuf_len = 0;
static char tokbuf[TOKBUF_MAX];

void push_indent(int level);
int pop_indent(void);
int current_indent(void);

static void linebuf_clear(void) { linebuf[0] = '\0'; linebuf_len = 0; }
static void linebuf_append_escaped(const char* s, int n) {
    if (n < 0) n = (int)strlen(s);
    for (int i = 0; i < n; i++) {
        char c = s[i];
        const char* esc = NULL;
        switch (c) {
            case '\n': esc = "\\n"; break;
            case '\t': esc = "\\t"; break;
            case '\r': esc = "\\r"; break;
            default: esc = NULL; break;
        }
        if (esc) {
            int el = (int)strlen(esc);
            if (linebuf_len + el < LINEBUF_MAX-1) {
                memcpy(linebuf + linebuf_len, esc, el);
                linebuf_len += el;
            }
        } else {
            if (linebuf_len + 1 < LINEBUF_MAX-1) {
                linebuf[linebuf_len++] = c;
            }
        }
    }
    linebuf[linebuf_len] = '\0';
}
static void tokbuf_set_escaped(const char* s, int n) {
    int p = 0;
    if (n < 0) n = (int)strlen(s);
    for (int i = 0; i < n && p < TOKBUF_MAX-2; i++) {
        char c = s[i];
        if (c == '\n') { if (p < TOKBUF_MAX-2) { tokbuf[p++]='\\'; tokbuf[p++]='n'; } }
        else if (c == '\t') { if (p < TOKBUF_MAX-2) { tokbuf[p++]='\\'; tokbuf[p++]='t'; } }
        else if (c == '\r') { if (p < TOKBUF_MAX-2) { tokbuf[p++]='\\'; tokbuf[p++]='r'; } }
        else tokbuf[p++] = c;
    }
    tokbuf[p] = '\0';
}

const char* lex_current_line(void) { return linebuf; }
int lex_tok_line(void) { return tok_line; }
int lex_tok_col(void) { return tok_col; }
int lex_tok_len(void) { return tok_len; }
const char* lex_tok_text(void) { return tokbuf; }

#define RETTOK(TK) do { \
    tok_line = yylineno; tok_col = curr_col + 1; tok_len = yyleng; \
    tokbuf_set_escaped(yytext, yyleng); \
    curr_col += yyleng; linebuf_append_escaped(yytext, yyleng); \
    return (TK); \
} while(0)
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT         {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
SCIENTIFIC  ({INT}|{FLOAT})[eE][+-]?{DIGIT}+
COMPLEX     ({INT}|{FLOAT})[+-]({INT}|{FLOAT})[ij]
COMPLEX_I   ({INT}|{FLOAT})[ij]

%%

^[ \t]+ {
    if (at_bol && inside_brackets == 0) {
        // Drena DEDENTs pendentes antes de processar o recuo da linha
        if (pending_dedents > 0) {
            pending_dedents--;
            tok_line = yylineno; tok_col = curr_col + 1; tok_len = 0;
            tokbuf_set_escaped("", 0);
            yyless(0);
            at_bol = 1;
            return DEDENT;
        }
        int spaces = 0;
        for (int i = 0; yytext[i]; i++) spaces += (yytext[i] == '\t') ? 4 : 1;
        int level = spaces / 4;

        if (level > current_indent()) {
            push_indent(level);
            at_bol = 0;
            RETTOK(INDENT);
        } else if (level < current_indent()) {
            while (level < current_indent() && indent_top > 0) {
                pop_indent();
                pending_dedents++;
            }
            if (pending_dedents > 0) {
                pending_dedents--;
                tok_line = yylineno; tok_col = curr_col + 1; tok_len = 0;
                tokbuf_set_escaped("", 0);
                yyless(0);
                at_bol = 1;
                return DEDENT;
            }
        }
        at_bol = 0;
        tok_line = yylineno; tok_col = curr_col + 1; tok_len = yyleng;
        tokbuf_set_escaped(yytext, yyleng);
        curr_col += yyleng;
        linebuf_append_escaped(yytext, yyleng);
        /* não retorna token */
    }
}

^[^ \t\n] {
    if (at_bol && inside_brackets == 0) {
        // Drena DEDENTs pendentes antes de consumir tokens não-espaço
        if (pending_dedents > 0) {
            pending_dedents--;
            tok_line = yylineno; tok_col = curr_col + 1; tok_len = 0;
            tokbuf_set_escaped("", 0);
            yyless(0);
            at_bol = 1;
            return DEDENT;
        }
        int level = 0;
        if (level < current_indent()) {
            while (level < current_indent() && indent_top > 0) {
                pop_indent();
                pending_dedents++;
            }
            if (pending_dedents > 0) {
                pending_dedents--;
                tok_line = yylineno; tok_col = curr_col + 1; tok_len = 0;
                tokbuf_set_escaped("", 0);
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
    if (inside_brackets == 0) {
        at_bol = 1;
        tok_line = yylineno; tok_col = curr_col + 1; tok_len = 2;
        tokbuf_set_escaped("\\n", 2);
        curr_col = 0;
        linebuf_clear();
        return NEWLINE;
    }
}

"#".* {
    tok_line = yylineno; tok_col = curr_col + 1; tok_len = yyleng;
    tokbuf_set_escaped(yytext, yyleng);
    curr_col += yyleng;
    linebuf_append_escaped(yytext, yyleng);
    /* ignora comentário */
}

"def"       { at_bol = 0; RETTOK(DEF); }

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
            tok_line = yylineno; tok_col = curr_col + 1; tok_len = 0;
            tokbuf_set_escaped("", 0);
            yyless(0);
            at_bol = 1;
            return DEDENT;
        }
    }
    at_bol = 0; RETTOK(END);
}

"fun"       { at_bol = 0; RETTOK(FUN); }
"class"     { at_bol = 0; RETTOK(CLASS); }
"import"    { at_bol = 0; RETTOK(IMPORT); }
"const"     { at_bol = 0; RETTOK(CONST); }

"if"        { at_bol = 0; RETTOK(IF); }
"elif"      { at_bol = 0; RETTOK(ELIF); }
"else"      { at_bol = 0; RETTOK(ELSE); }
"while"     { at_bol = 0; RETTOK(WHILE); }
"for"       { at_bol = 0; RETTOK(FOR); }
"in"        { at_bol = 0; RETTOK(IN); }
"return"    { at_bol = 0; RETTOK(RETURN); }
"break"     { at_bol = 0; RETTOK(BREAK); }
"continue"  { at_bol = 0; RETTOK(CONTINUE); }
"try"       { at_bol = 0; RETTOK(TRY); }
"catch"     { at_bol = 0; RETTOK(CATCH); }
"finally"   { at_bol = 0; RETTOK(FINALLY); }

"int"       { at_bol = 0; RETTOK(INT_TYPE); }
"float"     { at_bol = 0; RETTOK(FLOAT_TYPE); }
"bool"      { at_bol = 0; RETTOK(BOOL_TYPE); }
"complex"   { at_bol = 0; RETTOK(COMPLEX_TYPE); }
"char"      { at_bol = 0; RETTOK(CHAR_TYPE); }
"string"    { at_bol = 0; RETTOK(STRING_TYPE); }
"void"      { at_bol = 0; RETTOK(VOID_TYPE); }
"undefined" { at_bol = 0; RETTOK(UNDEFINED_TYPE); }
"matrix"    { at_bol = 0; RETTOK(MATRIX_TYPE); }
"set"       { at_bol = 0; RETTOK(SET_TYPE); }
"graph"     { at_bol = 0; RETTOK(GRAPH_TYPE); }

"true"      { at_bol = 0; yylval.bval = 1; RETTOK(TRUE); }
"false"     { at_bol = 0; yylval.bval = 0; RETTOK(FALSE); }

"and"       { at_bol = 0; RETTOK(AND); }
"or"        { at_bol = 0; RETTOK(OR); }
"not"       { at_bol = 0; RETTOK(NOT); }

"this"      { at_bol = 0; RETTOK(THIS); }
"new"       { at_bol = 0; RETTOK(NEW); }
"print"     { at_bol = 0; RETTOK(PRINT); }
"range"     { at_bol = 0; RETTOK(RANGE); }

"=="        { at_bol = 0; RETTOK(EQUAL); }
"!="        { at_bol = 0; RETTOK(NOT_EQUAL); }
"<="        { at_bol = 0; RETTOK(LESS_EQUAL); }
">="        { at_bol = 0; RETTOK(GREATER_EQUAL); }
"&&"        { at_bol = 0; RETTOK(AND); }
"||"        { at_bol = 0; RETTOK(OR); }
"//"        { at_bol = 0; RETTOK(FLOOR_DIV); }
"+="        { at_bol = 0; RETTOK(PLUS_ASSIGN); }
"-="        { at_bol = 0; RETTOK(MINUS_ASSIGN); }
"*="        { at_bol = 0; RETTOK(STAR_ASSIGN); }
"/="        { at_bol = 0; RETTOK(SLASH_ASSIGN); }
"%="        { at_bol = 0; RETTOK(MOD_ASSIGN); }
"++"        { at_bol = 0; RETTOK(INCREMENT); }
"--"        { at_bol = 0; RETTOK(DECREMENT); }
"+"         { at_bol = 0; RETTOK(PLUS); }
"-"         { at_bol = 0; RETTOK(MINUS); }
"*"         { at_bol = 0; RETTOK(STAR); }
"/"         { at_bol = 0; RETTOK(SLASH); }
"%"         { at_bol = 0; RETTOK(MOD); }
"^"         { at_bol = 0; RETTOK(POWER); }
"@"         { at_bol = 0; RETTOK(MATMUL); }
"<"         { at_bol = 0; RETTOK(LESS); }
">"         { at_bol = 0; RETTOK(GREATER); }
"="         { at_bol = 0; RETTOK(ASSIGN); }
"!"         { at_bol = 0; RETTOK(NOT); }

"("         { at_bol = 0; inside_brackets++; RETTOK(LPAREN); }
")"         { at_bol = 0; if (inside_brackets>0) inside_brackets--; RETTOK(RPAREN); }
"["         { at_bol = 0; inside_brackets++; RETTOK(LBRACKET); }
"]"         { at_bol = 0; if (inside_brackets>0) inside_brackets--; RETTOK(RBRACKET); }
"{"         { at_bol = 0; inside_brackets++; RETTOK(LBRACE); }
"}"         { at_bol = 0; if (inside_brackets>0) inside_brackets--; RETTOK(RBRACE); }
","         { at_bol = 0; RETTOK(COMMA); }
":"         { at_bol = 0; RETTOK(COLON); }
"."         { at_bol = 0; RETTOK(DOT); }

{COMPLEX}    { at_bol = 0; yylval.sval = strdup(yytext); RETTOK(COMPLEX_LITERAL); }
{COMPLEX_I}  { at_bol = 0; yylval.sval = strdup(yytext); RETTOK(COMPLEX_LITERAL); }
{SCIENTIFIC} { at_bol = 0; yylval.fval = atof(yytext); RETTOK(FLOAT_LITERAL); }
{FLOAT}      { at_bol = 0; yylval.fval = atof(yytext); RETTOK(FLOAT_LITERAL); }
{INT}        { at_bol = 0; yylval.ival = atoi(yytext); RETTOK(INT_LITERAL); }

\"([^\"\\]|\\.)*\" {
    at_bol = 0; yylval.sval = strndup(yytext+1, yyleng-2); RETTOK(STRING_LITERAL);
}

\'([^\'\\]|\\.)?\' {
    at_bol = 0;
    if (yyleng == 3) yylval.cval = yytext[1];
    else if (yyleng == 4 && yytext[1] == '\\') {
        switch (yytext[2]) {
            case 'n': yylval.cval = '\n'; break;
            case 't': yylval.cval = '\t'; break;
            case 'r': yylval.cval = '\r'; break;
            case '\\': yylval.cval = '\\'; break;
            case '\'': yylval.cval = '\''; break;
            case '0': yylval.cval = '\0'; break;
            default: yylval.cval = yytext[2]; break;
        }
    } else yylval.cval = '\0';
    RETTOK(CHAR_LITERAL);
}

{ID}        { at_bol = 0; yylval.sval = strdup(yytext); RETTOK(IDENTIFIER); }

[ \t]+ {
    tok_line = yylineno; tok_col = curr_col + 1; tok_len = yyleng;
    tokbuf_set_escaped(yytext, yyleng);
    curr_col += yyleng;
    linebuf_append_escaped(yytext, yyleng);
    /* ignora espaços fora do BOL */
}

[ \r]+ { }

<<EOF>> {
    while (pending_dedents > 0) { pending_dedents--; return DEDENT; }
    while (indent_top > 0) { pop_indent(); return DEDENT; }
    return 0;
}

. {
    char esc[8];
    if (yytext[0] == '\n') strcpy(esc, "\\n");
    else if (yytext[0] == '\t') strcpy(esc, "\\t");
    else if (yytext[0] == '\r') strcpy(esc, "\\r");
    else { esc[0] = yytext[0]; esc[1] = '\0'; }
    fprintf(stderr, "Erro léxico na linha %d, col %d: caractere inválido '%s'\n",
            yylineno, curr_col+1, esc);
}
%%

void push_indent(int level) {
    if (indent_top < MAX_INDENT - 1) indent_stack[++indent_top] = level;
}
int pop_indent(void) {
    if (indent_top > 0) return indent_stack[indent_top--];
    return 0;
}
int current_indent(void) {
    return (indent_top > 0) ? indent_stack[indent_top] : 0;
}