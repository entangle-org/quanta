# Quanta Programming Language

**Quanta** is a strongly typed, compiled quantum programming language designed for hybrid classical-quantum computation. It combines classical control with OpenQASM-compatible quantum semantics, enabling intuitive yet powerful quantum algorithm development.

---
## ✨ Features

- 🧠 **Hybrid execution model**: Classical code compiles to C++, quantum functions compile to OpenQASM.
- 🧮 **Quantum & classical primitives**: `qubit`, `bit`, `int`, `float`, `char`, `string`
- 🛡️ **Immutability**: `final` keyword for read-only declarations
- 🧵 **Annotations**:
  - `@quantum` — marks functions for OpenQASM codegen
  - `@adjoint` — auto-generates inverse of a quantum function
  - `@state('0')` — symbolic qubit initialization
- 📦 **Modules & Imports**: e.g. `import quanta.core.gates.h;`
- 🧰 **Standard library support** for gates, QEC, utilities (in progress)
- 📈 **Built-in simulator**: Ideal quantum circuit simulation out of the box
- 💬 **Built-in `echo(...)`** for classical output

---
## 🧪 Example

```quanta
@quantum
function bell(qubit q0, qubit q1) -> void {
  h(q0);
  cx(q0, q1);
}

final int n = 5;
@state('0') qubit q0;
bit b0 = measure q0;
```
---
## 🚀 How It Works
- The **Quanta** compiler translates classical code to C++ and quantum code to OpenQASM.
- The CLI executes `.quanta` source files
```bash
quanta program.quanta
quanta program.quanta --shots=1024
```
---
## 📚 Docs
See `/docs` for 
- Language specification
- Grammar
- Standard library
- How the compiler works

