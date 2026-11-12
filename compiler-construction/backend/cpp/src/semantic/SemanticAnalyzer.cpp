#include "SemanticAnalyzer.hpp"

#include <utility>

namespace compiler_cpp {

std::vector<std::string> SemanticAnalyzer::analyze(const std::shared_ptr<ASTNode>& root) {
    errors_.clear();
    symbolTable_ = SymbolTable();
    analyzeNode(root);
    return errors_;
}

const SymbolTable& SemanticAnalyzer::getSymbolTable() const {
    return symbolTable_;
}

const std::vector<std::string>& SemanticAnalyzer::getErrors() const {
    return errors_;
}

void SemanticAnalyzer::analyzeNode(const std::shared_ptr<ASTNode>& node) {
    if (!node) {
        return;
    }

    const std::string& type = node->getType();
    if (type == "PROGRAM") {
        for (const auto& child : node->getChildren()) {
            analyzeNode(child);
        }
        return;
    }
    if (type == "MAIN_FUNCTION") {
        for (const auto& child : node->getChildren()) {
            analyzeNode(child);
        }
        return;
    }
    if (type == "BLOCK") {
        symbolTable_.enterScope();
        for (const auto& child : node->getChildren()) {
            analyzeNode(child);
        }
        symbolTable_.exitScope();
        return;
    }
    if (type == "DECLARATION") {
        declareVariable(node);
        return;
    }
    if (type == "ASSIGNMENT") {
        checkAssignment(node);
        return;
    }
    if (type == "IF" || type == "WHILE") {
        analyzeCondition(node);
        return;
    }
    if (type == "FOR") {
        analyzeFor(node);
        return;
    }
    if (type == "RETURN") {
        if (!node->getChildren().empty()) {
            checkExpression(node->getChildren().front());
        }
        return;
    }

    for (const auto& child : node->getChildren()) {
        analyzeNode(child);
    }
}

void SemanticAnalyzer::declareVariable(const std::shared_ptr<ASTNode>& node) {
    if (node->getChildren().empty()) {
        return;
    }

    const std::string variableName = node->getChildren().front()->getValue();
    const std::string variableType = node->getValue();
    Symbol symbol(variableName, variableType, node->getLine());

    if (!symbolTable_.declare(symbol)) {
        error(node, "Variable already declared: " + variableName);
    }

    if (node->getChildren().size() > 1) {
        const std::string expressionType = checkExpression(node->getChildren().back());
        if (!isCompatible(variableType, expressionType)) {
            error(node, "Cannot initialize " + variableType + " variable '" + variableName + "' with " + expressionType + " value.");
        }
    }
}

void SemanticAnalyzer::checkAssignment(const std::shared_ptr<ASTNode>& node) {
    const std::string name = node->getValue();
    const Symbol* symbol = symbolTable_.lookup(name);

    if (symbol == nullptr) {
        error(node, "Undeclared variable: " + name);
        if (!node->getChildren().empty()) {
            checkExpression(node->getChildren().front());
        }
        return;
    }

    const std::string expressionType = checkExpression(node->getChildren().front());
    if (!isCompatible(symbol->getType(), expressionType)) {
        error(node, "Cannot assign " + expressionType + " value to " + symbol->getType() + " variable '" + name + "'.");
    }
}

void SemanticAnalyzer::analyzeCondition(const std::shared_ptr<ASTNode>& node) {
    if (!node->getChildren().empty()) {
        const std::string conditionType = checkExpression(node->getChildren().front());
        if (!conditionType.empty() && conditionType != "bool") {
            error(node, "Condition must be of type bool.");
        }
    }

    for (std::size_t i = 1; i < node->getChildren().size(); ++i) {
        analyzeNode(node->getChildren()[i]);
    }
}

void SemanticAnalyzer::analyzeFor(const std::shared_ptr<ASTNode>& node) {
    symbolTable_.enterScope();

    const auto& children = node->getChildren();
    if (children.size() >= 4) {
        if (children[0]->getType() != "EMPTY") {
            analyzeNode(children[0]);
        }
        if (children[1]->getType() != "EMPTY") {
            const std::string conditionType = checkExpression(children[1]);
            if (!conditionType.empty() && conditionType != "bool") {
                error(node, "For loop condition must be of type bool.");
            }
        }
        if (children[2]->getType() != "EMPTY") {
            analyzeNode(children[2]);
        }
        analyzeNode(children[3]);
    }

    symbolTable_.exitScope();
}

std::string SemanticAnalyzer::checkExpression(const std::shared_ptr<ASTNode>& node) {
    if (!node) {
        return "";
    }

    const std::string& type = node->getType();
    if (type == "LITERAL") {
        return getLiteralType(node->getValue());
    }
    if (type == "IDENTIFIER") {
        const Symbol* symbol = symbolTable_.lookup(node->getValue());
        if (symbol == nullptr) {
            error(node, "Undeclared variable: " + node->getValue());
            return "";
        }
        return symbol->getType();
    }
    if (type == "UNARY") {
        return checkUnaryExpression(node);
    }
    if (type == "BINARY") {
        return checkBinaryExpression(node);
    }

    std::string result;
    for (const auto& child : node->getChildren()) {
        result = checkExpression(child);
    }
    return result;
}

std::string SemanticAnalyzer::checkUnaryExpression(const std::shared_ptr<ASTNode>& node) {
    const std::string op = node->getValue();
    const std::string operandType = checkExpression(node->getChildren().front());
    if (operandType.empty()) {
        return "";
    }

    if (op == "!") {
        if (operandType != "bool") {
            error(node, "Logical NOT requires a bool operand.");
            return "";
        }
        return "bool";
    }

    if (op == "-" || op == "+") {
        if (!isNumeric(operandType)) {
            error(node, "Unary " + op + " requires a numeric operand.");
            return "";
        }
        return operandType;
    }

    return "";
}

std::string SemanticAnalyzer::checkBinaryExpression(const std::shared_ptr<ASTNode>& node) {
    const std::string op = node->getValue();
    const std::string leftType = checkExpression(node->getChildren().front());
    const std::string rightType = checkExpression(node->getChildren().back());

    if (leftType.empty() || rightType.empty()) {
        return "";
    }

    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (!isNumeric(leftType) || !isNumeric(rightType)) {
            error(node, "Arithmetic operator '" + op + "' requires numeric operands.");
            return "";
        }
        if (op == "%" && (leftType != "int" || rightType != "int")) {
            error(node, "Modulo operator requires int operands.");
            return "";
        }
        return (leftType == "float" || rightType == "float") ? "float" : "int";
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (!isNumeric(leftType) || !isNumeric(rightType)) {
            error(node, "Comparison operator '" + op + "' requires numeric operands.");
            return "";
        }
        return "bool";
    }

    if (op == "==" || op == "!=") {
        if (!isCompatible(leftType, rightType) && !isCompatible(rightType, leftType)) {
            error(node, "Incompatible types in equality comparison.");
            return "";
        }
        return "bool";
    }

    if (op == "&&" || op == "||") {
        if (leftType != "bool" || rightType != "bool") {
            error(node, "Logical operator '" + op + "' requires bool operands.");
            return "";
        }
        return "bool";
    }

    error(node, "Unknown operator: " + op);
    return "";
}

std::string SemanticAnalyzer::getLiteralType(const std::string& value) const {
    if (value == "true" || value == "false") {
        return "bool";
    }
    if (value.find('.') != std::string::npos) {
        return "float";
    }
    return "int";
}

bool SemanticAnalyzer::isNumeric(const std::string& type) const {
    return type == "int" || type == "float";
}

bool SemanticAnalyzer::isCompatible(const std::string& target, const std::string& source) const {
    if (target == source) {
        return true;
    }
    return target == "float" && source == "int";
}

void SemanticAnalyzer::error(const std::shared_ptr<ASTNode>& node, const std::string& message) {
    errors_.push_back("Line " + std::to_string(node->getLine()) + ": " + message);
}

}  // namespace compiler_cpp
