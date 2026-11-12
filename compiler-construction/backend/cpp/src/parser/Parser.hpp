#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../ast/ASTNode.hpp"
#include "../lexer/Token.hpp"

namespace compiler_cpp {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::shared_ptr<ASTNode> parse();
    const std::vector<std::string>& getErrors() const;

private:
    std::vector<Token> tokens_;
    std::size_t index_{0};
    std::vector<std::string> errors_;

    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);
    Token advance();
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);

    std::shared_ptr<ASTNode> parseProgram();
    std::shared_ptr<ASTNode> parseMainFunction();
    std::shared_ptr<ASTNode> parseBlock();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseDeclaration();
    std::shared_ptr<ASTNode> parseAssignment();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseWhileStatement();
    std::shared_ptr<ASTNode> parseForStatement();
    std::shared_ptr<ASTNode> parsePrintStatement();
    std::shared_ptr<ASTNode> parseReturnStatement();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseLogicalOr();
    std::shared_ptr<ASTNode> parseLogicalAnd();
    std::shared_ptr<ASTNode> parseEquality();
    std::shared_ptr<ASTNode> parseComparison();
    std::shared_ptr<ASTNode> parseAdditive();
    std::shared_ptr<ASTNode> parseMultiplicative();
    std::shared_ptr<ASTNode> parseUnary();
    std::shared_ptr<ASTNode> parsePrimary();
    std::shared_ptr<ASTNode> parseDeclarationHeader();

    std::string parseTypeName();
    std::shared_ptr<ASTNode> makeNode(const std::string& type, const std::string& value = "", int line = 1) const;
};

}  // namespace compiler_cpp
