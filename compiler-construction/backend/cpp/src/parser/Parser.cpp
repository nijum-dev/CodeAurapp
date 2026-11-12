#include "Parser.hpp"

#include <initializer_list>
#include <utility>

namespace compiler_cpp {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

std::shared_ptr<ASTNode> Parser::parse() {
    auto root = parseProgram();
    if (!isAtEnd()) {
        errors_.push_back("Unexpected trailing token: " + tokens_[index_].lexeme);
    }
    return root;
}

const std::vector<std::string>& Parser::getErrors() const {
    return errors_;
}

bool Parser::isAtEnd() const {
    return index_ >= tokens_.size() || tokens_[index_].type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const {
    return !isAtEnd() && tokens_[index_].type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::advance() {
    if (!isAtEnd()) {
        return tokens_[index_++];
    }
    return tokens_.back();
}

Token Parser::peek() const {
    return isAtEnd() ? tokens_.back() : tokens_[index_];
}

Token Parser::previous() const {
    return index_ == 0 ? tokens_.front() : tokens_[index_ - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    errors_.push_back("Line " + std::to_string(peek().line) + ": " + message);
    return {type, "", peek().line, peek().column};
}

std::shared_ptr<ASTNode> Parser::makeNode(const std::string& type, const std::string& value, int line) const {
    return std::make_shared<ASTNode>(type, value, line);
}

std::string Parser::parseTypeName() {
    if (match({TokenType::INT})) return "int";
    if (match({TokenType::FLOAT})) return "float";
    if (match({TokenType::BOOL})) return "bool";
    errors_.push_back("Line " + std::to_string(peek().line) + ": expected a type");
    return "unknown";
}

std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto program = makeNode("PROGRAM");
    program->addChild(parseMainFunction());
    return program;
}

std::shared_ptr<ASTNode> Parser::parseMainFunction() {
    auto mainNode = makeNode("MAIN_FUNCTION");

    std::string type = parseTypeName();
    consume(TokenType::MAIN, "expected 'main'");
    consume(TokenType::LPAREN, "expected '(' after main");
    consume(TokenType::RPAREN, "expected ')' after main");
    mainNode->addChild(parseBlock());
    mainNode->setValue(type);
    return mainNode;
}

std::shared_ptr<ASTNode> Parser::parseBlock() {
    auto block = makeNode("BLOCK");
    consume(TokenType::LBRACE, "expected '{'");
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->addChild(parseStatement());
    }
    consume(TokenType::RBRACE, "expected '}'");
    return block;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (check(TokenType::INT) || check(TokenType::FLOAT) || check(TokenType::BOOL)) {
        return parseDeclaration();
    }
    if (check(TokenType::IDENTIFIER)) {
        const std::size_t saved = index_;
        const std::string nextLexeme = tokens_[index_ + 1].lexeme;
        if (index_ + 1 < tokens_.size() && tokens_[index_ + 1].type == TokenType::ASSIGN) {
            return parseAssignment();
        }
        if (index_ + 1 < tokens_.size() && tokens_[index_ + 1].type == TokenType::LPAREN) {
            return parseExpression();
        }
        if (index_ + 1 < tokens_.size() && nextLexeme == "++") {
            return parseAssignment();
        }
        index_ = saved;
    }
    if (check(TokenType::IF)) return parseIfStatement();
    if (check(TokenType::WHILE)) return parseWhileStatement();
    if (check(TokenType::FOR)) return parseForStatement();
    if (check(TokenType::COUT)) return parsePrintStatement();
    if (check(TokenType::RETURN)) return parseReturnStatement();
    if (check(TokenType::LBRACE)) return parseBlock();

    errors_.push_back("Line " + std::to_string(peek().line) + ": unexpected statement");
    advance();
    return makeNode("UNKNOWN");
}

std::shared_ptr<ASTNode> Parser::parseDeclaration() {
    std::string type = parseTypeName();
    auto identifier = consume(TokenType::IDENTIFIER, "expected identifier in declaration");
    auto declaration = makeNode("DECLARATION", type, identifier.line);
    declaration->addChild(makeNode("IDENTIFIER", identifier.lexeme, identifier.line));

    if (match({TokenType::ASSIGN})) {
        declaration->addChild(parseExpression());
    }

    consume(TokenType::SEMICOLON, "expected ';' after declaration");
    return declaration;
}

std::shared_ptr<ASTNode> Parser::parseAssignment() {
    Token identifier = consume(TokenType::IDENTIFIER, "expected identifier in assignment");
    consume(TokenType::ASSIGN, "expected '=' in assignment");
    auto assignment = makeNode("ASSIGNMENT", identifier.lexeme, identifier.line);
    assignment->addChild(parseExpression());
    consume(TokenType::SEMICOLON, "expected ';' after assignment");
    return assignment;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    auto ifNode = makeNode("IF");
    consume(TokenType::IF, "expected 'if'");
    consume(TokenType::LPAREN, "expected '(' after if");
    ifNode->addChild(parseExpression());
    consume(TokenType::RPAREN, "expected ')' after condition");
    ifNode->addChild(parseStatement());
    if (match({TokenType::ELSE})) {
        ifNode->addChild(parseStatement());
    }
    return ifNode;
}

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    auto whileNode = makeNode("WHILE");
    consume(TokenType::WHILE, "expected 'while'");
    consume(TokenType::LPAREN, "expected '(' after while");
    whileNode->addChild(parseExpression());
    consume(TokenType::RPAREN, "expected ')' after condition");
    whileNode->addChild(parseStatement());
    return whileNode;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {
    auto forNode = makeNode("FOR");
    consume(TokenType::FOR, "expected 'for'");
    consume(TokenType::LPAREN, "expected '(' after for");

    if (!check(TokenType::SEMICOLON)) {
        if (check(TokenType::INT) || check(TokenType::FLOAT) || check(TokenType::BOOL)) {
            forNode->addChild(parseDeclaration());
        } else {
            forNode->addChild(parseAssignment());
        }
    } else {
        forNode->addChild(makeNode("EMPTY"));
    }
    consume(TokenType::SEMICOLON, "expected ';' after for init");

    if (!check(TokenType::SEMICOLON)) {
        forNode->addChild(parseExpression());
    } else {
        forNode->addChild(makeNode("EMPTY"));
    }
    consume(TokenType::SEMICOLON, "expected ';' after for condition");

    if (!check(TokenType::RPAREN)) {
        forNode->addChild(parseAssignment());
    } else {
        forNode->addChild(makeNode("EMPTY"));
    }
    consume(TokenType::RPAREN, "expected ')' after for update");
    forNode->addChild(parseStatement());
    return forNode;
}

std::shared_ptr<ASTNode> Parser::parsePrintStatement() {
    auto printNode = makeNode("PRINT");
    consume(TokenType::COUT, "expected 'cout'");

    while (match({TokenType::SHIFT})) {
        if (check(TokenType::ENDL)) {
            printNode->addChild(makeNode("LITERAL", "endl", peek().line));
            advance();
        } else {
            printNode->addChild(parseExpression());
        }
    }

    consume(TokenType::SEMICOLON, "expected ';' after print statement");
    return printNode;
}

std::shared_ptr<ASTNode> Parser::parseReturnStatement() {
    auto returnNode = makeNode("RETURN");
    consume(TokenType::RETURN, "expected 'return'");
    if (!check(TokenType::SEMICOLON)) {
        returnNode->addChild(parseExpression());
    }
    consume(TokenType::SEMICOLON, "expected ';' after return");
    return returnNode;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseLogicalOr();
}

std::shared_ptr<ASTNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (match({TokenType::OR})) {
        Token op = previous();
        auto right = parseLogicalAnd();
        auto node = makeNode("BINARY", op.lexeme, op.line);
        node->addChild(left);
        node->addChild(right);
        left = node;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseLogicalAnd() {
    auto left = parseEquality();
    while (match({TokenType::AND})) {
        Token op = previous();
        auto right = parseEquality();
        auto node = makeNode("BINARY", op.lexeme, op.line);
        node->addChild(left);
        node->addChild(right);
        left = node;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseEquality() {
    auto left = parseComparison();
    while (match({TokenType::EQUAL, TokenType::NOT_EQUAL})) {
        Token op = previous();
        auto right = parseComparison();
        auto node = makeNode("BINARY", op.lexeme, op.line);
        node->addChild(left);
        node->addChild(right);
        left = node;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseAdditive();
    while (match({TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL})) {
        Token op = previous();
        auto right = parseAdditive();
        auto node = makeNode("BINARY", op.lexeme, op.line);
        node->addChild(left);
        node->addChild(right);
        left = node;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = parseMultiplicative();
        auto node = makeNode("BINARY", op.lexeme, op.line);
        node->addChild(left);
        node->addChild(right);
        left = node;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseMultiplicative() {
    auto left = parseUnary();
    while (match({TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO})) {
        Token op = previous();
        auto right = parseUnary();
        auto node = makeNode("BINARY", op.lexeme, op.line);
        node->addChild(left);
        node->addChild(right);
        left = node;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseUnary() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        auto operand = parseUnary();
        auto node = makeNode("UNARY", op.lexeme, op.line);
        node->addChild(operand);
        return node;
    }
    return parsePrimary();
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    if (match({TokenType::INTEGER_LITERAL, TokenType::FLOAT_LITERAL, TokenType::STRING_LITERAL, TokenType::BOOLEAN_LITERAL})) {
        Token literal = previous();
        return makeNode("LITERAL", literal.lexeme, literal.line);
    }

    if (check(TokenType::IDENTIFIER)) {
        Token ident = advance();
        return makeNode("IDENTIFIER", ident.lexeme, ident.line);
    }

    if (match({TokenType::LPAREN})) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "expected ')' after expression");
        return expr;
    }

    errors_.push_back("Line " + std::to_string(peek().line) + ": unexpected token in expression");
    advance();
    return makeNode("UNKNOWN");
}

}  // namespace compiler_cpp
