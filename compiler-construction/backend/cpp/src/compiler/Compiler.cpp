#include "Compiler.hpp"

#include <sstream>

#include "../intermediate/TACGenerator.hpp"
#include "../lexer/Lexer.hpp"
#include "../parser/Parser.hpp"
#include "../semantic/SemanticAnalyzer.hpp"

namespace compiler_cpp {

namespace {

std::string jsonEscape(const std::string& input) {
    std::ostringstream out;
    for (char ch : input) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << ch; break;
        }
    }
    return out.str();
}

std::string astToJson(const std::shared_ptr<ASTNode>& node) {
    if (!node) {
        return "null";
    }

    std::ostringstream out;
    out << "{\"type\":\"" << jsonEscape(node->getType()) << "\",";
    out << "\"value\":\"" << jsonEscape(node->getValue()) << "\",";
    out << "\"line\":" << node->getLine() << ",\"children\":[";

    for (std::size_t i = 0; i < node->getChildren().size(); ++i) {
        if (i > 0) out << ",";
        out << astToJson(node->getChildren()[i]);
    }
    out << "]}";
    return out.str();
}

std::string tokenListToJson(const std::vector<Token>& tokens) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) out << ",";
        out << "{\"type\":\"" << jsonEscape(tokenTypeToString(tokens[i].type))
            << "\",\"lexeme\":\"" << jsonEscape(tokens[i].lexeme)
            << "\",\"line\":" << tokens[i].line << ",\"column\":" << tokens[i].column << "}";
    }
    out << "]";
    return out.str();
}

std::string symbolListToJson(const std::vector<Symbol>& symbols) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < symbols.size(); ++i) {
        if (i > 0) out << ",";
        out << "{\"name\":\"" << jsonEscape(symbols[i].getName())
            << "\",\"type\":\"" << jsonEscape(symbols[i].getType())
            << "\",\"line\":" << symbols[i].getLine() << "}";
    }
    out << "]";
    return out.str();
}

std::string stringListToJson(const std::vector<std::string>& items) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) out << ",";
        out << "\"" << jsonEscape(items[i]) << "\"";
    }
    out << "]";
    return out.str();
}

}  // namespace

CompileResult Compiler::compile(const std::string& source) {
    CompileResult result;
    Lexer lexer(source);
    result.tokens = lexer.tokenize();

    const auto lexerErrors = lexer.getErrors();
    result.errors.insert(result.errors.end(), lexerErrors.begin(), lexerErrors.end());

    if (!lexerErrors.empty()) {
        result.success = false;
        return result;
    }

    Parser parser(result.tokens);
    result.ast = parser.parse();
    auto parserErrors = parser.getErrors();
    result.errors.insert(result.errors.end(), parserErrors.begin(), parserErrors.end());

    if (!parserErrors.empty()) {
        result.success = false;
        return result;
    }

    SemanticAnalyzer analyzer;
    auto semanticErrors = analyzer.analyze(result.ast);
    result.errors.insert(result.errors.end(), semanticErrors.begin(), semanticErrors.end());
    result.symbols = analyzer.getSymbolTable().getAllSymbols();

    TACGenerator tacGenerator;
    result.tac = tacGenerator.generate(result.ast);

    result.success = result.errors.empty();
    return result;
}

std::string Compiler::toJson(const CompileResult& result) {
    std::ostringstream out;
    out << "{\"success\":" << (result.success ? "true" : "false") << ",";
    out << "\"tokens\":" << tokenListToJson(result.tokens) << ",";
    out << "\"ast\":" << astToJson(result.ast) << ",";
    out << "\"symbols\":" << symbolListToJson(result.symbols) << ",";
    out << "\"errors\":" << stringListToJson(result.errors) << ",";
    out << "\"tac\":" << stringListToJson(result.tac) << "}";
    return out.str();
}

}  // namespace compiler_cpp
