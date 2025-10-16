# Analisador Léxico - Linguagem Matemática

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Especificação da Linguagem](#especificação-da-linguagem)
3. [Implementação](#implementação)
4. [Como Usar](#como-usar)
5. [Exemplos](#exemplos)
6. [Referências](#referências)
7. [Tabela de Tokens](#tabela-completa-de-tokens)
8. [Glossário](#glossário)

---

## 🎯 Visão Geral

Nosso analisador léxico (scanner/lexer) foi desenvolvido pra a **Linguagem Matemática**, uma linguagem de programação
imperativa voltada pra o domínio matemático. O objetivo é facilitar a modelagem de problemas matemáticos com uma
sintaxe próxima à notação matemática tradicional.

### Características Principais

- ✅ **Tipagem estática** com tipos matemáticos especializados
- ✅ **Indentação como delimitador** de blocos (estilo Python)
- ✅ **Suporte nativo** a números complexos, matrizes, conjuntos e grafos
- ✅ **Operadores matemáticos** especializados (@, ^, etc.)
- ✅ **Funções trigonométricas** e matemáticas embutidas
- ✅ **Análise de erros** com localização precisa (linha e coluna)
- ✅ **Suporte a comentários** (linhas e blocos)

### Equipe

- Allane Emanoelle
- Franklin Oliveira
- Luís Henrique
- Mateus Santos
- Marcos Martins

**Disciplina:** DIM0548 - Engenharia de Linguagens  
**Instituição:** Universidade Federal do Rio Grande do Norte - Instituto Metrópole Digital (UFRN/IMD)

---

## 🔧 Especificação da Linguagem

### Tokens Reconhecidos

#### 1. Palavras-Chave (Keywords)

Palavras reservadas com significado especial na linguagem:

```
def, end, fun, if, elif, else, while, for, in, return,
break, continue, try, catch, finally, import, const, var,
this, new
```

**Exemplo:**

```python
def fun int soma(int a, int b)
    return a + b
end def
```

#### 2. Tipos Primitivos

| Tipo | Descrição | Exemplo |
|------|-----------|---------|
| `int` | Inteiro | `42`, `-10` |
| `float` | Ponto flutuante | `3.14`, `2.5e-3` |
| `bool` | Booleano | `true`, `false` |
| `complex` | Número complexo | `3+4i`, `2.5i` |
| `char` | Caractere | `'a'`, `'\n'` |
| `string` | Cadeia de caracteres | `"hello"` |
| `void` | Sem retorno | - |
| `matrix` | Matriz | `matrix[float]` |
| `set` | Conjunto | `set[int]` |
| `graph` | Grafo | `graph[int]` |

#### 3. Literais

##### Literais Numéricos

```c
// Inteiros
42
1000
0

// Float
3.14
2.5e10
1.5E-3

// Complexos
3+4i      // Parte real 3, imaginária 4
2.5-1i    // Parte real 2.5, imaginária -1
5i        // Imaginário puro
```

##### Literais de String e Char

```c
"Hello, World!"
"String com \"aspas\" escapadas"
"String com \n quebra de linha"

'a'
'\n'
'\t'
```

##### Literais Booleanos

```c
true
false
```

#### 4. Operadores

##### Operadores Aritméticos

| Operador | Descrição | Exemplo |
|----------|-----------|---------|
| `+` | Adição | `a + b` |
| `-` | Subtração | `a - b` |
| `*` | Multiplicação | `a * b` |
| `/` | Divisão | `a / b` |
| `//` | Divisão inteira | `7 // 2 = 3` |
| `%` | Módulo | `7 % 3 = 1` |
| `^` | Potência | `2 ^ 3 = 8` |
| `@` | Produto matricial | `A @ B` |

##### Operadores de Atribuição

```c
=       // Atribuição simples
+=      // a += b  equivale a  a = a + b
-=      // a -= b  equivale a  a = a - b
*=      // a *= b  equivale a  a = a * b
/=      // a /= b  equivale a  a = a / b
%=      // a %= b  equivale a  a = a % b
```

##### Operadores de Incremento/Decremento

```c
++      // Incremento: x++ ou ++x
--      // Decremento: x-- ou --x
```

##### Operadores Relacionais

```c
==      // Igual a
!=      // Diferente de
<       // Menor que
>       // Maior que
<=      // Menor ou igual
>=      // Maior ou igual
in      // Pertence (pra conjuntos/iterações)
```

##### Operadores Lógicos

```c
and     // E lógico
or      // OU lógico
not     // NÃO lógico
&&      // E lógico (alternativa simbólica)
||      // OU lógico (alternativa simbólica)
!       // NÃO lógico (alternativa simbólica)
```

#### 5. Funções Matemáticas Embutidas

##### Trigonométricas

```c
sin, sen    // Seno
cos         // Cosseno
tan         // Tangente
sec         // Secante
csc, cosec  // Cossecante
cot, cotan  // Cotangente
asin, arcsin // Arco-seno
acos, arccos // Arco-cosseno
atan, arctan // Arco-tangente
```

##### Outras Funções Matemáticas

```c
sqrt(x)      // Raiz quadrada
abs(x)       // Valor absoluto
log(x)       // Logaritmo (base 10)
ln(x)        // Logaritmo natural
exp(x)       // e^x
```

##### Funções de Arrays/Listas

```c
length()     // Tamanho da coleção
add(x)       // Adiciona elemento
push(x)      // Empilha elemento
pop()        // Desempilha elemento
remove(i)    // Remove elemento no índice i
insert(i, x) // Insere x no índice i
clear()      // Limpa a coleção
isEmpty()    // Verifica se está vazia
```

##### Funções de Matrizes

```c
transpose()     // Matriz transposta
determinant()   // Determinante
inverse()       // Matriz inversa
rows()          // Número de linhas
cols()          // Número de colunas
```

##### Funções de Grafos

```c
nodeCount()     // Número de nós
neighbors(v)    // Vizinhos do vértice v
weight(u, v)    // Peso da aresta u→v
```

##### Funções de I/O

```c
print(x)        // Imprime sem quebra de linha
println(x)      // Imprime com quebra de linha
read()          // Lê entrada do usuário
range(a, b)     // Gera sequência de a até b
```

#### 6. Delimitadores

```c
(  )    // Parênteses
[  ]    // Colchetes
{  }    // Chaves
,       // Vírgula
;       // Ponto e vírgula
:       // Dois pontos
.       // Ponto (acesso de membros)
```

#### 7. Tokens Especiais: Indentação

**Nota:** A versão atual do lexer (lexer.l) trata indentação de forma **simplificada**. O suporte completo a
INDENT/DEDENT com stack de indentação foi documentado no arquivo original em C, mas não está totalmente implementado no
arquivo Flex fornecido.

Implementação futura deve incluir:

- Stack pra gerenciar níveis de indentação
- Impressão de tokens INDENT quando indentação aumenta
- Impressão de tokens DEDENT quando indentação diminui
- Tratamento especial de linhas vazias

**Regra esperada:** Cada nível de indentação = 4 espaços (ou 1 tab)

#### 8. Comentários

Dois estilos suportados:

```python
// Comentário de linha - tudo após // até o final da linha
x = 42  // Comentário no fim da linha

/* Comentário de bloco
   pode ocupar múltiplas linhas
   /* aninhamento NÃO é suportado */
```

---

## 💻 Implementação

### Arquitetura do Analisador

```
┌─────────────────────────────────────────┐
│   Código-fonte (arquivo .mf)            │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│   Flex - lexer.l                        │
│   ├─ Expressões regulares               │
│   ├─ Ações (código C)                   │
│   └─ Seções organizadas                 │
└────────────────┬────────────────────────┘
                 │
                 ▼ (gera)
┌─────────────────────────────────────────┐
│   lex.yy.c (código C gerado)            │
│   └─ DFA implementado com tabelas       │
└────────────────┬────────────────────────┘
                 │
                 ▼ (compila)
┌─────────────────────────────────────────┐
│   Executável (a.out / lexer)            │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│   Stream de Tokens                      │
│   INT_TYPE | float | ASSIGN | ...       │
└─────────────────────────────────────────┘
```

### Estrutura do Arquivo lexer.l

#### Seção 1: Declarações C

```c
%{
#include <stdio.h>

int line_number = 1;      // Contador de linhas
int column_number = 1;    // Contador de colunas

void update_column() {
    column_number += yyleng;
}

void report_token(const char* token_type, const char* lexeme) {
    printf("%-20s | %-30s | Line: %d, Col: %d\n", 
           token_type, lexeme, line_number, column_number);
    update_column();
}
%}
```

#### Seção 2: Regras (Padrão → Ação)

Organizada em 13 seções:

1. **Whitespace e Newlines** - Tratamento de espaços em branco
2. **Comentários** - Comentários de linha (//) e bloco (/* */)
3. **Palavras-chave** - Keywords da linguagem
4. **Tipos Primitivos** - int, float, bool, etc.
5. **Literais Booleanos e NULL** - true, false, null
6. **Operadores Lógicos** - and, or, not
7. **Funções Matemáticas** - sin, cos, sqrt, etc.
8. **Literais Numéricos** - Inteiros, floats, complexos
9. **Literais de String e Char** - Strings e caracteres com escape
10. **Identificadores** - Nomes de variáveis e funções
11. **Operadores** - Aritméticos, lógicos, relacionais
12. **Delimitadores e Pontuação** - Parênteses, colchetes, etc.
13. **Tratamento de Erros** - Caracteres inválidos

#### Seção 3: Código C Auxiliar

```c
int yywrap(void) {
    return 1;  // Indica fim de entrada
}

int main(int argc, char** argv) {
    // Abre arquivo ou usa stdin
    // Imprime cabeçalho
    // Executa yylex()
    // Imprime estatísticas
}
```

### Destaques da Implementação

#### 1. Tratamento de Números Complexos

Reconhece várias formas:

```lex
([0-9]+|[0-9]+\.[0-9]+)[+-][0-9]+[ij]    /* 3+4i, 2.5-1.2i */
([0-9]+|[0-9]+\.[0-9]+)[ij]              /* 5i, 2.3i (puro) */
```

#### 2. Comentários

```lex
"//".*                                   /* Linha inteira */
"/*"([^*]|"*"+[^*/])*"*"+"/"            /* Bloco multi-linha */
```

#### 3. Strings com Escape Sequences

```lex
\"([^\"\\]|\\.)*\"

Explicação:
  [^\"\\]    - Qualquer caractere exceto " e \
  |          - OU
  \\.        - Barra invertida seguida de qualquer char
  *          - Zero ou mais vezes
```

Suporta: `"Olá\nMundo\t!"`

#### 4. Tratamento de Erros

```c
. { 
    fprintf(stderr, "ERRO LÉXICO: Caractere inválido '%s' na linha %d, coluna %d\n",
            yytext, line_number, column_number);
    update_column();
}
```

#### 5. Rastreamento de Posição

- `line_number`: Incrementado em newlines
- `column_number`: Incrementado com cada caractere reconhecido
- Exibido em cada token: `Line: X, Col: Y`

---

## 🚀 Como Usar

### Pré-requisitos

- **Flex:** Gerador de analisadores léxicos (versão 2.6+)
- **GCC:** Compilador C (versão 5.0+)
- **Make:** (opcional, pra automação)

### Instalação do Flex

#### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install flex
```

#### macOS

```bash
brew install flex
```

#### Windows (WSL/MinGW)

```bash
# WSL - Ubuntu
sudo apt-get install flex

# MinGW (no terminal do MinGW)
mingw-get install msys-flex
```

### Compilação

#### Método 1: Linha de comando

```bash
# Passo 1: Gerar código C a partir do lexer.l
flex lexer.l

# Passo 2: Compilar
gcc lex.yy.c -o lexer
```

#### Método 2: Com a biblioteca libfl

```bash
flex lexer.l
gcc lex.yy.c -o lexer -lfl
```

#### Método 3: Com Makefile (recomendado)

```makefile
CC = gcc
FLEX = flex

all: lexer

lexer: lex.yy.c
	$(CC) -o lexer lex.yy.c

lex.yy.c: lexer.l
	$(FLEX) lexer.l

clean:
	rm -f lexer lex.yy.c lex.yy.h

.PHONY: all clean
```

Uso:

```bash
make           # Compila
make clean     # Remove gerados
```

### Execução

#### Opção 1: Ler de um arquivo

```bash
./lexer programa.mf
```

#### Opção 2: Entrada padrão (stdin)

```bash
./lexer
# Digite o código e pressione Ctrl+D (Linux/Mac) ou Ctrl+Z (Windows)
```

#### Opção 3: Redirecionamento

```bash
./lexer < entrada.txt
```

#### Opção 4: Pipeline

```bash
echo "int x = 42" | ./lexer
```

#### Opção 5: Salvar saída

```bash
./lexer programa.mf > tokens.txt
./lexer programa.mf 2> erros.txt
```

### Saída

O analisador produz uma tabela formatada:

```
============================================================================
ANALISADOR LÉXICO - LINGUAGEM MATEMÁTICA
============================================================================
TOKEN                | LEXEMA                         | Posição
----------------------------------------------------------------------------
INT_TYPE             | int                            | Line: 1, Col: 1
IDENTIFIER           | x                              | Line: 1, Col: 5
ASSIGN               | =                              | Line: 1, Col: 7
INT_LITERAL          | 42                             | Line: 1, Col: 9
============================================================================
Análise léxica concluída com sucesso!
Total de linhas processadas: 1
============================================================================
```

---

## 📝 Exemplos

### Exemplo 1: Programa Simples

**Arquivo:** `exemplo1.mf`

```python
def fun int soma(int a, int b)
    return a + b
end def
```

**Comando:**

```bash
./lexer exemplo1.mf
```

**Tokens gerados:**

```
DEF              | def
FUN              | fun
INT_TYPE         | int
IDENTIFIER       | soma
LPAREN           | (
INT_TYPE         | int
IDENTIFIER       | a
COMMA            | ,
INT_TYPE         | int
IDENTIFIER       | b
RPAREN           | )
NEWLINE          | \n
RETURN           | return
IDENTIFIER       | a
PLUS             | +
IDENTIFIER       | b
END              | end
FUN              | fun
```

### Exemplo 2: Números Complexos

**Entrada:**

```python
complex z1 = 3+4i
complex z2 = 2.5-1.2i
complex z3 = 5i
```

**Tokens:**

```
COMPLEX_TYPE     | complex
IDENTIFIER       | z1
ASSIGN           | =
COMPLEX_LITERAL  | 3+4i

COMPLEX_TYPE     | complex
IDENTIFIER       | z2
ASSIGN           | =
COMPLEX_LITERAL  | 2.5-1.2i

COMPLEX_TYPE     | complex
IDENTIFIER       | z3
ASSIGN           | =
COMPLEX_LITERAL  | 5i
```

### Exemplo 3: Operações Matriciais

**Entrada:**

```python
matrix[float] A = [[1.0, 2.0], [3.0, 4.0]]
matrix[float] B = A @ A.transpose()
float det = A.determinant()
```

**Tokens principais:**

```
MATRIX_TYPE      | matrix
LBRACKET         | [
FLOAT_TYPE       | float
RBRACKET         | ]
IDENTIFIER       | A
ASSIGN           | =
LBRACKET         | [
LBRACKET         | [
FLOAT_LITERAL    | 1.0
...
MATMUL           | @
DOT              | .
TRANSPOSE        | transpose
...
```

### Exemplo 4: Estruturas de Controle

**Entrada:**

```python
if (x > 0)
    print("Positivo")
elif (x < 0)
    print("Negativo")
else
    print("Zero")
```

**Tokens (parcial):**

```
IF               | if
LPAREN           | (
IDENTIFIER       | x
GREATER          | >
INT_LITERAL      | 0
RPAREN           | )
NEWLINE          | \n
PRINT            | print
LPAREN           | (
STRING_LITERAL   | "Positivo"
RPAREN           | )
...
```

### Exemplo 5: Comentários

**Entrada:**

```python
// Isso é um comentário de linha
int x = 10  // Comentário no fim da linha

/* Isso é um
   comentário de
   bloco */
float y = 3.14
```

**Saída:** Comentários são ignorados, apenas os tokens do código são impressos.

---

## 📖 Referências

### Livros

1. **Sebesta, Robert W.** *Concepts of Programming Languages*, 11ª edição.
    - Capítulo 3: Describing Syntax and Semantics
    - Capítulo 4: Lexical and Syntax Analysis

2. **Appel, Andrew W.** *Modern Compiler Implementation in C*.
    - Capítulo 2: Lexical Analysis

3. **Aho, Sethi, Ullman.** *Compilers: Principles, Techniques, and Tools*.
    - Capítulo 3: Lexical Analysis

4. **Slonneger & Kurtz.** *Formal Syntax and Semantics of Programming Languages*.
    - Capítulo 1: Specifying Syntax

### Ferramentas

- **Flex (Fast Lex):** https://github.com/westes/flex
- **GNU Flex Manual:** https://westes.github.io/flex/manual/
- **Lex & Yacc:** https://www.oreilly.com/library/view/lex-yacc/9781565920583/

---

## 🔍 Tabela Completa de Tokens

| Token | Lexema(s) | Categoria |
|-------|-----------|-----------|
| `DEF` | def | Palavra-chave |
| `END` | end | Palavra-chave |
| `FUN` | fun | Palavra-chave |
| `IF` | if | Palavra-chave |
| `ELIF` | elif | Palavra-chave |
| `ELSE` | else | Palavra-chave |
| `WHILE` | while | Palavra-chave |
| `FOR` | for | Palavra-chave |
| `IN` | in | Palavra-chave |
| `RETURN` | return | Palavra-chave |
| `BREAK` | break | Palavra-chave |
| `CONTINUE` | continue | Palavra-chave |
| `TRY` | try | Palavra-chave |
| `CATCH` | catch | Palavra-chave |
| `FINALLY` | finally | Palavra-chave |
| `IMPORT` | import | Palavra-chave |
| `CONST` | const | Palavra-chave |
| `VAR` | var | Palavra-chave |
| `THIS` | this | Palavra-chave |
| `NEW` | new | Palavra-chave |
| `INT_TYPE` | int | Tipo primitivo |
| `FLOAT_TYPE` | float | Tipo primitivo |
| `BOOL_TYPE` | bool | Tipo primitivo |
| `COMPLEX_TYPE` | complex | Tipo primitivo |
| `CHAR_TYPE` | char | Tipo primitivo |
| `STRING_TYPE` | string | Tipo primitivo |
| `VOID_TYPE` | void | Tipo primitivo |
| `MATRIX_TYPE` | matrix | Tipo composto |
| `SET_TYPE` | set | Tipo composto |
| `GRAPH_TYPE` | graph | Tipo composto |
| `TRUE` | true | Literal booleano |
| `FALSE` | false | Literal booleano |
| `NULL` | null | Literal especial |
| `AND` | and | Operador lógico |
| `OR` | or | Operador lógico |
| `NOT` | not | Operador lógico |
| `SIN` | sin, sen | Função matemática |
| `COS` | cos | Função matemática |
| `TAN` | tan | Função matemática |
| `SEC` | sec | Função matemática |
| `CSC` | csc, cosec | Função matemática |
| `COT` | cot, cotan | Função matemática |
| `ASIN` | asin, arcsin | Função matemática |
| `ACOS` | acos, arccos | Função matemática |
| `ATAN` | atan, arctan | Função matemática |
| `SQRT` | sqrt | Função matemática |
| `ABS` | abs | Função matemática |
| `LOG` | log | Função matemática |
| `LN` | ln | Função matemática |
| `EXP` | exp | Função matemática |
| `LENGTH` | length | Função de array |
| `ADD` | add | Função de array |
| `PUSH` | push | Função de array |
| `POP` | pop | Função de array |
| `REMOVE` | remove | Função de array |
| `INSERT` | insert | Função de array |
| `CLEAR` | clear | Função de array |
| `IS_EMPTY` | isEmpty | Função de array |
| `TRANSPOSE` | transpose | Função de matriz |
| `DETERMINANT` | determinant | Função de matriz |
| `INVERSE` | inverse | Função de matriz |
| `ROWS` | rows | Função de matriz |
| `COLS` | cols | Função de matriz |
| `NODE_COUNT` | nodeCount | Função de grafo |
| `NEIGHBORS` | neighbors | Função de grafo |
| `WEIGHT` | weight | Função de grafo |
| `PRINT` | print | Função de I/O |
| `PRINTLN` | println | Função de I/O |
| `READ` | read | Função de I/O |
| `RANGE` | range | Função de I/O |
| `PLUS` | + | Operador aritmético |
| `MINUS` | - | Operador aritmético |
| `STAR` | * | Operador aritmético |
| `SLASH` | / | Operador aritmético |
| `FLOOR_DIV` | // | Operador aritmético |
| `MOD` | % | Operador aritmético |
| `POWER` | ^ | Operador aritmético |
| `MATMUL` | @ | Operador matricial |
| `ASSIGN` | = | Operador de atribuição |
| `PLUS_ASSIGN` | += | Operador de atribuição |
| `MINUS_ASSIGN` | -= | Operador de atribuição |
| `STAR_ASSIGN` | *= | Operador de atribuição |
| `SLASH_ASSIGN` | /= | Operador de atribuição |
| `MOD_ASSIGN` | %= | Operador de atribuição |
| `INCREMENT` | ++ | Operador unário |
| `DECREMENT` | -- | Operador unário |
| `EQUAL` | == | Operador relacional |
| `NOT_EQUAL` | != | Operador relacional |
| `LESS` | < | Operador relacional |
| `GREATER` | > | Operador relacional |
| `LESS_EQUAL` | <= | Operador relacional |
| `GREATER_EQUAL` | >= | Operador relacional |
| `LOGICAL_AND` | && | Operador lógico |
| `LOGICAL_OR` | \|\| | Operador lógico |
| `LOGICAL_NOT` | ! | Operador lógico |
| `LPAREN` | ( | Delimitador |
| `RPAREN` | ) | Delimitador |
| `LBRACKET` | [ | Delimitador |
| `RBRACKET` | ] | Delimitador |
| `LBRACE` | { | Delimitador |
| `RBRACE` | } | Delimitador |      
| `COMMA` | , | Delimitador |
| `SEMICOLON` | ; |
| `COLON` | : | Delimitador |
| `DOT` | . | Delimitador |
| `IDENTIFIER` | [a-zA-Z][a-zA-Z0-9_]* | Identificador |
| `INT_LITERAL` | [0-9]+ | Literal numérico |
| `FLOAT_LITERAL` | [0-9]+\.[0-9]+ | Literal numérico |
| `COMPLEX_LITERAL` | [0-9]+[+-][0-9]+i | Literal numérico |
| `STRING_LITERAL` | "..." | Literal textual |
| `CHAR_LITERAL` | '.' | Literal textual |
| `INDENT` | (aumento de indentação) | Estrutural |
| `DEDENT` | (diminuição de indentação) | Estrutural |
| `NEWLINE` | \n | Estrutural |

---

## 🎓Glossário

- **ER (Expressão Regular):** Notação pra descrever padrões de texto
- **NFA:** Autômato Finito Não-Determinístico
- **DFA:** Autômato Finito Determinístico
- **Flex:** Ferramenta geradora de analisadores léxicos
- **yytext:** Variável Flex que contém o lexema atual
- **yyleng:** Variável Flex com o tamanho do lexema atual
- **yyin:** Stream de entrada do Flex
- **yylex():** Função principal do analisador gerado pelo Flex

---
