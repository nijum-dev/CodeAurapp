#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../ast/ASTNode.hpp"
#include "../symboltable/SymbolTable.hpp"

namespace compiler_cpp {

class SemanticAnalyzer {
public:
    std::vector<std::string> analyze(const std::shared_ptr<ASTNode>& root);
    const SymbolTable& getSymbolTable() const;
    const std::vector<std::string>& getErrors() const;

private:
    SymbolTable symbolTable_;
    std::vector<std::string> errors_;

    void analyzeNode(const std::shared_ptr<ASTNode>& node);
    void declareVariable(const std::shared_ptr<ASTNode>& node);
    void checkAssignment(const std::shared_ptr<ASTNode>& node);
    void analyzeCondition(const std::shared_ptr<ASTNode>& node);
    void analyzeFor(const std::shared_ptr<ASTNode>& node);
    std::string checkExpression(const std::shared_ptr<ASTNode>& node);
    std::string checkUnaryExpression(const std::shared_ptr<ASTNode>& node);
    std::string checkBinaryExpression(const std::shared_ptr<ASTNode>& node);
    std::string getLiteralType(const std::string& value) const;
    bool isNumeric(const std::string& type) const;
    bool isCompatible(const std::string& target, const std::string& source) const;
    void error(const std::shared_ptr<ASTNode>& node, const std::string& message);
};

}  // namespace compiler_cpp
