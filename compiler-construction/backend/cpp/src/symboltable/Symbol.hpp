#pragma once

#include <string>

namespace compiler_cpp {

class Symbol {
public:
    Symbol();
    Symbol(std::string name, std::string type, int line);

    const std::string& getName() const;
    const std::string& getType() const;
    int getLine() const;

private:
    std::string name_;
    std::string type_;
    int line_;
};

}  // namespace compiler_cpp
