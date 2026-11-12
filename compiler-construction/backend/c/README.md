# C compiler backend

This directory contains a standalone C99 compiler frontend for the initial C
subset used by the compiler-construction project.

Supported constructs:

- `int`, `float`, and `bool` declarations
- assignments and arithmetic/logical expressions
- `if`/`else`, `while`, and `for`
- `printf(expression);`
- `//` and `/* ... */` comments

The pipeline produces tokens, an AST, scoped symbols, three-address code, and
diagnostics. It is intentionally backend-only; the existing React application
and Java API are unchanged.

## Build and run

```sh
make
./c-compiler path/to/program.c
```

The compiler returns a non-zero exit status when lexing, parsing, or semantic
diagnostics are reported.
