# Documentação do Analisador Léxico

Este documento detalha a implementação do analisador léxico (lexer) do compilador.

> **Documentação relacionada:**
> - [README](README.md) - Visão geral do projeto
> - [Guia de Execução](GUIA_EXECUCAO.md) - Como usar o compilador
> - [Documentação do Sintático](DOCUMENTACAO_SINTATICO.md) - Análise sintática

## O Que É o Analisador Léxico

O analisador léxico é a primeira fase do compilador. Ele lê o código-fonte caractere por caractere e o transforma em uma sequência de **tokens** (unidades léxicas). Por exemplo, a linha:

```python
int x = 42
```

É transformada em:
```
INT_TYPE, IDENTIFIER('x'), ASSIGN, INT_LITERAL(42)
```

## Implementação

O analisador léxico foi implementado usando **Flex** (Fast Lexical Analyzer Generator), uma ferramenta que gera código C a partir de uma especificação de expressões regulares.

### Arquivo: `src/lexer.l`

O arquivo contém três seções principais:

```c
%{
/* Seção 1: Código C e definições */
#include <stdio.h>
#include "parser.tab.h"
...
%}

/* Seção 2: Expressões regulares e regras */
DIGIT    [0-9]
LETTER   [a-zA-Z]
...

%%
"int"    { return INT_TYPE; }
{DIGIT}+ { yylval.ival = atoi(yytext); return INT_LITERAL; }
...

%%
/* Seção 3: Código C adicional */
```

## Tokens Reconhecidos

### Palavras-Chave (43 tokens)

```
def, end, fun, if, elif, else, while, for, in, return,
break, continue, try, catch, finally, import, const,
this, new, and, or, not, print, range
```

Exemplo de implementação:
```c
"if"       { return IF; }
"while"    { return WHILE; }
"return"   { return RETURN; }
```

### Tipos de Dados (10 tokens)

```
int, float, bool, complex, char, string, void,
matrix, set, graph
```

Exemplo:
```c
"int"      { return INT_TYPE; }
"float"    { return FLOAT_TYPE; }
"complex"  { return COMPLEX_TYPE; }
```

### Operadores (35 tokens)

**Aritméticos:**
```c
"+"        { return PLUS; }
"-"        { return MINUS; }
"*"        { return STAR; }
"/"        { return SLASH; }
"//"       { return FLOOR_DIV; }    // Divisão inteira
"%"        { return MOD; }
"^"        { return POWER; }         // Potenciação
"@"        { return MATMUL; }        // Produto matricial
```

**Atribuição:**
```c
"="        { return ASSIGN; }
"+="       { return PLUS_ASSIGN; }
"-="       { return MINUS_ASSIGN; }
"*="       { return STAR_ASSIGN; }
"/="       { return SLASH_ASSIGN; }
"%="       { return MOD_ASSIGN; }
```

**Incremento/Decremento:**
```c
"++"       { return INCREMENT; }
"--"       { return DECREMENT; }
```

**Relacionais:**
```c
"=="       { return EQUAL; }
"!="       { return NOT_EQUAL; }
"<"        { return LESS; }
">"        { return GREATER; }
"<="       { return LESS_EQUAL; }
">="       { return GREATER_EQUAL; }
```

**Lógicos:**
```c
"and"      { return AND; }
"or"       { return OR; }
"not"      { return NOT; }
"&&"       { return AND; }     // Alternativa
"||"       { return OR; }      // Alternativa
"!"        { return NOT; }     // Alternativa
```

### Literais

**Números Inteiros:**
```c
{DIGIT}+   { yylval.ival = atoi(yytext); return INT_LITERAL; }
```

Exemplos: `42`, `1000`, `0`

**Números de Ponto Flutuante:**
```c
{DIGIT}+\.{DIGIT}+  { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
```

Exemplos: `3.14`, `2.5`, `0.001`

**Notação Científica:**
```c
({INT}|{FLOAT})[eE][+-]?{DIGIT}+  { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
```

Exemplos: `1.5e10`, `2.5e-3`, `1E5`

**Números Complexos:**
```c
{NUMBER}[+-]{NUMBER}[ij]  { yylval.sval = strdup(yytext); return COMPLEX_LITERAL; }
{NUMBER}[ij]              { yylval.sval = strdup(yytext); return COMPLEX_LITERAL; }
```

Exemplos: `3+4i`, `2.5-1.5i`, `5i`

**Strings:**
```c
\"([^\\\"]|\\.)*\"  { yylval.sval = strdup(yytext); return STRING_LITERAL; }
```

Suporta escape sequences: `\n`, `\t`, `\"`, `\\`

Exemplos: `"hello"`, `"linha 1\nlinha 2"`, `"aspas: \"texto\""`

**Caracteres:**
```c
'([^'\\]|\\.)'  { yylval.cval = yytext[1]; return CHAR_LITERAL; }
```

Exemplos: `'a'`, `'\n'`, `'\t'`

**Booleanos:**
```c
"true"     { yylval.bval = 1; return TRUE; }
"false"    { yylval.bval = 0; return FALSE; }
```

### Identificadores

```c
{LETTER}({LETTER}|{DIGIT}|_)*  { yylval.sval = strdup(yytext); return IDENTIFIER; }
```

Regras:
- Começa com letra
- Pode conter letras, dígitos e underscore
- Não pode ser palavra-chave

Exemplos: `x`, `variavel`, `nome_completo`, `valor2`

### Delimitadores

```c
"("        { return LPAREN; }
")"        { return RPAREN; }
"["        { return LBRACKET; }
"]"        { return RBRACKET; }
"{"        { return LBRACE; }
"}"        { return RBRACE; }
","        { return COMMA; }
":"        { return COLON; }
"."        { return DOT; }
";"        { return SEMICOLON; }
```

## Gerenciamento de Indentação

Uma característica especial da linguagem é o uso de **indentação significativa** (como Python). O lexer gerencia isso automaticamente.

### Pilha de Indentação

```c
#define MAX_INDENT 100
static int indent_stack[MAX_INDENT];
static int indent_top = 0;
static int pending_dedents = 0;
static int at_bol = 1;
```

### Funcionamento

Quando o lexer encontra o início de uma linha:

1. **Conta os espaços** de indentação
2. **Compara com o topo da pilha**:
   - Se maior: empilha novo nível e emite `INDENT`
   - Se menor: desempilha até igualar e emite `DEDENT`s
   - Se igual: continua normalmente

### Exemplo

Código:
```python
def fun void main()
    int x = 10
    if (x > 0)
        print(x)
end def
```

Tokens gerados:
```
DEF FUN VOID_TYPE IDENTIFIER LPAREN RPAREN NEWLINE
INDENT                           # Entra na função
INT_TYPE IDENTIFIER ASSIGN INT_LITERAL NEWLINE
IF LPAREN IDENTIFIER GREATER INT_LITERAL RPAREN NEWLINE
INDENT                           # Entra no if
PRINT LPAREN IDENTIFIER RPAREN NEWLINE
DEDENT                          # Sai do if
DEDENT                          # Sai da função
END DEF
```

### Implementação da Detecção

```c
^[ \t]+ {
    if (at_bol && inside_brackets == 0) {
        int spaces = 0;
        for (int i = 0; i < yyleng; i++) {
            if (yytext[i] == ' ') spaces++;
            else if (yytext[i] == '\t') spaces += 4;
        }
        
        int current = current_indent();
        
        if (spaces > current) {
            push_indent(spaces);
            at_bol = 0;
            return INDENT;
        } else if (spaces < current) {
            while (current_indent() > spaces) {
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
```

### Tratamento Especial: `end`

Antes da palavra-chave `end`, todos os `DEDENT`s pendentes são emitidos automaticamente:

```c
"end" {
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
```

Isso garante que blocos aninhados sejam fechados corretamente.

## Comentários

A linguagem suporta comentários de linha:

```c
"#".*  { /* Ignora até o fim da linha */ }
```

Exemplo:
```python
int x = 10  # Declaração de variável
# Este é um comentário
```

## Tratamento de Erros

Caracteres inválidos são detectados e reportados:

```c
.  {
    fprintf(stderr, "Erro léxico: caractere inválido '%c' (ASCII %d) na linha %d\n",
            yytext[0], yytext[0], yylineno);
}
```

Exemplo de erro:
```
Erro léxico: caractere inválido '@' (ASCII 64) na linha 5
```

## Espaços em Branco e Quebras de Linha

```c
\n  {
    yylineno++;
    at_bol = 1;
    if (inside_brackets == 0) {
        return NEWLINE;
    }
}

[ \t]+  {
    if (!at_bol || inside_brackets > 0) {
        /* Ignora espaços que não estão no início da linha */
    }
}
```

Newlines são significativos exceto dentro de parênteses, colchetes ou chaves.

## Teste do Analisador Léxico

Para testar o lexer isoladamente, use o programa `test_tokens`:

```bash
make tokens
./test_tokens arquivo.mf
```

Exemplo de saída:
```
=== ANÁLISE LÉXICA (TOKENS) ===
Linha Token                Lexema                        
---------------------------------------------------------------
1     IMPORT               'import'
1     IDENTIFIER           'io'
2     NEWLINE              '\n'
3     DEF                  'def'
3     FUN                  'fun'
3     VOID_TYPE            'void'
3     IDENTIFIER           'main'
3     LPAREN               '('
3     RPAREN               ')'
4     NEWLINE              '\n'
4     INDENT               '    '
4     PRINT                'print'
4     LPAREN               '('
4     STRING_LITERAL       '"Hello, World!"'
4     RPAREN               ')'
5     NEWLINE              '\n'
5     DEDENT               ''
5     END                  'end'
5     DEF                  'def'
```

## Expressões Regulares Principais

### Definições

```c
DIGIT    [0-9]
LETTER   [a-zA-Z]
ID       {LETTER}({LETTER}|{DIGIT}|_)*

INT      {DIGIT}+
FLOAT    {DIGIT}+\.{DIGIT}+
EXP      [eE][+-]?{DIGIT}+
SCIENTIFIC ({INT}|{FLOAT}){EXP}

COMPLEX_FULL  ({INT}|{FLOAT})[+-]({INT}|{FLOAT})[ij]
COMPLEX_PURE  ({INT}|{FLOAT})[ij]
```

### Prioridade de Regras

O Flex aplica as regras na ordem:
1. **Regra mais longa** (longest match)
2. **Primeira regra** definida (se empate)

Por isso, palavras-chave são definidas antes de identificadores:

```c
"if"       { return IF; }           // Mais específico
"while"    { return WHILE; }        // Mais específico
{ID}       { return IDENTIFIER; }   // Mais genérico
```

## Variáveis Globais do Lexer

```c
extern int yylineno;        // Número da linha atual
extern char* yytext;        // Texto do token atual
extern int yyleng;          // Comprimento do token
extern YYSTYPE yylval;      // Valor semântico do token
```

## Integração com o Parser

O lexer é chamado pela função `yylex()` gerada pelo Flex. O parser (Bison) chama `yylex()` repetidamente para obter tokens:

```c
int token = yylex();  // Obtém próximo token
```

O valor semântico do token é passado através de `yylval`, que é uma união definida no parser:

```c
%union {
    int ival;      // Para números inteiros
    double fval;   // Para floats
    char* sval;    // Para strings e identificadores
    char cval;     // Para caracteres
    int bval;      // Para booleanos
}
```

## Exemplo Completo de Tokenização

Entrada:
```python
import io

def fun void main()
    int x = 42
    print(x)
end def
```

Tokens gerados:
```
IMPORT IDENTIFIER('io') NEWLINE
NEWLINE
DEF FUN VOID_TYPE IDENTIFIER('main') LPAREN RPAREN NEWLINE
INDENT
INT_TYPE IDENTIFIER('x') ASSIGN INT_LITERAL(42) NEWLINE
PRINT LPAREN IDENTIFIER('x') RPAREN NEWLINE
DEDENT
END DEF
```

## Estatísticas

- **Total de tokens únicos**: ~90
- **Palavras-chave**: 43
- **Operadores**: 35
- **Tipos**: 10
- **Delimitadores**: 10

## Arquivos Gerados

Ao compilar com `make all`, o Flex gera:

- `build/lex.yy.c` - Código C do analisador léxico
- `build/lex.yy.o` - Objeto compilado

## Referências

- Flex Manual: https://westes.github.io/flex/manual/
- Levine, J. "flex & bison" (2009)

---