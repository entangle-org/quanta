# Quanta Language Specification

## Top-Level Program

A Quanta program is composed of:
- Import statements
- Function declarations
- Class declarations
- Top-level variable declarations

## Types

- `int`, `float`, `char`, `string`, `bit`, `qubit`
- `logical<code>` (planned)
- Arrays: `qubit[]`, `int[]`, etc.

## Function Declarations
Functions in **Quanta** can be either classical or quantum. A classical function is declared via the following syntax
```quanta
function name(params) -> returnType { ... }
```
A quantum function is declared using the `@quantum` annotation and may have a returnType of `void` or `bit` only
```quanta
@quantum
function teleport(qubit a, qubit b) -> void { ... }
```
---
## Annotations
- `@quantum` — marks function as quantum
- `@adjoint` — generate inverse of quantum function
- `@state('0')` — initialize qubit symbolically
---
## Classes
You can define custom classes which can then be imported. Classes may only contain methods

`MyClass.quanta`
```quanta
class MyClass {

  function add(int a, int b) {
    return a + b;
  } 

  @quantum
  function entangle(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
  }
}
```
Then this can be imported

`main.quanta`
```quanta
import MyClass

function main() -> int {
    return MyClass.add(20, 22);
}
```
---
## Statements
- `return`, `if`, `for`, `reset`, `measure`
- variable declarations, e.g. `final int x = 5;`