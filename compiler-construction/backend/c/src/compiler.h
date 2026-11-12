#ifndef C_COMPILER_H
#define C_COMPILER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_PRINTF,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_PRINT,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BINARY,
    AST_UNARY,
    AST_IDENTIFIER,
    AST_LITERAL,
    AST_EMPTY
} AstKind;

typedef struct AstNode {
    AstKind kind;
    char *value;
    int line;
    struct AstNode **children;
    size_t child_count;
    size_t child_capacity;
} AstNode;

typedef struct {
    AstNode **items;
    size_t count;
    size_t capacity;
} AstNodeArray;

typedef struct {
    char *message;
    int line;
    int column;
} Diagnostic;

typedef struct {
    Diagnostic *items;
    size_t count;
    size_t capacity;
} DiagnosticArray;

AstNode *ast_node(AstKind kind, const char *value, int line);
void ast_add_child(AstNode *parent, AstNode *child);

bool lex_source(const char *source, TokenArray *tokens, DiagnosticArray *diagnostics);
void free_tokens(TokenArray *tokens);
const char *token_type_name(TokenType type);

AstNode *parse_program(const TokenArray *tokens, DiagnosticArray *diagnostics);
void free_ast(AstNode *node);
const char *ast_kind_name(AstKind kind);
void print_ast(const AstNode *node, int depth);

bool analyze_semantics(const AstNode *root, DiagnosticArray *diagnostics);
void print_symbols(void);
void free_symbols(void);

void generate_tac(const AstNode *root);

void add_diagnostic(DiagnosticArray *diagnostics, int line, int column,
                   const char *format, ...);
void free_diagnostics(DiagnosticArray *diagnostics);

#endif
