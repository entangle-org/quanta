# Quanta Standard Library

The Quanta standard library contains reusable quantum gates, error correction modules, and classical helpers.

## Gates

- `import quanta.core.gates.h;`
- `import quanta.core.gates.cx;`
- All gate functions must be called within `@quantum` functions

## Quantum Error Correction (planned)

- `encode`, `decode`, `syndrome`, `correct` from `quanta.core.qec.*`

## Classical Math

- `import quanta.core.math.*;` — exposes `sin`, `cos`, etc.

More coming soon.