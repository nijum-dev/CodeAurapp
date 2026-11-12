#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *source = malloc((size_t)length + 1);
    if (source == NULL) { fclose(file); return NULL; }
    size_t read = fread(source, 1, (size_t)length, file);
    source[read] = '\0';
    fclose(file);
    return source;
}

static void print_diagnostics(const DiagnosticArray *diagnostics) {
    for (size_t i = 0; i < diagnostics->count; i++)
        fprintf(stderr, "Line %d, column %d: %s\n",
                diagnostics->items[i].line, diagnostics->items[i].column,
                diagnostics->items[i].message);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.c>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *source = read_file(argv[1]);
    if (source == NULL) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }
    TokenArray tokens = {0};
    DiagnosticArray diagnostics = {0};
    lex_source(source, &tokens, &diagnostics);
    printf("=== TOKENS ===\n");
    for (size_t i = 0; i < tokens.count; i++)
        printf("%s\t%s\tline %d\n", token_type_name(tokens.items[i].type),
               tokens.items[i].lexeme, tokens.items[i].line);
    if (diagnostics.count == 0) {
        AstNode *ast = parse_program(&tokens, &diagnostics);
        if (diagnostics.count == 0) {
            printf("\n=== AST ===\n"); print_ast(ast, 0);
            analyze_semantics(ast, &diagnostics);
            if (diagnostics.count == 0) {
                printf("\n=== SYMBOLS ===\n"); print_symbols();
                printf("\n=== THREE-ADDRESS CODE ===\n"); generate_tac(ast);
            }
        }
        free_ast(ast);
    }
    bool success = diagnostics.count == 0;
    if (!success) {
        printf("\n=== DIAGNOSTICS ===\n");
        print_diagnostics(&diagnostics);
    }
    free_symbols(); free_diagnostics(&diagnostics); free_tokens(&tokens); free(source);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
