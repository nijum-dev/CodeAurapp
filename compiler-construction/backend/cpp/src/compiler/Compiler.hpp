#pragma once

#include <string>

#include "CompileResult.hpp"

namespace compiler_cpp {

class Compiler {
public:
    CompileResult compile(const std::string& source);
    static std::string toJson(const CompileResult& result);
};

}  // namespace compiler_cpp
