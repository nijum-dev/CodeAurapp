#include "compiler.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { TYPE_UNKNOWN, TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STRING } ValueType;
typedef struct Symbol {
    char *name;
    ValueType type;
    int line;
    int depth;
    struct Symbol *next;
} Symbol;

static Symbol *symbols;
static int scope_depth;
static ValueType expression_type(const AstNode *node, DiagnosticArray *diagnostics);

static char *duplicate(const char *value) {
    char *copy = malloc(strlen(value) + 1);
    if (copy == NULL) { fprintf(stderr, "fatal: out of memory\n"); exit(EXIT_FAILURE); }
    strcpy(copy, value);
    return copy;
}

static const char *type_name(ValueType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        default: return "unknown";
    }
}

static ValueType declared_type(const char *value) {
    if (strcmp(value, "INT") == 0) return TYPE_INT;
    if (strcmp(value, "FLOAT") == 0) return TYPE_FLOAT;
    return TYPE_BOOL;
}

static Symbol *lookup(const char *name) {
    for (Symbol *symbol = symbols; symbol != NULL; symbol = symbol->next)
        if (symbol->depth <= scope_depth && strcmp(symbol->name, name) == 0)
            return symbol;
    return NULL;
}

static void declare_symbol(const AstNode *node, DiagnosticArray *diagnostics) {
    const char *name = node->children[0]->value;
    Symbol *existing = lookup(name);
    if (existing != NULL && existing->depth == scope_depth) {
        add_diagnostic(diagnostics, node->line, 1, "variable '%s' already declared", name);
        return;
    }
    Symbol *symbol = calloc(1, sizeof(*symbol));
    if (symbol == NULL) { fprintf(stderr, "fatal: out of memory\n"); exit(EXIT_FAILURE); }
    symbol->name = duplicate(name);
    symbol->type = declared_type(node->value);
    symbol->line = node->line;
    symbol->depth = scope_depth;
    symbol->next = symbols;
    symbols = symbol;
}

static bool compatible(ValueType target, ValueType source) {
    return target == source || (target == TYPE_FLOAT && source == TYPE_INT);
}

static ValueType literal_type(const char *value) {
    if (value[0] == '"') return TYPE_STRING;
    if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0) return TYPE_BOOL;
    return strchr(value, '.') == NULL ? TYPE_INT : TYPE_FLOAT;
}

static ValueType expression_type(const AstNode *node, DiagnosticArray *diagnostics) {
    if (node == NULL) return TYPE_UNKNOWN;
    if (node->kind == AST_LITERAL) return literal_type(node->value);
    if (node->kind == AST_IDENTIFIER) {
        Symbol *symbol = lookup(node->value);
        if (symbol == NULL) {
            add_diagnostic(diagnostics, node->line, 1, "undeclared variable '%s'", node->value);
            return TYPE_UNKNOWN;
        }
        return symbol->type;
    }
    if (node->kind == AST_UNARY) {
        ValueType operand = expression_type(node->children[0], diagnostics);
        if (strcmp(node->value, "!") == 0) {
            if (operand != TYPE_BOOL && operand != TYPE_UNKNOWN)
                add_diagnostic(diagnostics, node->line, 1, "operator ! requires a bool operand");
            return TYPE_BOOL;
        }
        if (operand != TYPE_INT && operand != TYPE_FLOAT && operand != TYPE_UNKNOWN)
            add_diagnostic(diagnostics, node->line, 1, "unary - requires a numeric operand");
        return operand;
    }
    if (node->kind == AST_BINARY) {
        ValueType left = expression_type(node->children[0], diagnostics);
        ValueType right = expression_type(node->children[1], diagnostics);
        const char *operator = node->value;
        if (strcmp(operator, "&&") == 0 || strcmp(operator, "||") == 0) {
            if ((left != TYPE_BOOL && left != TYPE_UNKNOWN) ||
                (right != TYPE_BOOL && right != TYPE_UNKNOWN))
                add_diagnostic(diagnostics, node->line, 1, "logical operator '%s' requires bool operands", operator);
            return TYPE_BOOL;
        }
        if (strcmp(operator, "==") == 0 || strcmp(operator, "!=") == 0) {
            if (left != TYPE_UNKNOWN && right != TYPE_UNKNOWN &&
                !compatible(left, right) && !compatible(right, left))
                add_diagnostic(diagnostics, node->line, 1, "incompatible equality operands");
            return TYPE_BOOL;
        }
        if (strchr("+-*/%", operator[0]) != NULL) {
            if ((left != TYPE_INT && left != TYPE_FLOAT && left != TYPE_UNKNOWN) ||
                (right != TYPE_INT && right != TYPE_FLOAT && right != TYPE_UNKNOWN))
                add_diagnostic(diagnostics, node->line, 1, "arithmetic operator '%s' requires numeric operands", operator);
            if (strcmp(operator, "%") == 0 && (left != TYPE_INT || right != TYPE_INT))
                add_diagnostic(diagnostics, node->line, 1, "modulo requires int operands");
            return left == TYPE_FLOAT || right == TYPE_FLOAT ? TYPE_FLOAT : TYPE_INT;
        }
        if (strchr("<>", operator[0]) != NULL) return TYPE_BOOL;
    }
    return TYPE_UNKNOWN;
}

static void analyze_node(const AstNode *node, DiagnosticArray *diagnostics) {
    if (node == NULL) return;
    switch (node->kind) {
        case AST_BLOCK:
            scope_depth++;
            for (size_t i = 0; i < node->child_count; i++) analyze_node(node->children[i], diagnostics);
            scope_depth--;
            break;
        case AST_DECLARATION:
            declare_symbol(node, diagnostics);
            if (node->child_count > 1) {
                ValueType source = expression_type(node->children[1], diagnostics);
                ValueType target = declared_type(node->value);
                if (source != TYPE_UNKNOWN && !compatible(target, source))
                    add_diagnostic(diagnostics, node->line, 1, "cannot initialize %s variable with %s value",
                                   type_name(target), type_name(source));
            }
            break;
        case AST_ASSIGNMENT: {
            Symbol *symbol = lookup(node->value);
            ValueType source = expression_type(node->children[0], diagnostics);
            if (symbol == NULL) add_diagnostic(diagnostics, node->line, 1, "undeclared variable '%s'", node->value);
            else if (source != TYPE_UNKNOWN && !compatible(symbol->type, source))
                add_diagnostic(diagnostics, node->line, 1, "cannot assign %s value to %s variable '%s'",
                               type_name(source), type_name(symbol->type), node->value);
            break;
        }
        case AST_IF:
        case AST_WHILE:
            if (expression_type(node->children[0], diagnostics) != TYPE_BOOL)
                add_diagnostic(diagnostics, node->line, 1, "condition must be bool");
            for (size_t i = 1; i < node->child_count; i++) analyze_node(node->children[i], diagnostics);
            break;
        case AST_FOR:
            analyze_node(node->children[0], diagnostics);
            if (node->children[1]->kind != AST_EMPTY &&
                expression_type(node->children[1], diagnostics) != TYPE_BOOL)
                add_diagnostic(diagnostics, node->line, 1, "for condition must be bool");
            analyze_node(node->children[2], diagnostics);
            analyze_node(node->children[3], diagnostics);
            break;
        case AST_PRINT:
            expression_type(node->children[0], diagnostics);
            break;
        default:
            for (size_t i = 0; i < node->child_count; i++) analyze_node(node->children[i], diagnostics);
            break;
    }
}

bool analyze_semantics(const AstNode *root, DiagnosticArray *diagnostics) {
    free_symbols();
    scope_depth = 0;
    analyze_node(root, diagnostics);
    return diagnostics->count == 0;
}

void print_symbols(void) {
    for (Symbol *symbol = symbols; symbol != NULL; symbol = symbol->next)
        printf("%s\t%s\tline %d\n", symbol->name, type_name(symbol->type), symbol->line);
}

void free_symbols(void) {
    while (symbols != NULL) {
        Symbol *next = symbols->next;
        free(symbols->name); free(symbols); symbols = next;
    }
}
