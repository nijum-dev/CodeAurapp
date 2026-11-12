#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../ast/ASTNode.hpp"

namespace compiler_cpp {

class TACGenerator {
public:
    std::vector<std::string> generate(const std::shared_ptr<ASTNode>& root);

private:
    std::vector<std::string> code_;
    int tempCount_{0};
    int labelCount_{0};

    void generateNode(const std::shared_ptr<ASTNode>& node);
    void generateIf(const std::shared_ptr<ASTNode>& node);
    void generateWhile(const std::shared_ptr<ASTNode>& node);
    void generateFor(const std::shared_ptr<ASTNode>& node);
    std::string generateExpression(const std::shared_ptr<ASTNode>& node);
    std::string newTemp();
    std::string newLabel();
};

}  // namespace compiler_cpp
