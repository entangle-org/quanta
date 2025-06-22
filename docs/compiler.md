# Quanta Compiler Architecture

## Overview

The Quanta compiler transforms `.quanta` files into:

- **C++**: For classical control (e.g. conditionals, loops, main logic)
- **OpenQASM**: For quantum functions (annotated with `@quantum`)

## Compiler Phases

1. **Lexing** — Produces a stream of typed tokens
2. **Parsing** — Builds an abstract syntax tree (AST)
3. **Static Analysis** — Type checking, scope analysis (WIP)
4. **Code Generation**
   - Classical AST → C++
   - Quantum AST → OpenQASM
5. **Execution**
   - Classical code compiled to native binary
   - Quantum code passed to built-in simulator (or real backend in future)

## Entry Point

The entry point is a classical `function main() -> int { ... }`.

Quantum functions must be marked with `@quantum`.

## CLI Usage

```bash
quanta my_program.quanta --shots=1024
```

## Runtime Support
- Ideal simulator built-in