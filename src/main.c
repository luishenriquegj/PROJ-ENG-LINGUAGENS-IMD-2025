#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "codegen.h"
#include "../libs/symbol-table.h"

extern FILE* yyin;
extern int yyparse();
extern ASTNode* ast_root;
extern SymbolTable *symbol_table;

int main(int argc, char** argv) {
    FILE* input_file = NULL;
    int result = 0;
    const char* output_dir = NULL;

    // Processa argumentos da linha de comando
    if (argc >= 2 && strcmp(argv[1], "-o") == 0 && argc >= 4) {
        output_dir = argv[2];
        // Desloca argumentos
        argv[2] = argv[0]; // nome do programa
        argv = &argv[2];
        argc -= 2;
    }

    // Verifica variável de ambiente OUTPUT_DIR se não especificado via linha de comando
    if (!output_dir) {
        output_dir = getenv("MATHC_OUTPUT_DIR");
    }

    // Usa "build/tests" como padrão se não especificado
    if (!output_dir) {
        output_dir = "build/tests";
    }

    printf("============================================================================\n");
    printf("COMPILADOR - LINGUAGEM MATEMÁTICA\n");
    printf("UFRN - Engenharia de Linguagens\n");
    printf("============================================================================\n\n");

    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            fprintf(stderr, "ERRO: Não foi possível abrir o arquivo '%s'\n", argv[1]);
            return 1;
        }
        yyin = input_file;
        printf("Analisando arquivo: %s\n\n", argv[1]);
    } else {
        fprintf(stderr, "USO: %s [-o <output_dir>] <arquivo.math>\n", argv[0]);
        fprintf(stderr, "     -o <output_dir>: Diretório para arquivos de saída (padrão: build/tests)\n");
        fprintf(stderr, "     Alternativamente, defina MATHC_OUTPUT_DIR=<dir>\n");
        return 1;
    }

    // Registra funções built-in
    symbol_table = symbol_table_create();
    symbol_table_enter_scope(symbol_table);

    TypeSpec* t_void = create_type_spec(TYPE_VOID, NULL, NULL);
    TypeSpec* t_float = create_type_spec(TYPE_FLOAT, NULL, NULL);
    TypeSpec* t_int = create_type_spec(TYPE_INT, NULL, NULL);

    const char* builtins[] = {
        "print","println","input","range","len",
        // Matemática básica
        "abs","sqrt","exp","log","log10","log2",
        "sin","cos","tan","asin","acos","atan","atan2",
        "sinh","cosh","tanh",
        "floor","ceil","round","trunc",
        "pow","fmod","fabs",
        "max","min",
        "rand","srand",
        // Teoria dos números
        "factorial","gcd","lcm","isprime",
        "combinations","permutations",
        // Estatísticas de arrays
        "sum","mean","prod","std","variance",
        "sort","reverse","append","pop","push",
        "min_element","max_element","find","count",
        // Operações com matrizes
        "transpose","determinant","trace","matmul",
        "identity","zeros","ones",
        // Operações com conjuntos
        "union","intersection","difference","symmetric_diff",
        "is_subset","is_superset","cardinality",
        // Operações com complexos
        "real","imag","conjugate","phase","magnitude"
    };
    TypeSpec* types[] = {
        t_void,t_void,t_void,t_void,t_int,
        // Matemática básica
        t_float,t_float,t_float,t_float,t_float,t_float,
        t_float,t_float,t_float,t_float,t_float,t_float,t_float,
        t_float,t_float,t_float,
        t_float,t_float,t_float,t_float,
        t_float,t_float,t_float,
        t_float,t_float,
        t_int,t_void,
        // Teoria dos números
        t_int,t_int,t_int,t_int,
        t_int,t_int,
        // Estatísticas de arrays
        t_float,t_float,t_float,t_float,t_float,
        t_void,t_void,t_void,t_int,t_void,
        t_float,t_float,t_int,t_int,
        // Operações com matrizes
        t_void,t_float,t_float,t_void,
        t_void,t_void,t_void,
        // Operações com conjuntos
        t_void,t_void,t_void,t_void,
        t_int,t_int,t_int,
        // Operações com complexos
        t_float,t_float,t_void,t_float,t_float
    };
    int count = sizeof(builtins)/sizeof(builtins[0]);
    for (int i=0;i<count;i++) {
        if (!symbol_table_lookup(symbol_table, builtins[i])) {
            symbol_table_insert(symbol_table, builtins[i], types[i], SYM_FUNC, 0, NULL);
        }
    }
    // Removido: constantes nativas como variáveis para evitar conflito com macros no C

    printf("Iniciando análise sintática...\n");
    result = yyparse();

    if (result == 0) {
        printf("\n✓ Análise sintática concluída com SUCESSO!\n\n");

        if (ast_root) {
            printf("============================================================================\n");
            printf("ÁRVORE SINTÁTICA ABSTRATA (AST)\n");
            printf("============================================================================\n\n");

            int node_count = count_ast_nodes(ast_root);
            printf("Total de nós na AST: %d\n\n", node_count);

            print_ast(ast_root, 0);

            printf("\n============================================================================\n");

            // Gera código C no diretório de saída especificado
            char nome_base[256];
            char nome_arquivo_saida[1024];
            char nome_executavel[1024];
            char comando_mkdir[512];

            // Extrai apenas o nome do arquivo sem o caminho
            const char* caminho_entrada = argv[1];
            const char* ultima_barra = strrchr(caminho_entrada, '/');
            const char* nome_arquivo = ultima_barra ? ultima_barra + 1 : caminho_entrada;

            // Remove extensão .mf
            strcpy(nome_base, nome_arquivo);
            char* ponto = strrchr(nome_base, '.');
            if (ponto && strcmp(ponto, ".mf") == 0) {
                *ponto = '\0';
            }

            // Cria caminhos de saída no diretório especificado
            snprintf(nome_arquivo_saida, sizeof(nome_arquivo_saida), "%s/%s.c", output_dir, nome_base);
            snprintf(nome_executavel, sizeof(nome_executavel), "%s/%s.out", output_dir, nome_base);

            // Garante que o diretório de saída existe
            snprintf(comando_mkdir, sizeof(comando_mkdir), "mkdir -p %s", output_dir);
            system(comando_mkdir);

            FILE* arquivo_saida = fopen(nome_arquivo_saida, "w");
            if (arquivo_saida) {
                printf("\nGerando código C em: %s\n", nome_arquivo_saida);

                CodeGenContext* contexto_codegen = codegen_create(arquivo_saida);
                codegen_program(contexto_codegen, ast_root);
                codegen_free(contexto_codegen);

                fclose(arquivo_saida);
                printf("✓ Código C gerado com sucesso!\n");

                // Compila o código C gerado
                char comando_compilacao[4096];
                snprintf(comando_compilacao, sizeof(comando_compilacao),
                        "gcc -o %s %s -lm 2>&1", nome_executavel, nome_arquivo_saida);

                printf("\nCompilando código C...\n");
                int resultado_compilacao = system(comando_compilacao);

                if (resultado_compilacao == 0) {
                    printf("✓ Compilação concluída com sucesso!\n");
                    printf("Executável gerado: %s\n", nome_executavel);
                } else {
                    printf("✗ Erro na compilação do código C\n");
                }
            } else {
                fprintf(stderr, "Erro ao criar arquivo de saída: %s\n", nome_arquivo_saida);
            }

            printf("\nLiberando memória da AST...\n");
            free_ast_tree(&ast_root);
            printf("Memória liberada com sucesso!\n");
        }
    } else {
        printf("\n✗ Análise sintática FALHOU!\n");
        if (ast_root) {
            free_ast_tree(&ast_root);
        }
    }

    if (input_file) {
        fclose(input_file);
    }

#ifdef DEBUG_MEMORY
    print_memory_stats();
#endif

    return result;
}