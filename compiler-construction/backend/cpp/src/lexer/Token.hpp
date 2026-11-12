#pragma once

#include <string>

namespace compiler_cpp {

enum class TokenType {
    EOF_TOKEN,
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,

    INT,
    FLOAT,
    BOOL,
    IF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    MAIN,
    COUT,
    ENDL,

    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    AND,
    OR,
    NOT,
    SHIFT,

    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,
    UNKNOWN
};

struct Token {
    TokenType type{TokenType::EOF_TOKEN};
    std::string lexeme;
    int line{1};
    int column{1};
};

std::string tokenTypeToString(TokenType type);

}  // namespace compiler_cpp
