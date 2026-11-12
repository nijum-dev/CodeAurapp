#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../ast/ASTNode.hpp"
#include "../lexer/Token.hpp"
#include "../symboltable/Symbol.hpp"

namespace compiler_cpp {

struct CompileResult {
    bool success{false};
    std::vector<Token> tokens;
    std::shared_ptr<ASTNode> ast;
    std::vector<Symbol> symbols;
    std::vector<std::string> errors;
    std::vector<std::string> tac;
};

}  // namespace compiler_cpp
