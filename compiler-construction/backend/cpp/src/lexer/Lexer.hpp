#pragma once

#include <string>
#include <vector>

#include "Token.hpp"

namespace compiler_cpp {

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();
    std::vector<std::string> getErrors() const;

private:
    std::string source_;
    std::size_t index_{0};
    int line_{1};
    int column_{1};
    std::vector<std::string> errors_;

    char peek() const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();
    void skipComment();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
    Token readOperator();
};

}  // namespace compiler_cpp
