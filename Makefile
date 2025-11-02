CC = gcc
CFLAGS = -Wall -Wno-unused-function -g -D_GNU_SOURCE
LEX = flex
YACC = bison

SRC = src
BUILD = build
TESTS = tests

TARGET = mathc
LEXTEST = test_lexer

YACC_SRC = $(SRC)/parser.y
LEX_SRC = $(SRC)/lexer.l
YACC_OUT = $(BUILD)/parser.tab.c
YACC_HDR = $(BUILD)/parser.tab.h
LEX_OUT = $(BUILD)/lex.yy.c

OBJS = $(BUILD)/lex.yy.o $(BUILD)/parser.tab.o $(BUILD)/ast.o $(BUILD)/main.o

.PHONY: all clean test lexer-test debug help

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(YACC_OUT) $(YACC_HDR): $(YACC_SRC) | $(BUILD)
	$(YACC) -d -o $(YACC_OUT) $<

$(LEX_OUT): $(LEX_SRC) $(YACC_HDR) | $(BUILD)
	$(LEX) -o $@ $<

$(BUILD)/lex.yy.o: $(LEX_OUT)
	$(CC) $(CFLAGS) -I$(BUILD) -c -o $@ $<

$(BUILD)/parser.tab.o: $(YACC_OUT)
	$(CC) $(CFLAGS) -I$(BUILD) -I$(SRC) -c -o $@ $<

$(BUILD)/ast.o: $(SRC)/ast.c $(SRC)/ast.h $(YACC_HDR)
	$(CC) $(CFLAGS) -I$(BUILD) -I$(SRC) -c -o $@ $<

$(BUILD)/main.o: $(SRC)/main.c $(SRC)/ast.h $(YACC_HDR)
	$(CC) $(CFLAGS) -I$(BUILD) -I$(SRC) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(LEXTEST): test_lexer.c $(BUILD)/lex.yy.o $(BUILD)/parser.tab.o $(BUILD)/ast.o | $(BUILD)
	$(CC) $(CFLAGS) -I$(BUILD) -I$(SRC) -o $@ $^

clean:
	rm -rf $(BUILD) $(TARGET) $(LEXTEST)

test: $(TARGET)
	@for f in $(TESTS)/*.mf; do \
		echo ""; \
		echo "========================================"; \
		echo "Testando: $$f"; \
		echo "========================================"; \
		./$(TARGET) $$f || true; \
	done

lexer-test: $(LEXTEST)
	./$(LEXTEST) $(TESTS)/hello_world.mf

debug: CFLAGS += -DDEBUG
debug: clean all

help:
	@echo "Comandos disponíveis:"
	@echo "  make all        - Compila o compilador"
	@echo "  make clean      - Remove arquivos gerados"
	@echo "  make test       - Executa suite de testes"
	@echo "  make lexer-test - Testa o analisador léxico"
	@echo "  make debug      - Compila com debug"
	@echo "  make help       - Mostra esta mensagem"