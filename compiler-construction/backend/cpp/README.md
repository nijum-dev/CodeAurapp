# C++ Compiler Backend

This backend implements a mini C++ compiler pipeline for the compiler-construction project.

Included phases:
- Lexical analysis
- Syntax analysis
- AST generation
- Symbol table construction
- Semantic analysis
- Three Address Code (TAC) generation

## Folder structure
- src/ast: AST node definitions
- src/compiler: orchestrates compilation and JSON output
- src/intermediate: TAC generation
- src/lexer: tokenization and lexing rules
- src/parser: recursive-descent parser
- src/semantic: semantic validation
- src/symboltable: scope and symbol management
- tests: valid and invalid sample source programs

## Build
```bash
cd compiler-construction/backend/cpp
cmake -S . -B build
cmake --build build
```

## Run
```bash
./build/cpp_compiler tests/sample_valid.cpp
./build/cpp_compiler tests/sample_invalid.cpp
```

The compiler prints a JSON result containing:
- success
- tokens
- AST
- symbols
- semantic errors
- TAC instructions
