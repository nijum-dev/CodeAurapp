# Compiler Construction Project

This repository contains a compiler-construction project with a Java-based frontend and compiler pipeline plus a C++ backend implementation.

## Structure
- backend/java: Java compiler implementation and API service
- backend/cpp: C++ compiler backend with lexical analysis, parser, AST, symbol table, semantic analysis, and TAC generation
- backend/c: C backend scaffold
- frontend: React frontend for demonstrating compiler output
- docs: grammar and project documentation
- examples: valid and invalid sample programs

## Getting started

### C++ backend
```bash
cd compiler-construction/backend/cpp
cmake -S . -B build
cmake --build build
./build/cpp_compiler tests/sample_valid.cpp
```
