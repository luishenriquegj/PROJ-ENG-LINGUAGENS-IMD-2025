# 🚀 Guia Rápido

## Instalação Rápida

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install flex gcc make
```

### macOS

```bash
brew install flex gcc make
```

### Windows (WSL)

```bash
# No Ubuntu dentro do WSL
sudo apt-get install flex gcc make
```

---

## Setup do Projeto

### 1. Criar estrutura de diretórios

```bash
mkdir lexer-matematico
cd lexer-matematico
mkdir tests
```

### 2. Copiar arquivos

Copie para o diretório:

- `lexer.l` (analisador léxico)
- `Makefile`
- `README.md`

### 3. Compilar

```bash
make
```

**Saída esperada:**

```
⚙️  Gerando código C com Flex...
🔨 Compilando o analisador léxico...
✅ Analisador léxico compilado com sucesso!
```

---

## Uso Básico

### Opção 1: Testar com entrada interativa

```bash
./lexer
```

Digite código e pressione `Ctrl+D` (Linux/Mac) ou `Ctrl+Z` (Windows):

```python
int x = 42
float y = 3.14
```

### Opção 2: Testar com arquivo

```bash
# Criar arquivo de teste
echo "int x = 42" > teste.mf

# Executar
./lexer teste.mf
```

### Opção 3: Usar redirecionamento

```bash
echo "float pi = 3.14159" | ./lexer
```

---

## Exemplos Rápidos

### Exemplo 1: Tipos básicos

**Entrada:**

```python
int idade = 25
float altura = 1.75
bool ativo = true
```

**Saída:**

```
INT_TYPE             | int
IDENTIFIER           | idade
ASSIGN               | =
INT_LITERAL          | 25
NEWLINE              | \n
...
```

### Exemplo 2: Função simples

**Entrada:**

```python
def fun int soma(int a, int b)
    return a + b
end def
```

**Saída:**

```
DEF                  | def
FUN                  | fun
INT_TYPE             | int
IDENTIFIER           | soma
LPAREN               | (
...
```

### Exemplo 3: Números complexos

**Entrada:**

```python
complex z = 3+4i
```

**Saída:**

```
COMPLEX_TYPE         | complex
IDENTIFIER           | z
ASSIGN               | =
COMPLEX_LITERAL      | 3+4i
```

---

## Comandos do Makefile

```bash
make              # Compila o lexer
make test         # Executa todos os testes
make test-files   # Cria arquivos de teste
make clean        # Remove arquivos gerados
make help         # Mostra ajuda completa
```

---

## Troubleshooting Rápido

### Erro: "flex: command not found"

```bash
# Ubuntu/Debian
sudo apt-get install flex

# macOS
brew install flex
```

### Erro: "undefined reference to yywrap"

```bash
# Recompile com -lfl
gcc lex.yy.c -o lexer -lfl
```

### Erro: Caractere inválido

Verifique se o arquivo está em UTF-8 sem BOM.

---

## Próximos Passos

1. ✅ Lexer funcionando
2. 📖 Leia o `README.md` completo
3. 📚 Estude o `TUTORIAL_TEORIA.md` para entender a fundo
4. 🧪 Execute `make test` para ver todos os casos
5. ✏️ Modifique `lexer.l` para adicionar novos tokens

---

## Estrutura de Arquivos

```
lexer-matematico/
├── lexer.l              ← Analisador léxico (fonte)
├── Makefile             ← Automação
├── README.md            ← Documentação completa
├── TUTORIAL_TEORIA.md   ← Teoria detalhada
├── QUICK_START.md       ← Este arquivo
├── test_completo.mf   ← Suite de testes
├── tests/               ← Diretório de testes
│   ├── test_tipos.mf
│   ├── test_funcao.mf
│   └── ...
├── lex.yy.c             ← Gerado pelo Flex
└── lexer                ← Executável