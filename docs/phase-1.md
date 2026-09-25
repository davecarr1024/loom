# Phase 1: retired wide-register baseline

This page archives the first register-only implementation. It is historical:
`loom::Register<W>`, `loom/structure.h`, and `loom/circuit.h` have been removed
from the supported source tree. The implementation used atomic 1-to-64-bit
registers, object-identity connections, and path-based enable strings.

Its useful guarantees were stable owned definitions, independent simulation
state, all-or-nothing shared edges, nested same-width connections, simultaneous
swap, fan-out, and one-register-per-edge chain movement. Those behaviors now
run through `components::WordRegister`, `EnabledWordRegister`, and actual DFF
children in the current `simulation::Definition` / `simulation::Simulation`
engine. `tests/circuit_test.cpp` preserves exhaustive four-bit transfer,
hold, and swap coverage through the replacement interfaces.

The original baseline was checked with CMake, GCC, lcov, and clang-format 14.
The current presubmit uses Bazel and Clang/LLVM 19; run `./scripts/check.sh`.
See the [current status](status.md), [construction roadmap](roadmap.md), and
[timing contract](timing.md) for the supported design.
