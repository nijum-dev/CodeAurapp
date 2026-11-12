#include "compiler.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const TokenArray *tokens;
    size_t current;
    DiagnosticArray *diagnostics;
} Parser;

static const Token *peek(Parser *p) { return &p->tokens->items[p->current]; }
static const Token *previous(Parser *p) { return &p->tokens->items[p->current - 1]; }
static bool check(Parser *p, TokenType type) { return peek(p)->type == type; }
static bool at_end(Parser *p) { return check(p, TOKEN_EOF); }
static const Token *advance(Parser *p) { if (!at_end(p)) p->current++; return previous(p); }
static bool match(Parser *p, TokenType type) { if (!check(p, type)) return false; advance(p); return true; }

static bool expect(Parser *p, TokenType type, const char *message) {
    if (match(p, type)) return true;
    add_diagnostic(p->diagnostics, peek(p)->line, peek(p)->column,
                   "%s; found %s", message, token_type_name(peek(p)->type));
    return false;
}

static AstNode *expression(Parser *p);

static bool is_type(TokenType type) {
    return type == TOKEN_INT || type == TOKEN_FLOAT || type == TOKEN_BOOL;
}

static AstNode *block(Parser *p);

static AstNode *declaration(Parser *p, bool consume_semicolon) {
    const Token *type = advance(p);
    if (!expect(p, TOKEN_IDENTIFIER, "expected variable name")) return NULL;
    const Token *name = previous(p);
    AstNode *node = ast_node(AST_DECLARATION, token_type_name(type->type), type->line);
    ast_add_child(node, ast_node(AST_IDENTIFIER, name->lexeme, name->line));
    if (match(p, TOKEN_ASSIGN)) ast_add_child(node, expression(p));
    if (consume_semicolon) expect(p, TOKEN_SEMICOLON, "expected ';' after declaration");
    return node;
}

static AstNode *assignment(Parser *p, bool consume_semicolon) {
    const Token *name = advance(p);
    expect(p, TOKEN_ASSIGN, "expected '='");
    AstNode *node = ast_node(AST_ASSIGNMENT, name->lexeme, name->line);
    ast_add_child(node, expression(p));
    if (consume_semicolon) expect(p, TOKEN_SEMICOLON, "expected ';' after assignment");
    return node;
}

static AstNode *increment(Parser *p) {
    const Token *name = advance(p);
    const Token *operator = advance(p);
    AstNode *node = ast_node(AST_ASSIGNMENT, name->lexeme, name->line);
    AstNode *binary = ast_node(AST_BINARY,
                               operator->type == TOKEN_INCREMENT ? "+" : "-",
                               operator->line);
    ast_add_child(binary, ast_node(AST_IDENTIFIER, name->lexeme, name->line));
    ast_add_child(binary, ast_node(AST_LITERAL, "1", operator->line));
    ast_add_child(node, binary);
    return node;
}

static AstNode *statement(Parser *p);

static AstNode *print_statement(Parser *p) {
    const Token *token = advance(p);
    expect(p, TOKEN_LPAREN, "expected '(' after printf");
    AstNode *node = ast_node(AST_PRINT, NULL, token->line);
    ast_add_child(node, expression(p));
    expect(p, TOKEN_RPAREN, "expected ')' after printf argument");
    expect(p, TOKEN_SEMICOLON, "expected ';' after printf");
    return node;
}

static AstNode *if_statement(Parser *p) {
    const Token *token = advance(p);
    expect(p, TOKEN_LPAREN, "expected '(' after if");
    AstNode *node = ast_node(AST_IF, NULL, token->line);
    ast_add_child(node, expression(p));
    expect(p, TOKEN_RPAREN, "expected ')' after condition");
    ast_add_child(node, statement(p));
    if (match(p, TOKEN_ELSE)) ast_add_child(node, statement(p));
    return node;
}

static AstNode *while_statement(Parser *p) {
    const Token *token = advance(p);
    expect(p, TOKEN_LPAREN, "expected '(' after while");
    AstNode *node = ast_node(AST_WHILE, NULL, token->line);
    ast_add_child(node, expression(p));
    expect(p, TOKEN_RPAREN, "expected ')' after condition");
    ast_add_child(node, statement(p));
    return node;
}

static AstNode *for_statement(Parser *p) {
    const Token *token = advance(p);
    expect(p, TOKEN_LPAREN, "expected '(' after for");
    AstNode *node = ast_node(AST_FOR, NULL, token->line);
    if (match(p, TOKEN_SEMICOLON)) ast_add_child(node, ast_node(AST_EMPTY, NULL, token->line));
    else if (is_type(peek(p)->type)) {
        ast_add_child(node, declaration(p, false));
        expect(p, TOKEN_SEMICOLON, "expected ';' after for initializer");
    } else {
        if (!check(p, TOKEN_IDENTIFIER)) add_diagnostic(p->diagnostics, peek(p)->line, peek(p)->column, "expected for initializer");
        ast_add_child(node, assignment(p, false));
        expect(p, TOKEN_SEMICOLON, "expected ';' after for initializer");
    }
    if (check(p, TOKEN_SEMICOLON)) ast_add_child(node, ast_node(AST_EMPTY, NULL, peek(p)->line));
    else ast_add_child(node, expression(p));
    expect(p, TOKEN_SEMICOLON, "expected ';' after for condition");
    if (check(p, TOKEN_RPAREN)) ast_add_child(node, ast_node(AST_EMPTY, NULL, peek(p)->line));
    else if (check(p, TOKEN_IDENTIFIER)) {
        if (p->tokens->items[p->current + 1].type == TOKEN_INCREMENT ||
            p->tokens->items[p->current + 1].type == TOKEN_DECREMENT)
            ast_add_child(node, increment(p));
        else
            ast_add_child(node, assignment(p, false));
    } else {
        add_diagnostic(p->diagnostics, peek(p)->line, peek(p)->column, "expected for update");
        ast_add_child(node, ast_node(AST_EMPTY, NULL, peek(p)->line));
    }
    expect(p, TOKEN_RPAREN, "expected ')' after for clauses");
    ast_add_child(node, statement(p));
    return node;
}

static AstNode *statement(Parser *p) {
    if (is_type(peek(p)->type)) return declaration(p, true);
    if (match(p, TOKEN_PRINTF)) { p->current--; return print_statement(p); }
    if (check(p, TOKEN_IF)) return if_statement(p);
    if (check(p, TOKEN_WHILE)) return while_statement(p);
    if (check(p, TOKEN_FOR)) return for_statement(p);
    if (match(p, TOKEN_LBRACE)) return block(p);
    if (check(p, TOKEN_IDENTIFIER)) return assignment(p, true);
    add_diagnostic(p->diagnostics, peek(p)->line, peek(p)->column,
                   "expected statement; found %s", token_type_name(peek(p)->type));
    advance(p);
    return ast_node(AST_EMPTY, NULL, previous(p)->line);
}

static AstNode *block(Parser *p) {
    AstNode *node = ast_node(AST_BLOCK, NULL, previous(p)->line);
    while (!check(p, TOKEN_RBRACE) && !at_end(p)) ast_add_child(node, statement(p));
    expect(p, TOKEN_RBRACE, "expected '}' after block");
    return node;
}

static AstNode *primary(Parser *p) {
    const Token *token = peek(p);
    if (match(p, TOKEN_IDENTIFIER)) return ast_node(AST_IDENTIFIER, token->lexeme, token->line);
    if (match(p, TOKEN_INT_LITERAL) || match(p, TOKEN_FLOAT_LITERAL) ||
        match(p, TOKEN_STRING_LITERAL) || match(p, TOKEN_BOOL_LITERAL)) {
        return ast_node(AST_LITERAL, token->lexeme, token->line);
    }
    if (match(p, TOKEN_LPAREN)) {
        AstNode *node = expression(p);
        expect(p, TOKEN_RPAREN, "expected ')' after expression");
        return node;
    }
    add_diagnostic(p->diagnostics, token->line, token->column, "expected expression");
    advance(p);
    return ast_node(AST_EMPTY, NULL, token->line);
}

static AstNode *unary(Parser *p) {
    if (match(p, TOKEN_NOT) || match(p, TOKEN_MINUS)) {
        const Token *operator = previous(p);
        AstNode *node = ast_node(AST_UNARY, operator->lexeme, operator->line);
        ast_add_child(node, unary(p));
        return node;
    }
    return primary(p);
}

static AstNode *binary(Parser *p, AstNode *(*next)(Parser *), TokenType first, TokenType second) {
    AstNode *left = next(p);
    while (check(p, first) || check(p, second)) {
        const Token *operator = advance(p);
        AstNode *node = ast_node(AST_BINARY, operator->lexeme, operator->line);
        ast_add_child(node, left); ast_add_child(node, next(p)); left = node;
    }
    return left;
}

static AstNode *factor(Parser *p) {
    AstNode *left = unary(p);
    while (check(p, TOKEN_STAR) || check(p, TOKEN_SLASH) || check(p, TOKEN_PERCENT)) {
        const Token *operator = advance(p);
        AstNode *node = ast_node(AST_BINARY, operator->lexeme, operator->line);
        ast_add_child(node, left);
        ast_add_child(node, unary(p));
        left = node;
    }
    return left;
}
static AstNode *term(Parser *p) { return binary(p, factor, TOKEN_PLUS, TOKEN_MINUS); }
static AstNode *comparison(Parser *p) {
    AstNode *left = term(p);
    while (check(p, TOKEN_LESS) || check(p, TOKEN_LESS_EQUAL) ||
           check(p, TOKEN_GREATER) || check(p, TOKEN_GREATER_EQUAL)) {
        const Token *operator = advance(p);
        AstNode *node = ast_node(AST_BINARY, operator->lexeme, operator->line);
        ast_add_child(node, left); ast_add_child(node, term(p)); left = node;
    }
    return left;
}
static AstNode *equality(Parser *p) {
    AstNode *left = comparison(p);
    while (check(p, TOKEN_EQUAL) || check(p, TOKEN_NOT_EQUAL)) {
        const Token *operator = advance(p);
        AstNode *node = ast_node(AST_BINARY, operator->lexeme, operator->line);
        ast_add_child(node, left); ast_add_child(node, comparison(p)); left = node;
    }
    return left;
}
static AstNode *logical_and(Parser *p) {
    AstNode *left = equality(p);
    while (match(p, TOKEN_AND)) {
        const Token *operator = previous(p);
        AstNode *node = ast_node(AST_BINARY, operator->lexeme, operator->line);
        ast_add_child(node, left); ast_add_child(node, equality(p)); left = node;
    }
    return left;
}
static AstNode *logical_or(Parser *p) {
    AstNode *left = logical_and(p);
    while (match(p, TOKEN_OR)) {
        const Token *operator = previous(p);
        AstNode *node = ast_node(AST_BINARY, operator->lexeme, operator->line);
        ast_add_child(node, left); ast_add_child(node, logical_and(p)); left = node;
    }
    return left;
}
static AstNode *expression(Parser *p) { return logical_or(p); }

AstNode *parse_program(const TokenArray *tokens, DiagnosticArray *diagnostics) {
    Parser parser = {tokens, 0, diagnostics};
    AstNode *program = ast_node(AST_PROGRAM, NULL, 1);
    while (!at_end(&parser)) ast_add_child(program, statement(&parser));
    return program;
}
