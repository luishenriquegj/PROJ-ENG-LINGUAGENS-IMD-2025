# Guia de Execução

Este guia mostra como instalar, compilar e usar o compilador da Linguagem Matemática.

> **Documentação relacionada:**
> - [README principal](README.md) - Visão geral do projeto
> - [Documentação do Léxico](DOCUMENTACAO_LEXICO.md) - Análise léxica
> - [Documentação do Sintático](DOCUMENTACAO_SINTATICO.md) - Análise sintática

## Índice

1. [Instalação](#instalação)
2. [Compilação](#compilação)
3. [Uso Básico](#uso-básico)
4. [Exemplos](#exemplos)
5. [Solução de Problemas](#solução-de-problemas)

## Instalação

### Pré-requisitos

O projeto precisa de:
- **Flex** (gerador de analisadores léxicos)
- **Bison** (gerador de parsers)
- **GCC** (compilador C)
- **Make** (ferramenta de build)

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install flex bison gcc make
```

### macOS

```bash
brew install flex bison gcc make
```

Se necessário, adicione ao PATH:

```bash
echo 'export PATH="/usr/local/opt/flex/bin:$PATH"' >> ~/.zshrc
echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Fedora/RHEL

```bash
sudo dnf install flex bison gcc make
```

### Windows (WSL)

Recomendamos usar o Windows Subsystem for Linux:

```bash
# No PowerShell como administrador
wsl --install

# Dentro do Ubuntu no WSL
sudo apt-get install flex bison gcc make
```

### Verificar Instalação

```bash
flex --version    # Deve mostrar versão 2.6.x ou superior
bison --version   # Deve mostrar versão 3.x ou superior
gcc --version     # Deve mostrar versão 9.x ou superior
make --version    # Deve mostrar versão 4.x ou superior
```

## Compilação

### Clonar/Obter o Projeto

```bash
cd PROJ-ENG-LINGUAGENS-IMD-2025
```

### Compilar o Compilador

```bash
# Limpar compilações anteriores (se houver)
make clean

# Compilar tudo
make all
```

Saída esperada:
```
mkdir -p build
flex -o build/lex.yy.c src/lexer.l
bison -Wcounterexamples -Wconflicts-sr -Wconflicts-rr -v -d -o build/parser.tab.c src/parser.y
gcc -Wall -Wno-unused-function -g -D_GNU_SOURCE -Ibuild -c -o build/lex.yy.o build/lex.yy.c
gcc -Wall -Wno-unused-function -g -D_GNU_SOURCE -Ibuild -Isrc -c -o build/parser.tab.o build/parser.tab.c
gcc -Wall -Wno-unused-function -g -D_GNU_SOURCE -Ibuild -Isrc -c -o build/ast.o src/ast.c
gcc -Wall -Wno-unused-function -g -D_GNU_SOURCE -Ibuild -Isrc -c -o build/main.o src/main.c
gcc -Wall -Wno-unused-function -g -D_GNU_SOURCE -o mathc build/lex.yy.o build/parser.tab.o build/ast.o build/main.o
✅ Compilador gerado com sucesso!
```

### Comandos Make Disponíveis

```bash
make help        # Ver todos os comandos disponíveis
make all         # Compilar o compilador
make test-all    # Executar todos os testes
make test        # Executar testes básicos
make tokens      # Compilar analisador de tokens
make clean       # Limpar arquivos compilados
```

## Uso Básico

### "Compilar" um Programa

```bash
./mathc arquivo.mf
```

Exemplo:
```bash
./mathc tests/hello_world.mf
```

Saída esperada:
```
============================================================================
COMPILADOR - LINGUAGEM MATEMÁTICA
UFRN - Engenharia de Linguagens
============================================================================

Analisando arquivo: tests/hello_world.mf

Iniciando análise sintática...

✓ Análise sintática concluída com SUCESSO!

============================================================================
ÁRVORE SINTÁTICA ABSTRATA (AST)
============================================================================

Total de nós na AST: 7

(PROGRAM
  (IMPORTS
    (IMPORT io)
  )
  (MAIN
    (FUNCTION main
      (RETURN_TYPE
        (TYPE void)
      )
      (BODY
        (EXPR_STMT
          (CALL
            (ID print)
            (ARGS
              (STRING "Hello, World!")
            )
          )
        )
      )
    )
  )
)

============================================================================

Liberando memória da AST...
Memória liberada com sucesso!
```

### Analisar Apenas os Tokens

Para ver os tokens reconhecidos sem fazer parsing:

```bash
make tokens
./test_tokens arquivo.mf
```

Exemplo:
```bash
./test_tokens tests/hello_world.mf
```

Saída:
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
...
```

### Executar Todos os Testes

```bash
make test-all
```

Ou diretamente:
```bash
./run_tests.sh
```

Saída esperada:
```
========================================
COMPILADOR DE LINGUAGEM MATEMÁTICA
Teste de Validação Final
========================================

Testando hello_world.mf... ✅ PASSOU
Testando variables.mf... ✅ PASSOU
Testando arrays.mf... ✅ PASSOU
...

========================================
RESULTADO FINAL
========================================
✅ Testes aprovados: 15
❌ Testes falhados: 0
========================================
🎉 TODOS OS TESTES PASSARAM!
```

## Exemplos

### Exemplo 1: Hello World

Crie um arquivo `hello.mf`:

```python
import io

def fun void main()
    print("Hello, World!")
end def
```

Compile:
```bash
./mathc hello.mf
```

### Exemplo 2: Variáveis e Operações

Crie `operacoes.mf`:

```python
import io

def fun void main()
    int a = 10
    int b = 3
    
    int soma = a + b
    int div_inteira = a // b
    int resto = a % b
    int potencia = 2 ^ 8
    
    print(soma)
    print(div_inteira)
    print(resto)
    print(potencia)
end def
```

### Exemplo 3: Estruturas de Controle

Crie `controle.mf`:

```python
import io

def fun void main()
    int x = 10
    
    if (x > 0)
        print("Positivo")
    elif (x < 0)
        print("Negativo")
    else
        print("Zero")
    
    int i = 0
    while (i < 5)
        print(i)
        i++
end def
```

### Exemplo 4: Funções

Crie `funcoes.mf`:

```python
import math

def fun int fibonacci(int n)
    if (n <= 1)
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)
end def

def fun void main()
    int fib = fibonacci(10)
    print(fib)
end def
```

### Exemplo 5: Números Complexos

Crie `complexos.mf`:

```python
import io

def fun void main()
    complex z1 = 3+4i
    complex z2 = 2-1i
    
    complex soma = z1 + z2
    print(soma)
end def
```

### Exemplo 6: Arrays e For

Crie `arrays.mf`:

```python
import io

def fun void main()
    int[] numeros = [1, 2, 3, 4, 5]
    
    for (num in numeros)
        print(num)
    
    # For com índice
    for (i, val in numeros)
        print(i)
        print(val)
end def
```

### Exemplo 7: Conjuntos e Operador IN

Crie `conjuntos.mf`:

```python
import io

def fun void main()
    set[int] primos = {2, 3, 5, 7, 11}
    
    int x = 5
    bool pertence = x in primos
    
    if (pertence)
        print("É primo")
    else
        print("Não é primo")
end def
```

## Estrutura de Arquivos Gerados

Após compilar com `make all`, a estrutura fica:

```
.
├── mathc               # Executável do compilador
├── test_tokens         # Executável do analisador de tokens
└── build/              # Arquivos intermediários
    ├── lex.yy.c        # Lexer gerado pelo Flex
    ├── parser.tab.c    # Parser gerado pelo Bison
    ├── parser.tab.h    # Definições de tokens
    ├── parser.output   # Relatório da gramática
    └── *.o             # Arquivos objeto
```