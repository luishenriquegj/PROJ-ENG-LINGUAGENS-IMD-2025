#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern FILE* yyin;
extern int yyparse();
extern ASTNode* ast_root;

int main(int argc, char** argv) {
    FILE* input_file = NULL;
    int result = 0;

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
        fprintf(stderr, "USO: %s <arquivo.math>\n", argv[0]);
        return 1;
    }

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