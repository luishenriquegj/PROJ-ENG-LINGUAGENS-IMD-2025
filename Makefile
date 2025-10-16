# ============================================================================
# MAKEFILE - ANALISADOR LÉXICO DA LINGUAGEM MATEMÁTICA
# ============================================================================

# Compilador C
CC = gcc

# Flags de compilação
CFLAGS = -Wall -g

# Nome do executável
TARGET = lexer

# Arquivo fonte do Flex
LEX_SOURCE = lexer.l

# Arquivo C gerado pelo Flex
LEX_GENERATED = lex.yy.c

# Diretório de testes
TEST_DIR = tests

# ============================================================================
# ALVOS PRINCIPAIS
# ============================================================================

# Alvo padrão: compila tudo
all: $(TARGET)

# Compila o analisador léxico
$(TARGET): $(LEX_GENERATED)
	@echo "🔨 Compilando o analisador léxico..."
	$(CC) $(CFLAGS) $(LEX_GENERATED) -o $(TARGET)
	@echo "✅ Analisador léxico compilado com sucesso!"
	@echo "   Execute com: ./$(TARGET) <arquivo.mf>"

# Gera o código C a partir do arquivo Flex
$(LEX_GENERATED): $(LEX_SOURCE)
	@echo "⚙️  Gerando código C com Flex..."
	flex $(LEX_SOURCE)

# ============================================================================
# TESTES
# ============================================================================

# Executa todos os testes
test: $(TARGET)
	@echo "🧪 Executando testes..."
	@echo ""
	@for file in $(TEST_DIR)/*.mf; do \
		echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"; \
		echo "📝 Testando: $file"; \
		echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"; \
		./$(TARGET) $file; \
		echo ""; \
	done
	@echo "✅ Todos os testes executados!"

# Teste individual
test-one: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "❌ Erro: Especifique o arquivo com FILE=<arquivo>"; \
		echo "   Exemplo: make test-one FILE=tests/exemplo1.mf"; \
		exit 1; \
	fi
	@echo "🧪 Testando arquivo único: $(FILE)"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./$(TARGET) $(FILE)

# Cria diretório de testes se não existir
$(TEST_DIR):
	@mkdir -p $(TEST_DIR)

# Gera arquivos de teste de exemplo
test-files: $(TEST_DIR)
	@echo "📝 Criando arquivos de teste..."
	@echo "// Teste básico de tipos" > $(TEST_DIR)/test_tipos.mf
	@echo "int x = 42" >> $(TEST_DIR)/test_tipos.mf
	@echo "float y = 3.14" >> $(TEST_DIR)/test_tipos.mf
	@echo "complex z = 3+4i" >> $(TEST_DIR)/test_tipos.mf
	@echo "bool flag = true" >> $(TEST_DIR)/test_tipos.mf
	@echo ""
	@echo "// Teste de operadores" > $(TEST_DIR)/test_operadores.mf
	@echo "int soma = a + b" >> $(TEST_DIR)/test_operadores.mf
	@echo "float div = x / y" >> $(TEST_DIR)/test_operadores.mf
	@echo "int potencia = 2 ^ 10" >> $(TEST_DIR)/test_operadores.mf
	@echo "matrix[float] produto = A @ B" >> $(TEST_DIR)/test_operadores.mf
	@echo ""
	@echo "// Teste de função" > $(TEST_DIR)/test_funcao.mf
	@echo "def fun int soma(int a, int b)" >> $(TEST_DIR)/test_funcao.mf
	@echo "    return a + b" >> $(TEST_DIR)/test_funcao.mf
	@echo "end def" >> $(TEST_DIR)/test_funcao.mf
	@echo "✅ Arquivos de teste criados em $(TEST_DIR)/"

# ============================================================================
# LIMPEZA
# ============================================================================

# Remove arquivos gerados
clean:
	@echo "🧹 Limpando arquivos gerados..."
	rm -f $(LEX_GENERATED) $(TARGET)
	@echo "✅ Limpeza concluída!"

# Remove tudo, incluindo testes
clean-all: clean
	@echo "🧹 Removendo diretório de testes..."
	rm -rf $(TEST_DIR)
	@echo "✅ Limpeza completa!"

# ============================================================================
# UTILIDADES
# ============================================================================

# Mostra ajuda
help:
	@echo "═══════════════════════════════════════════════════════════════"
	@echo "  ANALISADOR LÉXICO - LINGUAGEM MATEMÁTICA"
	@echo "═══════════════════════════════════════════════════════════════"
	@echo ""
	@echo "📋 Comandos disponíveis:"
	@echo ""
	@echo "  make              - Compila o analisador léxico"
	@echo "  make all          - Mesmo que 'make'"
	@echo "  make test         - Executa todos os testes"
	@echo "  make test-one     - Testa um arquivo específico"
	@echo "                      Uso: make test-one FILE=arquivo.mf"
	@echo "  make test-files   - Cria arquivos de teste de exemplo"
	@echo "  make clean        - Remove arquivos gerados"
	@echo "  make clean-all    - Remove tudo (incluindo testes)"
	@echo "  make run          - Executa o lexer (entrada padrão)"
	@echo "  make help         - Mostra esta ajuda"
	@echo ""
	@echo "📝 Exemplos de uso:"
	@echo ""
	@echo "  # Compilar"
	@echo "  make"
	@echo ""
	@echo "  # Testar com arquivo"
	@echo "  ./lexer programa.mf"
	@echo ""
	@echo "  # Testar com entrada padrão"
	@echo "  echo 'int x = 42' | ./lexer"
	@echo ""
	@echo "  # Executar todos os testes"
	@echo "  make test"
	@echo ""
	@echo "═══════════════════════════════════════════════════════════════"

# Executa o lexer com entrada padrão
run: $(TARGET)
	@echo "⌨️  Digite o código (Ctrl+D para terminar):"
	./$(TARGET)

# Verifica se o Flex está instalado
check-flex:
	@which flex > /dev/null || (echo "❌ Erro: Flex não está instalado!" && \
		echo "   Ubuntu/Debian: sudo apt-get install flex" && \
		echo "   macOS: brew install flex" && exit 1)
	@echo "✅ Flex está instalado: $(flex --version)"

# Instala dependências (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "📦 Instalando dependências..."
	sudo apt-get update
	sudo apt-get install -y flex gcc make
	@echo "✅ Dependências instaladas!"

install-deps-arch:
	@echo "📦 Instalando dependências..."
	sudo pacman -Sy --noconfirm flex gcc make
	@echo "✅ Dependências instaladas!"
# ============================================================================
# PHONY TARGETS (não são arquivos)
# ============================================================================

.PHONY: all test test-one test-files clean clean-all help run check-flex install-deps