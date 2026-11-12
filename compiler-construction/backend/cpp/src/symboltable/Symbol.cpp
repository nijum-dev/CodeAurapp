#include "Symbol.hpp"

namespace compiler_cpp {

Symbol::Symbol() : name_(), type_(), line_(0) {}

Symbol::Symbol(std::string name, std::string type, int line)
    : name_(std::move(name)), type_(std::move(type)), line_(line) {}

const std::string& Symbol::getName() const {
    return name_;
}

const std::string& Symbol::getType() const {
    return type_;
}

int Symbol::getLine() const {
    return line_;
}

}  // namespace compiler_cpp
