#include "ASTNode.hpp"

namespace compiler_cpp {

ASTNode::ASTNode(std::string type, std::string value, int line)
    : type_(std::move(type)), value_(std::move(value)), line_(line) {}

void ASTNode::addChild(std::shared_ptr<ASTNode> child) {
    children_.push_back(std::move(child));
}

const std::string& ASTNode::getType() const {
    return type_;
}

const std::string& ASTNode::getValue() const {
    return value_;
}

void ASTNode::setValue(const std::string& value) {
    value_ = value;
}

int ASTNode::getLine() const {
    return line_;
}

const std::vector<std::shared_ptr<ASTNode>>& ASTNode::getChildren() const {
    return children_;
}

std::string ASTNode::toString() const {
    return type_ + (value_.empty() ? "" : ": " + value_);
}

}  // namespace compiler_cpp
