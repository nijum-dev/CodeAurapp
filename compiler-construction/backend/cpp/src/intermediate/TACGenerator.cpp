#include "TACGenerator.hpp"

namespace compiler_cpp {

std::vector<std::string> TACGenerator::generate(const std::shared_ptr<ASTNode>& root) {
    code_.clear();
    tempCount_ = 0;
    labelCount_ = 0;
    generateNode(root);
    return code_;
}

void TACGenerator::generateNode(const std::shared_ptr<ASTNode>& node) {
    if (!node) {
        return;
    }

    const std::string& type = node->getType();
    if (type == "PROGRAM" || type == "BLOCK") {
        for (const auto& child : node->getChildren()) {
            generateNode(child);
        }
        return;
    }
    if (type == "MAIN_FUNCTION") {
        generateNode(node->getChildren().front());
        return;
    }
    if (type == "DECLARATION") {
        if (node->getChildren().size() > 1) {
            const std::string value = generateExpression(node->getChildren().back());
            code_.push_back(node->getChildren().front()->getValue() + " = " + value);
        }
        return;
    }
    if (type == "ASSIGNMENT") {
        const std::string value = generateExpression(node->getChildren().front());
        code_.push_back(node->getValue() + " = " + value);
        return;
    }
    if (type == "PRINT") {
        for (const auto& child : node->getChildren()) {
            if (child->getType() == "LITERAL" && child->getValue() == "endl") {
                code_.push_back("print endl");
            } else {
                code_.push_back("print " + generateExpression(child));
            }
        }
        return;
    }
    if (type == "IF") {
        generateIf(node);
        return;
    }
    if (type == "WHILE") {
        generateWhile(node);
        return;
    }
    if (type == "FOR") {
        generateFor(node);
        return;
    }
    if (type == "RETURN") {
        if (!node->getChildren().empty()) {
            code_.push_back("return " + generateExpression(node->getChildren().front()));
        } else {
            code_.push_back("return");
        }
        return;
    }

    for (const auto& child : node->getChildren()) {
        generateNode(child);
    }
}

void TACGenerator::generateIf(const std::shared_ptr<ASTNode>& node) {
    const std::string condition = generateExpression(node->getChildren().front());
    const std::string elseLabel = newLabel();
    const std::string endLabel = newLabel();

    code_.push_back("ifFalse " + condition + " goto " + elseLabel);
    generateNode(node->getChildren()[1]);

    if (node->getChildren().size() > 2) {
        code_.push_back("goto " + endLabel);
        code_.push_back(elseLabel + ":");
        generateNode(node->getChildren()[2]);
        code_.push_back(endLabel + ":");
    } else {
        code_.push_back(elseLabel + ":");
    }
}

void TACGenerator::generateWhile(const std::shared_ptr<ASTNode>& node) {
    const std::string start = newLabel();
    const std::string end = newLabel();

    code_.push_back(start + ":");
    const std::string condition = generateExpression(node->getChildren().front());
    code_.push_back("ifFalse " + condition + " goto " + end);
    generateNode(node->getChildren()[1]);

    code_.push_back("goto " + start);
    code_.push_back(end + ":");
}

void TACGenerator::generateFor(const std::shared_ptr<ASTNode>& node) {
    const auto& children = node->getChildren();
    if (children.size() >= 4) {
        if (children[0]->getType() != "EMPTY") {
            generateNode(children[0]);
        }

        const std::string startLabel = newLabel();
        const std::string endLabel = newLabel();
        code_.push_back(startLabel + ":");

        if (children[1]->getType() != "EMPTY") {
            const std::string condition = generateExpression(children[1]);
            code_.push_back("ifFalse " + condition + " goto " + endLabel);
        }

        generateNode(children[3]);

        if (children[2]->getType() != "EMPTY") {
            generateNode(children[2]);
        }

        code_.push_back("goto " + startLabel);
        code_.push_back(endLabel + ":");
    }
}

std::string TACGenerator::generateExpression(const std::shared_ptr<ASTNode>& node) {
    if (!node) {
        return "";
    }

    const std::string& type = node->getType();
    if (type == "LITERAL" || type == "IDENTIFIER") {
        return node->getValue();
    }

    if (type == "BINARY") {
        const std::string left = generateExpression(node->getChildren().front());
        const std::string right = generateExpression(node->getChildren().back());
        const std::string temp = newTemp();
        code_.push_back(temp + " = " + left + " " + node->getValue() + " " + right);
        return temp;
    }

    if (type == "UNARY") {
        const std::string operand = generateExpression(node->getChildren().front());
        const std::string result = newTemp();
        code_.push_back(result + " = " + node->getValue() + operand);
        return result;
    }

    return "";
}

std::string TACGenerator::newTemp() {
    return "t" + std::to_string(++tempCount_);
}

std::string TACGenerator::newLabel() {
    return "L" + std::to_string(++labelCount_);
}

}  // namespace compiler_cpp
