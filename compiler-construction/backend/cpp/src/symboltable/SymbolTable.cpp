#include "SymbolTable.hpp"

namespace compiler_cpp {

SymbolTable::SymbolTable() {
    enterScope();
}

void SymbolTable::enterScope() {
    scopes_.emplace_front();
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_front();
    }
}

bool SymbolTable::declare(const Symbol& symbol) {
    if (scopes_.empty()) {
        return false;
    }
    auto& current = scopes_.front();
    if (current.find(symbol.getName()) != current.end()) {
        return false;
    }
    current[symbol.getName()] = symbol;
    allSymbols_.push_back(symbol);
    return true;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    for (const auto& scope : scopes_) {
        auto it = scope.find(name);
        if (it != scope.end()) {
            return &it->second;
        }
    }
    return nullptr;
}

std::vector<Symbol> SymbolTable::getAllSymbols() const {
    return allSymbols_;
}

}  // namespace compiler_cpp
