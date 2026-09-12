# Loom

A headless C++23 laboratory for building understandable digital machines from
small, typed circuits.

**Can increasingly complex machines preserve local reasoning, reproducible
failures, and explanations at every level?** Loom explores that question by
building upward: gates and flip-flops, registers and buses, arithmetic and
memory, controllers and successive CPUs.

The abstraction floor keeps evaluation simple: a small Boolean basis and
explicit one-bit storage, with acyclic combinational propagation between shared
clock edges. Larger components must execute as circuits of smaller components.
Hardware-ish modeling and constrained interfaces make state, timing, and valid
choices visible. Tests harden each component and its containing assemblies.

## Status

[NOT and the typed observation harness](docs/not.md) are implemented. A single
NOT and a two-NOT circuit execute from their actual owned gates and connections,
with call-scoped inputs, derived inventory/scheduling, and retained signal evidence.
The next component is AND; the complete atomic floor is not implemented yet.

The tested register-only Phase 1 baseline remains transitional, with atomic wide
registers to be replaced by composed flip-flops. See [roadmap](docs/roadmap.md)
for construction order and [status](docs/status.md) for evidence and limitations.

## Build and verify

Requires GCC 12/C++23, CMake, Make, Python 3.9+, lcov, clang-format, and
clang-tidy. CMake fetches GoogleTest v1.14.0. See [Phase 1](docs/phase-1.md)
for verified tool versions, the clang-tidy compatibility exception, and offline setup.

```sh
make check
./build/transfer
./build/not_demo
```

The `verify` Actions job runs the same documentation, behavioral, compile-fail,
formatting, compatible static-analysis, and production coverage gates.

## Design record

- [Design](docs/design.md): thesis, atom floor, representation, and scope.
- [Timing](docs/timing.md): authoritative discrete-time execution contract.
- [Component contracts](docs/component-contracts.md): interface, type-family, acceptance, and regression discipline.
- [Roadmap](docs/roadmap.md): component dependencies, proofs, and checkpoints.
- [Decisions](docs/decisions.md): revised direction and withdrawn assumptions.
- [Baseline](docs/baseline.md): inherited IRATA2 and Rule Lab lessons.
- [Status](docs/status.md): what runs today and what comes next.
- [Agent guide](AGENTS.md): contribution workflow and verification rules.
