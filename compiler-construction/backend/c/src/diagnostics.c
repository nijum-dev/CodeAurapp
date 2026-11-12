#include "compiler.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void add_diagnostic(DiagnosticArray *diagnostics, int line, int column,
                    const char *format, ...) {
    va_list args;
    va_list copy;
    va_start(args, format);
    va_copy(copy, args);
    int length = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    Diagnostic diagnostic = {0};
    diagnostic.message = malloc((size_t)length + 1);
    if (diagnostic.message == NULL) {
        va_end(args);
        fprintf(stderr, "fatal: out of memory\n");
        exit(EXIT_FAILURE);
    }
    vsnprintf(diagnostic.message, (size_t)length + 1, format, args);
    va_end(args);
    diagnostic.line = line;
    diagnostic.column = column;

    if (diagnostics->count == diagnostics->capacity) {
        size_t capacity = diagnostics->capacity == 0 ? 8 : diagnostics->capacity * 2;
        Diagnostic *items = realloc(diagnostics->items, capacity * sizeof(*items));
        if (items == NULL) {
            free(diagnostic.message);
            fprintf(stderr, "fatal: out of memory\n");
            exit(EXIT_FAILURE);
        }
        diagnostics->items = items;
        diagnostics->capacity = capacity;
    }
    diagnostics->items[diagnostics->count++] = diagnostic;
}

void free_diagnostics(DiagnosticArray *diagnostics) {
    for (size_t i = 0; i < diagnostics->count; i++) {
        free(diagnostics->items[i].message);
    }
    free(diagnostics->items);
    diagnostics->items = NULL;
    diagnostics->count = 0;
    diagnostics->capacity = 0;
}
