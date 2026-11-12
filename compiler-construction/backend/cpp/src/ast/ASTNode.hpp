#pragma once

#include <memory>
#include <string>
#include <vector>

namespace compiler_cpp {

class ASTNode {
public:
    ASTNode(std::string type, std::string value = "", int line = 1);

    void addChild(std::shared_ptr<ASTNode> child);

    const std::string& getType() const;
    const std::string& getValue() const;
    void setValue(const std::string& value);
    int getLine() const;
    const std::vector<std::shared_ptr<ASTNode>>& getChildren() const;

    std::string toString() const;

private:
    std::string type_;
    std::string value_;
    int line_;
    std::vector<std::shared_ptr<ASTNode>> children_;
};

}  // namespace compiler_cpp
