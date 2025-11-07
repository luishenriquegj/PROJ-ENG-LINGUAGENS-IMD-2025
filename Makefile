SHELL := /bin/bash

CC = gcc
CFLAGS = -Wall -Wno-unused-function -g -D_GNU_SOURCE
LEX = flex
YACC = bison

# Diretórios
BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests

# Arquivos de saída
TARGET = mathc
TEST_TOKENS = test_tokens

# Objetos do compilador
OBJS = $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/parser.tab.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/main.o

# Criar diretório build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Regra principal: compila o compilador e a ferramenta de tokens
all: $(BUILD_DIR) $(TARGET) $(TEST_TOKENS)

# Rebuild: clean + all
rebuild:
	@$(MAKE) -s clean
	@$(MAKE) -s all

# Gerar o executável final
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✅ Compilador gerado com sucesso!"

# Gerar ferramenta de teste de tokens
$(TEST_TOKENS): $(BUILD_DIR) $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/parser.tab.o $(BUILD_DIR)/ast.o test_tokens.c
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -o $@ test_tokens.c $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/parser.tab.o $(BUILD_DIR)/ast.o
	@echo "✅ Test tokens gerado!"

# Compilar main.c
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/ast.h $(BUILD_DIR)/parser.tab.h
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(SRC_DIR) -c -o $@ $(SRC_DIR)/main.c

# Compilar ast.c
$(BUILD_DIR)/ast.o: $(SRC_DIR)/ast.c $(SRC_DIR)/ast.h
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(SRC_DIR) -c -o $@ $(SRC_DIR)/ast.c

# Compilar o parser gerado pelo Bison
$(BUILD_DIR)/parser.tab.o: $(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h $(SRC_DIR)/ast.h
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(SRC_DIR) -c -o $@ $(BUILD_DIR)/parser.tab.c

# Gerar o parser com Bison
$(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h: $(SRC_DIR)/parser.y
	$(YACC) -Wcounterexamples -Wconflicts-sr -Wconflicts-rr -v -d -o $(BUILD_DIR)/parser.tab.c $(SRC_DIR)/parser.y

# Compilar o lexer gerado pelo Flex
$(BUILD_DIR)/lex.yy.o: $(BUILD_DIR)/lex.yy.c $(BUILD_DIR)/parser.tab.h
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -c -o $@ $(BUILD_DIR)/lex.yy.c

# Gerar o lexer com Flex
$(BUILD_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l
	$(LEX) -o $(BUILD_DIR)/lex.yy.c $(SRC_DIR)/lexer.l

# Testar tokens do hello_world.mf
tokens: $(TEST_TOKENS)
	@echo -e "\n🔍 Analisando tokens de hello_world.mf...\n"
	./$(TEST_TOKENS) $(TEST_DIR)/hello_world.mf

# Testar com exemplos básicos
test: $(TARGET)
	@echo -e "\n🧪 Testando hello_world.mf..."
	./$(TARGET) $(TEST_DIR)/hello_world.mf
	@echo -e "\n🧪 Testando variables.mf..."
	./$(TARGET) $(TEST_DIR)/variables.mf

# Executar todos os testes (script faz build se necessário)
test-all:
	@echo "========================================="
	@echo "EXECUTANDO TODOS OS TESTES"
	@echo "========================================="
	@chmod +x run_tests.sh
	@./run_tests.sh

# Executar testes e verificar sucesso
test-check:
	@echo "🔍 Verificando todos os testes..."
	@chmod +x run_tests.sh
	@if ./run_tests.sh | grep -q "❌ Testes falhados: 0"; then \
		echo "✅ Todos os testes passaram!"; \
		exit 0; \
	else \
		echo "❌ Alguns testes falharam!"; \
		exit 1; \
	fi

# Limpar arquivos gerados
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TOKENS)
	@echo "🧹 Arquivos limpos!"

# Debug: imprimir variáveis
debug:
	@echo "CC       = $(CC)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LEX      = $(LEX)"
	@echo "YACC     = $(YACC)"
	@echo "TARGET   = $(TARGET)"
	@echo "OBJS     = $(OBJS)"

# Ajuda
help:
	@echo "========================================="
	@echo "MAKEFILE - COMPILADOR DE LINGUAGEM MATEMÁTICA"
	@echo "========================================="
	@echo ""
	@echo "Comandos disponíveis:"
	@echo "  make all         - Compila o compilador e a ferramenta de tokens"
	@echo "  make rebuild     - Limpa e recompila tudo (clean + all)"
	@echo "  make tokens      - Compila test_tokens e analisa hello_world.mf"
	@echo "  make test        - Testa hello_world.mf e variables.mf"
	@echo "  make test-all    - Executa TODOS os testes (run_tests.sh)"
	@echo "  make test-check  - Verifica se todos os testes passam (exit code)"
	@echo "  make clean       - Remove arquivos compilados"
	@echo "  make debug       - Mostra variáveis de configuração"
	@echo "  make help        - Mostra esta mensagem"
	@echo ""
	@echo "Exemplos:"
	@echo "  make rebuild"
	@echo "  make test-all"
	@echo "  ./mathc tests/hello_world.mf"
	@echo ""

.PHONY: all rebuild test test-all test-check tokens clean debug help