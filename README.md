# Compilador para Linguagem Matemática

Este projeto implementa um compilador para uma linguagem de programação imperativa voltada para o domínio matemático. A linguagem foi projetada para facilitar a escrita de algoritmos matemáticos com uma sintaxe clara e intuitiva.

## Sobre o Projeto

### Objetivo

Desenvolver um compilador funcional que processa uma linguagem de programação com características especiais para aplicações matemáticas, incluindo tipos de dados específicos (números complexos, matrizes, conjuntos), operadores matemáticos e uma sintaxe baseada em indentação significativa.

### Disciplina

- **Curso**: Engenharia de Linguagens (DIM0548)
- **Instituição**: Universidade Federal do Rio Grande do Norte - Instituto Metrópole Digital
- **Período**: 2025.2

### Equipe

- Allane Emanoelle
- Franklin Oliveira
- Luís Henrique
- Marcos Martins
- Mateus Santos

## Características da Linguagem

### Sintaxe

A linguagem utiliza indentação significativa (similar ao Python) para delimitar blocos de código. As funções são definidas com `def fun` e finalizadas com `end def`.

```python
import io

def fun void main()
    int x = 42
    print(x)
end def
```

### Tipos de Dados

**Primitivos:**
- `int` - Números inteiros
- `float` - Números de ponto flutuante  
- `bool` - Valores booleanos (true/false)
- `complex` - Números complexos (ex: 3+4i)
- `char` - Caracteres
- `string` - Cadeias de caracteres
- `void` - Ausência de tipo (usado em funções)

**Compostos:**
- `tipo[]` - Arrays
- `matrix[tipo]` - Matrizes
- `set[tipo]` - Conjuntos
- `graph[tipo]` - Grafos

### Operadores

**Aritméticos:** `+` `-` `*` `/` `//` `%` `^` `@`
- `//` - Divisão inteira
- `^` - Potenciação
- `@` - Produto matricial

**Atribuição:** `=` `+=` `-=` `*=` `/=` `%=`

**Incremento/Decremento:** `++` `--`

**Relacionais:** `==` `!=` `<` `>` `<=` `>=` `in`

**Lógicos:** `and` `or` `not` (ou `&&` `||` `!`)

### Estruturas de Controle

```python
# Condicional
if (x > 0)
    print("positivo")
elif (x < 0)
    print("negativo")
else
    print("zero")

# Laço while
while (i < 10)
    print(i)
    i++

# Laço for
for (item in lista)
    print(item)

# Break e continue
while (condicao)
    if (parar)
        break
    if (pular)
        continue
```

### Funções

```python
# Função com retorno
def fun int fibonacci(int n)
    if (n <= 1)
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)
end def

# Função principal
def fun void main()
    int resultado = fibonacci(10)
    print(resultado)
end def
```

### Funções Matemáticas

A linguagem possui funções matemáticas embutidas:
- Trigonométricas: `sin`, `cos`, `tan`
- Outras: `sqrt`, `abs`, `log`, `ln`, `exp`
- I/O: `print`, `range`

### Tratamento de Exceções

```python
try
    resultado = operacao_arriscada()
catch (int erro)
    print("Erro capturado")
finally
    print("Limpeza")
```

## Exemplos Práticos

### Hello World

```python
import io

def fun void main()
    print("Hello, World!")
end def
```

### Números Complexos

```python
import io

def fun void main()
    complex z1 = 3+4i
    complex z2 = 2-1i
    complex resultado = z1 + z2
    print(resultado)
end def
```

### Recursão

```python
import math

def fun int fatorial(int n)
    if (n <= 1)
        return 1
    return n * fatorial(n - 1)
end def

def fun void main()
    int fat = fatorial(5)
    print(fat)
end def
```

### Arrays e Conjuntos

```python
import io

def fun void main()
    int[] numeros = [1, 2, 3, 4, 5]
    set[int] conjunto = {1, 2, 3}
    
    bool tem = 3 in conjunto
    print(tem)
    
    for (num in numeros)
        print(num)
end def
```

## Arquitetura do Compilador

O compilador é composto por três componentes principais:

1. **Analisador Léxico** (Flex)
   - Reconhece tokens da linguagem
   - Gerencia indentação e gera INDENT/DEDENT
   - Detecta e reporta erros léxicos

2. **Analisador Sintático** (Bison)
   - Parser LALR(1) bottom-up
   - Valida a estrutura do programa
   - Constrói a Árvore Sintática Abstrata (AST)

3. **AST (Árvore Sintática Abstrata)**
   - Representa a estrutura do programa
   - Facilita análises e transformações futuras
   - Base para geração de código

## Como Usar

### Instalação

Primeiro, instale as dependências necessárias:

```bash
# Ubuntu/Debian
sudo apt-get install flex bison gcc make

# macOS
brew install flex bison gcc make
```

### Compilação

```bash
# Compilar o compilador
make clean
make all
```

### Executando

```bash
# Compilar um programa
./mathc arquivo.mf

# Executar todos os testes
make test-all
```

Para mais detalhes sobre instalação e uso, consulte o [Guia de Execução](GUIA_EXECUCAO.md).

## Documentação Técnica

- **[Guia de Execução](GUIA_EXECUCAO.md)** - Como instalar, compilar e usar o compilador
- **[Documentação do Analisador Léxico](DOCUMENTACAO_LEXICO.md)** - Detalhes sobre a análise léxica
- **[Documentação do Analisador Sintático](DOCUMENTACAO_SINTATICO.md)** - Detalhes sobre a análise sintática

## Estrutura do Projeto

```
.
├── src/
│   ├── lexer.l          # Especificação do analisador léxico
│   ├── parser.y         # Gramática do analisador sintático
│   ├── ast.c/ast.h      # Implementação da AST
│   └── main.c           # Programa principal
├── tests/               # Arquivos de teste (.mf)
├── Makefile            # Sistema de build
└── run_tests.sh        # Script de testes automatizados
```

## Testes

O projeto inclui 28 arquivos de teste cobrindo todas as funcionalidades:

```bash
# Executar todos os testes
./run_tests.sh

# Resultado esperado:
# ✅ Testes aprovados: 28
# ❌ Testes falhados: 0
```

Os testes cobrem:
- Tipos de dados (primitivos e compostos)
- Operadores (aritméticos, lógicos, relacionais)
- Estruturas de controle (if, while, for)
- Funções (incluindo recursão)
- Números complexos
- Arrays e conjuntos
- Break e continue
- Try-catch-finally

## Status do Projeto

- ✅ Analisador léxico: Completo
- ✅ Analisador sintático: Completo
- ✅ AST: Implementada
- ✅ Testes: 15/15 passando
- ⏳ Análise semântica: em breve!
- ⏳ Geração de código: em breve!

## Referências

- Appel, Andrew W. "Modern Compiler Implementation in C" (1998)
- Levine, John. "flex & bison" (2009)
- Aho, Ullman. "Compilers: Principles, Techniques, and Tools" (2006)

---
