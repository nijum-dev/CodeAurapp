#pragma once

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "Symbol.hpp"

namespace compiler_cpp {

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool declare(const Symbol& symbol);
    const Symbol* lookup(const std::string& name) const;
    std::vector<Symbol> getAllSymbols() const;

private:
    std::deque<std::map<std::string, Symbol>> scopes_;
    std::vector<Symbol> allSymbols_;
};

}  // namespace compiler_cpp
