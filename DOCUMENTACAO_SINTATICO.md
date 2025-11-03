# Documentação do Analisador Sintático

Este documento detalha a implementação do analisador sintático (parser) do compilador.

> **Documentação relacionada:**
> - [README](README.md) - Visão geral do projeto
> - [Guia de Execução](GUIA_EXECUCAO.md) - Como usar o compilador
> - [Documentação do Léxico](DOCUMENTACAO_LEXICO.md) - Análise léxica

## O Que É o Analisador Sintático

O analisador sintático (parser) é a segunda fase do compilador. Ele recebe a sequência de tokens do [analisador léxico](DOCUMENTACAO_LEXICO.md) e verifica se ela obedece às regras gramaticais da linguagem, construindo uma **Árvore Sintática Abstrata (AST)**.

Por exemplo, os tokens:
```
INT_TYPE IDENTIFIER('x') ASSIGN INT_LITERAL(42)
```

São reconhecidos como uma **declaração de variável** e transformados em um nó da AST.

## Implementação

O parser foi implementado usando **Bison** (GNU Parser Generator), uma ferramenta que gera um parser LALR(1) bottom-up a partir de uma gramática livre de contexto.

### Arquivo: `src/parser.y`

O arquivo contém:

```c
%{
/* Seção 1: Código C e includes */
#include <stdio.h>
#include "ast.h"
...
%}

/* Seção 2: Definições de tokens e tipos */
%token INT_TYPE FLOAT_TYPE
%type <node> expression statement
...

%%
/* Seção 3: Regras da gramática */
program: opt_imports definition_list main_function
       | opt_imports main_function
       ;

statement: declaration
         | assignment
         | if_stmt
         ...
         ;

%%
/* Seção 4: Código C adicional */
```

## Tipo de Parser

O parser é **LALR(1)** (Look-Ahead LR com 1 token de lookahead), que significa:

- **Bottom-up**: Constrói a árvore das folhas para a raiz
- **Shift-reduce**: Usa operações de empilhar tokens (shift) e reduzir regras (reduce)
- **1 token de lookahead**: Olha apenas 1 token à frente para decidir a ação

## Gramática da Linguagem

### Estrutura do Programa

```yacc
program: opt_imports definition_list main_function opt_newlines
       | opt_imports main_function opt_newlines
       ;
```

Um programa pode ter:
- Imports (opcional)
- Definições de funções/tipos (opcional)
- Função main (obrigatória)

### Imports

```yacc
imports: import_list opt_newlines
       ;

import: IMPORT IDENTIFIER NEWLINE
      ;
```

Exemplo:
```python
import io
import math
```

### Função Main

```yacc
main_function: DEF FUN VOID_TYPE IDENTIFIER LPAREN RPAREN NEWLINE 
               INDENT statement_list opt_newlines DEDENT END DEF opt_newlines
             ;
```

A função main sempre retorna `void` e não tem parâmetros:
```python
def fun void main()
    # corpo da função
end def
```

### Definição de Funções

```yacc
function_def: DEF FUN type IDENTIFIER LPAREN params RPAREN NEWLINE
              INDENT statement_list opt_newlines DEDENT END DEF opt_newlines
            ;
```

Funções regulares podem ter qualquer tipo de retorno exceto `void`:
```python
def fun int soma(int a, int b)
    return a + b
end def
```

### Parâmetros

```yacc
params: param_list
      | /* vazio */
      ;

param_list: type IDENTIFIER
          | param_list COMMA type IDENTIFIER
          ;
```

Exemplo:
```python
def fun float calculo(int x, float y, bool flag)
    # ...
end def
```

### Tipos

```yacc
type: primitive_type 
    | array_type 
    | matrix_type 
    | set_type 
    | graph_type 
    | IDENTIFIER
    ;

primitive_type: INT_TYPE | FLOAT_TYPE | BOOL_TYPE 
              | COMPLEX_TYPE | CHAR_TYPE | STRING_TYPE
              ;

array_type: primitive_type LBRACKET RBRACKET
          ;

matrix_type: MATRIX_TYPE LBRACKET primitive_type RBRACKET
           ;
```

Note que `VOID_TYPE` **não** está em `primitive_type`, apenas em `main_function`. Isso elimina ambiguidades na gramática.

### Statements (Comandos)

```yacc
statement: declaration NEWLINE
         | assignment NEWLINE
         | if_stmt
         | while_stmt
         | for_stmt
         | return_stmt NEWLINE
         | break_stmt NEWLINE
         | continue_stmt NEWLINE
         | expr_stmt NEWLINE
         | try_stmt
         | NEWLINE
         ;
```

### Declarações

```yacc
declaration: type IDENTIFIER ASSIGN expression
           | type IDENTIFIER
           | CONST IDENTIFIER ASSIGN expression
           ;
```

Exemplos:
```python
int x = 42              # Com inicialização
float y                 # Sem inicialização
const PI = 3.14159     # Constante
```

### Atribuições

```yacc
assignment: postfix_expr assign_op expression
          ;

assign_op: ASSIGN | PLUS_ASSIGN | MINUS_ASSIGN 
         | STAR_ASSIGN | SLASH_ASSIGN | MOD_ASSIGN
         ;
```

Exemplos:
```python
x = 10
x += 5
array[0] = 42
obj.campo = valor
```

### Estrutura If-Elif-Else

```yacc
if_stmt: IF LPAREN expression RPAREN NEWLINE statement_block
       | IF LPAREN expression RPAREN NEWLINE statement_block 
         elif_chain_nonnull opt_else
       ;

elif_chain_nonnull: ELIF LPAREN expression RPAREN NEWLINE statement_block
                  | ELIF LPAREN expression RPAREN NEWLINE statement_block 
                    elif_chain_nonnull
                  ;

opt_else: ELSE NEWLINE statement_block
        | /* vazio */
        ;
```

A gramática suporta múltiplos `elif`:
```python
if (x > 0)
    print("positivo")
elif (x < 0)
    print("negativo")
else
    print("zero")
```

### Laços While e For

```yacc
while_stmt: WHILE LPAREN expression RPAREN NEWLINE statement_block
          ;

for_stmt: FOR LPAREN IDENTIFIER IN expression RPAREN NEWLINE statement_block
        | FOR LPAREN IDENTIFIER COMMA IDENTIFIER IN expression RPAREN NEWLINE statement_block
        ;
```

Exemplos:
```python
# While
while (i < 10)
    print(i)
    i++

# For simples
for (item in lista)
    print(item)

# For com índice
for (i, item in lista)
    print(i)
    print(item)
```

### Try-Catch-Finally

```yacc
try_stmt: TRY NEWLINE statement_block catch_list opt_finally
        ;

catch_clause: CATCH LPAREN type IDENTIFIER RPAREN NEWLINE statement_block
            ;

opt_finally: FINALLY NEWLINE statement_block
           | /* vazio */
           ;
```

Exemplo:
```python
try
    resultado = operacao()
catch (int erro)
    print("Erro capturado")
finally
    print("Limpeza")
```

### Return, Break e Continue

```yacc
return_stmt: RETURN expression
           | RETURN
           ;

break_stmt: BREAK
          ;

continue_stmt: CONTINUE
             ;
```

## Expressões

A gramática de expressões segue a precedência correta dos operadores.

### Hierarquia de Precedência (do menor para o maior)

```yacc
expression: or_expr
          ;

or_expr: and_expr 
       | or_expr OR and_expr
       ;

and_expr: not_expr 
        | and_expr AND not_expr
        ;

not_expr: rel_expr 
        | NOT not_expr
        ;

rel_expr: add_expr 
        | rel_expr rel_op add_expr
        ;

add_expr: mult_expr 
        | add_expr PLUS mult_expr 
        | add_expr MINUS mult_expr
        ;

mult_expr: exp_expr 
         | mult_expr STAR exp_expr 
         | mult_expr SLASH exp_expr 
         | mult_expr FLOOR_DIV exp_expr 
         | mult_expr MOD exp_expr 
         | mult_expr MATMUL exp_expr
         ;

exp_expr: unary_expr 
        | unary_expr POWER exp_expr
        ;

unary_expr: postfix_expr 
          | PLUS unary_expr 
          | MINUS unary_expr 
          | INCREMENT unary_expr 
          | DECREMENT unary_expr
          ;

postfix_expr: primary_expr 
            | postfix_expr LBRACKET expression RBRACKET 
            | postfix_expr LPAREN opt_args RPAREN 
            | postfix_expr DOT IDENTIFIER 
            | postfix_expr INCREMENT 
            | postfix_expr DECREMENT
            ;
```

### Tabela de Precedência (Bison)

```yacc
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
```

### Operador IN

O operador `in` é usado para verificar pertencimento:

```python
bool pertence = x in conjunto
```

Na gramática:
```yacc
rel_op: EQUAL | NOT_EQUAL | LESS | GREATER 
      | LESS_EQUAL | GREATER_EQUAL | IN
      ;
```

### Literais

```yacc
primary_expr: literal 
            | IDENTIFIER 
            | LPAREN expression RPAREN 
            | array_literal 
            | set_literal
            ;

literal: INT_LITERAL 
       | FLOAT_LITERAL 
       | BOOL_LITERAL 
       | COMPLEX_LITERAL 
       | STRING_LITERAL 
       | CHAR_LITERAL
       ;

array_literal: LBRACKET expression_list RBRACKET
             | LBRACKET RBRACKET
             ;

set_literal: LBRACE expression_list RBRACE
           | LBRACE RBRACE
           ;
```

## Árvore Sintática Abstrata (AST)

A AST é a estrutura de dados que representa o programa de forma hierárquica.

### Estrutura de um Nó

```c
typedef struct ASTNode {
    NodeType type;
    int line;
    
    union {
        struct { int value; } int_literal;
        struct { double value; } float_literal;
        struct { char* name; } identifier;
        struct {
            OperatorType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        struct {
            TypeSpec* var_type;
            char* name;
            struct ASTNode* initializer;
        } declaration;
        // ... outros tipos de nós
    };
} ASTNode;
```

### Tipos de Nós (35+ tipos)

- **Literais**: `NODE_INT_LITERAL`, `NODE_FLOAT_LITERAL`, `NODE_STRING_LITERAL`, etc.
- **Identificadores**: `NODE_IDENTIFIER`
- **Operações**: `NODE_BINARY_OP`, `NODE_UNARY_OP`
- **Declarações**: `NODE_DECLARATION`, `NODE_ASSIGNMENT`
- **Controle**: `NODE_IF_STMT`, `NODE_WHILE_STMT`, `NODE_FOR_STMT`
- **Funções**: `NODE_FUNCTION_DEF`, `NODE_CALL`, `NODE_RETURN_STMT`
- **Coleções**: `NODE_ARRAY_LITERAL`, `NODE_SET_LITERAL`

### Construção da AST

Durante o parsing, cada regra gramatical constrói um nó da AST:

```yacc
declaration: type IDENTIFIER ASSIGN expression {
    $$ = create_declaration($1, $2, $4, yylineno);
    free($2);
}
```

As ações semânticas (código entre `{ }`) são executadas quando a regra é reduzida.

### Exemplo de AST

Para o código:
```python
def fun void main()
    int x = 42
    print(x)
end def
```

AST gerada:
```
(PROGRAM
  (MAIN
    (FUNCTION main
      (RETURN_TYPE
        (TYPE void)
      )
      (BODY
        (DECL x
          (TYPE int)
          (INIT
            (INT 42)
          )
        )
        (EXPR_STMT
          (CALL
            (ID print)
            (ARGS
              (ID x)
            )
          )
        )
      )
    )
  )
)
```

### Impressão da AST

A AST é impressa de forma hierárquica e indentada:

```c
void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case NODE_INT_LITERAL:
            printf("(INT %d)\n", node->int_literal.value);
            break;
        case NODE_BINARY_OP:
            printf("(%s\n", op_to_string(node->binary_op.op));
            print_ast(node->binary_op.left, indent + 2);
            print_ast(node->binary_op.right, indent + 2);
            break;
        // ... outros casos
    }
}
```

### Liberação de Memória

A AST é liberada recursivamente após o uso:

```c
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_BINARY_OP:
            free_ast(node->binary_op.left);
            free_ast(node->binary_op.right);
            break;
        case NODE_DECLARATION:
            free_type_spec(node->declaration.var_type);
            free(node->declaration.name);
            free_ast(node->declaration.initializer);
            break;
        // ... outros casos
    }
    
    free(node);
}
```

## Resolução de Conflitos

Durante a construção do parser, o Bison pode reportar conflitos quando não consegue decidir entre duas ações. O projeto tem alguns conflitos esperados e aceitáveis.

### Conflito If-Elif-Else (Dangling Else)

```yacc
%nonassoc LOWER_THAN_ELIF
%nonassoc ELIF
%nonassoc ELSE
```

Essa declaração resolve o problema do "dangling else" (a qual `if` um `else` pertence).

### Conflitos Residuais

O parser tem **5 conflitos** conhecidos:
- **3 shift/reduce**: Relacionados a NEWLINE após END DEF e chamadas de método
- **2 reduce/reduce**: Ambiguidade entre NEWLINE como statement ou opt_newlines

Estes conflitos não afetam o funcionamento correto do compilador e identificamos que ocorrem por causa da indentação.

### Verificar Conflitos

Para ver detalhes dos conflitos:

```bash
cat build/parser.output | less
```

O arquivo `parser.output` contém:
- Lista de todos os estados do autômato
- Conflitos encontrados
- Exemplos de derivações conflitantes

## Tratamento de Erros

Quando o parser encontra um erro de sintaxe:

```c
void yyerror(const char* s) {
    fprintf(stderr, "Erro sintático na linha %d: %s\n", yylineno, s);
}
```

Exemplo de erro:
```
Erro sintático na linha 5: syntax error
```

## Exemplo Completo de Parsing

### Entrada

```python
import io

def fun int soma(int a, int b)
    return a + b
end def

def fun void main()
    int resultado = soma(10, 20)
    print(resultado)
end def
```

### Processo

1. **Lexer** gera tokens:
   ```
   IMPORT IDENTIFIER('io') NEWLINE
   DEF FUN INT_TYPE IDENTIFIER('soma') ...
   ```

2. **Parser** reconhece as regras:
   ```
   program → imports main_function
   imports → import_list
   main_function → DEF FUN VOID_TYPE ...
   ```

3. **AST** é construída:
   ```
   PROGRAM
     ├─ IMPORTS
     │   └─ IMPORT(io)
     ├─ DEFINITIONS
     │   └─ FUNCTION(soma)
     │       ├─ PARAMS(a:int, b:int)
     │       └─ BODY
     │           └─ RETURN(a + b)
     └─ MAIN
         └─ FUNCTION(main)
             └─ BODY
                 ├─ DECL(resultado)
                 └─ CALL(print)
   ```

## Estatísticas do Parser

- **Produções da gramática**: ~150
- **Não-terminais**: ~60
- **Estados do autômato**: ~400
- **Conflitos**: 5 (aceitáveis)

## Arquivos Gerados

Ao compilar com `make all`, o Bison gera:

- `build/parser.tab.c` - Código C do parser
- `build/parser.tab.h` - Definições de tokens
- `build/parser.output` - Relatório detalhado da gramática
- `build/parser.tab.o` - Objeto compilado

## Limitações Conhecidas

1. **Comentários de bloco** (`/* */`) não foram implementados
2. **Matriz literal** foi simplificada para evitar conflitos
3. **Tipos customizados** estão na gramática mas não totalmente testados

Essas limitações estão documentadas e podem ser implementadas em versões futuras.

## Testes do Parser

O projeto inclui 15 arquivos de teste que verificam todas as construções da gramática:

```bash
make test-all
```

Alguns testes importantes:
- `control.mf` - If-elif-else e while
- `complex_functions.mf` - Recursão
- `try_catch_const.mf` - Try-catch e constantes
- `break_continue.mf` - Break e continue em loops
- `complex_nested.mf` - Estruturas aninhadas

## Integração com Análise Léxica

O parser obtém tokens do lexer através da função `yylex()`:

```c
// No parser (gerado pelo Bison)
int token = yylex();

// Valor semântico em yylval
int number = yylval.ival;
char* name = yylval.sval;
```


## Referências

- Bison Manual: https://www.gnu.org/software/bison/manual/
- Levine, J. "flex & bison" (2009)
- Appel, A. W. "Modern Compiler Implementation in C" (1998)