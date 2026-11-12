#include "Lexer.hpp"

#include <cctype>
#include <sstream>
#include <utility>

namespace compiler_cpp {

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source_[index_];
}

char Lexer::advance() {
    if (isAtEnd()) {
        return '\0';
    }
    char current = source_[index_++];
    if (current == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return current;
}

bool Lexer::isAtEnd() const {
    return index_ >= source_.size();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skipComment() {
    if (peek() == '/' && index_ + 1 < source_.size() && source_[index_ + 1] == '/') {
        advance();
        advance();
        while (!isAtEnd() && peek() != '\n') {
            advance();
        }
        return;
    }

    if (peek() == '/' && index_ + 1 < source_.size() && source_[index_ + 1] == '*') {
        advance();
        advance();
        while (!isAtEnd()) {
            if (peek() == '*' && index_ + 1 < source_.size() && source_[index_ + 1] == '/') {
                advance();
                advance();
                return;
            }
            advance();
        }
        errors_.push_back("Unterminated block comment at line " + std::to_string(line_));
        return;
    }
}

Token Lexer::readIdentifierOrKeyword() {
    const int startLine = line_;
    const int startColumn = column_;
    std::string value;

    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        value.push_back(advance());
    }

    if (value == "int") return {TokenType::INT, value, startLine, startColumn};
    if (value == "float") return {TokenType::FLOAT, value, startLine, startColumn};
    if (value == "bool") return {TokenType::BOOL, value, startLine, startColumn};
    if (value == "if") return {TokenType::IF, value, startLine, startColumn};
    if (value == "else") return {TokenType::ELSE, value, startLine, startColumn};
    if (value == "while") return {TokenType::WHILE, value, startLine, startColumn};
    if (value == "for") return {TokenType::FOR, value, startLine, startColumn};
    if (value == "return") return {TokenType::RETURN, value, startLine, startColumn};
    if (value == "main") return {TokenType::MAIN, value, startLine, startColumn};
    if (value == "cout") return {TokenType::COUT, value, startLine, startColumn};
    if (value == "endl") return {TokenType::ENDL, value, startLine, startColumn};
    if (value == "true" || value == "false") return {TokenType::BOOLEAN_LITERAL, value, startLine, startColumn};
    return {TokenType::IDENTIFIER, value, startLine, startColumn};
}

Token Lexer::readNumber() {
    const int startLine = line_;
    const int startColumn = column_;
    std::string value;

    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
        value.push_back(advance());
    }

    if (!isAtEnd() && peek() == '.') {
        value.push_back(advance());
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            value.push_back(advance());
        }
        return {TokenType::FLOAT_LITERAL, value, startLine, startColumn};
    }

    return {TokenType::INTEGER_LITERAL, value, startLine, startColumn};
}

Token Lexer::readString() {
    const int startLine = line_;
    const int startColumn = column_;
    std::string value;

    advance();
    while (!isAtEnd()) {
        char c = advance();
        if (c == '"') {
            return {TokenType::STRING_LITERAL, value, startLine, startColumn};
        }
        if (c == '\\' && !isAtEnd()) {
            char escaped = advance();
            if (escaped == 'n') value.push_back('\n');
            else if (escaped == 't') value.push_back('\t');
            else if (escaped == '"') value.push_back('"');
            else if (escaped == '\\') value.push_back('\\');
            else value.push_back(escaped);
            continue;
        }
        value.push_back(c);
    }

    errors_.push_back("Unterminated string literal at line " + std::to_string(startLine));
    return {TokenType::UNKNOWN, value, startLine, startColumn};
}

Token Lexer::readOperator() {
    const int startLine = line_;
    const int startColumn = column_;
    const std::string remaining = source_.substr(index_);

    if (remaining.rfind("&&", 0) == 0) {
        advance();
        advance();
        return {TokenType::AND, "&&", startLine, startColumn};
    }
    if (remaining.rfind("||", 0) == 0) {
        advance();
        advance();
        return {TokenType::OR, "||", startLine, startColumn};
    }
    if (remaining.rfind("==", 0) == 0) {
        advance();
        advance();
        return {TokenType::EQUAL, "==", startLine, startColumn};
    }
    if (remaining.rfind("!=", 0) == 0) {
        advance();
        advance();
        return {TokenType::NOT_EQUAL, "!=", startLine, startColumn};
    }
    if (remaining.rfind("<=", 0) == 0) {
        advance();
        advance();
        return {TokenType::LESS_EQUAL, "<=", startLine, startColumn};
    }
    if (remaining.rfind(">=", 0) == 0) {
        advance();
        advance();
        return {TokenType::GREATER_EQUAL, ">=", startLine, startColumn};
    }
    if (remaining.rfind("<<", 0) == 0) {
        advance();
        advance();
        return {TokenType::SHIFT, "<<", startLine, startColumn};
    }

    const char op = advance();
    switch (op) {
        case '+': return {TokenType::PLUS, "+", startLine, startColumn};
        case '-': return {TokenType::MINUS, "-", startLine, startColumn};
        case '*': return {TokenType::MULTIPLY, "*", startLine, startColumn};
        case '/': return {TokenType::DIVIDE, "/", startLine, startColumn};
        case '%': return {TokenType::MODULO, "%", startLine, startColumn};
        case '=': return {TokenType::ASSIGN, "=", startLine, startColumn};
        case '<': return {TokenType::LESS, "<", startLine, startColumn};
        case '>': return {TokenType::GREATER, ">", startLine, startColumn};
        case '!': return {TokenType::NOT, "!", startLine, startColumn};
        case '(': return {TokenType::LPAREN, "(", startLine, startColumn};
        case ')': return {TokenType::RPAREN, ")", startLine, startColumn};
        case '{': return {TokenType::LBRACE, "{", startLine, startColumn};
        case '}': return {TokenType::RBRACE, "}", startLine, startColumn};
        case ';': return {TokenType::SEMICOLON, ";", startLine, startColumn};
        case ',': return {TokenType::COMMA, ",", startLine, startColumn};
        default: {
            errors_.push_back("Unexpected character '" + std::string(1, op) + "' at line " + std::to_string(startLine));
            return {TokenType::UNKNOWN, std::string(1, op), startLine, startColumn};
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        if (peek() == '/' && index_ + 1 < source_.size() && (source_[index_ + 1] == '/' || source_[index_ + 1] == '*')) {
            skipComment();
            if (isAtEnd()) break;
            continue;
        }

        char c = peek();
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(readNumber());
            continue;
        }
        if (c == '"') {
            tokens.push_back(readString());
            continue;
        }

        tokens.push_back(readOperator());
    }

    tokens.push_back({TokenType::EOF_TOKEN, "EOF", line_, column_});
    return tokens;
}

std::vector<std::string> Lexer::getErrors() const {
    return errors_;
}

}  // namespace compiler_cpp
