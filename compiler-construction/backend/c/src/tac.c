#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>

static int temporary;
static int label;

static void expression(const AstNode *node, char *buffer, size_t size) {
    if (node->kind == AST_LITERAL || node->kind == AST_IDENTIFIER) {
        snprintf(buffer, size, "%s", node->value);
    } else if (node->kind == AST_UNARY) {
        char operand[128]; expression(node->children[0], operand, sizeof(operand));
        snprintf(buffer, size, "t%d", ++temporary);
        printf("%s = %s%s\n", buffer, node->value, operand);
    } else if (node->kind == AST_BINARY) {
        char left[128], right[128]; expression(node->children[0], left, sizeof(left));
        expression(node->children[1], right, sizeof(right));
        snprintf(buffer, size, "t%d", ++temporary);
        printf("%s = %s %s %s\n", buffer, left, node->value, right);
    } else {
        if (size > 0) buffer[0] = '\0';
    }
}

static void node(const AstNode *ast) {
    if (ast == NULL) return;
    if (ast->kind == AST_PROGRAM || ast->kind == AST_BLOCK) {
        for (size_t i = 0; i < ast->child_count; i++) node(ast->children[i]);
    } else if (ast->kind == AST_DECLARATION && ast->child_count > 1) {
        char value[128]; expression(ast->children[1], value, sizeof(value));
        printf("%s = %s\n", ast->children[0]->value, value);
    } else if (ast->kind == AST_ASSIGNMENT) {
        char value[128]; expression(ast->children[0], value, sizeof(value));
        printf("%s = %s\n", ast->value, value);
    } else if (ast->kind == AST_PRINT) {
        char value[128]; expression(ast->children[0], value, sizeof(value));
        printf("print %s\n", value);
    } else if (ast->kind == AST_IF) {
        char condition[128]; expression(ast->children[0], condition, sizeof(condition));
        int else_label = ++label, end_label = ++label;
        printf("ifFalse %s goto L%d\n", condition, else_label);
        node(ast->children[1]);
        if (ast->child_count > 2) {
            printf("goto L%d\nL%d:\n", end_label, else_label);
            node(ast->children[2]); printf("L%d:\n", end_label);
        } else printf("L%d:\n", else_label);
    } else if (ast->kind == AST_WHILE) {
        int start = ++label, end = ++label;
        printf("L%d:\n", start);
        char condition[128]; expression(ast->children[0], condition, sizeof(condition));
        printf("ifFalse %s goto L%d\n", condition, end);
        node(ast->children[1]); printf("goto L%d\nL%d:\n", start, end);
    } else if (ast->kind == AST_FOR) {
        node(ast->children[0]); int start = ++label, end = ++label;
        printf("L%d:\n", start);
        if (ast->children[1]->kind != AST_EMPTY) {
            char condition[128]; expression(ast->children[1], condition, sizeof(condition));
            printf("ifFalse %s goto L%d\n", condition, end);
        }
        node(ast->children[3]); node(ast->children[2]);
        printf("goto L%d\nL%d:\n", start, end);
    }
}

void generate_tac(const AstNode *root) {
    temporary = 0; label = 0; node(root);
}
