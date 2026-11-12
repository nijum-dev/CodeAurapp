#include <fstream>
#include <iostream>
#include <string>

#include "src/compiler/Compiler.hpp"

using compiler_cpp::Compiler;

std::string readFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Unable to open file: " + path);
    }
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

int main(int argc, char* argv[]) {
    try {
        std::string source;

        if (argc > 1) {
            source = readFile(argv[1]);
        } else {
            std::getline(std::cin, source, '\0');
        }

        Compiler compiler;
        auto result = compiler.compile(source);
        std::cout << Compiler::toJson(result) << std::endl;
        return result.success ? 0 : 1;
    } catch (const std::exception& ex) {
        std::cout << "{\"success\":false,\"errors\":[\"" << ex.what() << "\"]}" << std::endl;
        return 1;
    }
}
