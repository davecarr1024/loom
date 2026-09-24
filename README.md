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

[NOT, AND, OR, constant bit, DFF, and the typed observation harness](docs/not.md)
are implemented. Single NOT, AND, OR, and constant bit atoms, plus their
composed circuits, execute from owned components and connections. Stateful
circuits use independent simulation state with pure observations and atomic
shared edges. The next group begins with gate-composed selection; the initial
atomic floor is complete. See [DFF](docs/d-flip-flop.md).

The tested register-only Phase 1 baseline remains transitional, with atomic wide
registers to be replaced by composed flip-flops. See [roadmap](docs/roadmap.md)
for construction order and [status](docs/status.md) for evidence and limitations.
See [AND](docs/and.md) and [OR](docs/or.md) for their two-input contracts and
evidence.

## Build and verify

Requires Bazelisk, Clang/LLVM 19 (including clang-format, clang-tidy,
llvm-cov, and llvm-profdata), and Python 3.9+. Bazel resolves GoogleTest
through Bzlmod.

```sh
./scripts/check.sh
bazel run //:transfer
bazel run //:not_demo
bazel run //:or_demo
bazel run //:and_demo
bazel run //:constant_bit_demo
bazel run //:d_flip_flop_demo
```

Run `bazel run //:format` to format C++ sources.

The `verify` Actions job runs the same Bazel documentation, behavioral,
compile-fail, formatting, compatible static-analysis, and production coverage gates.

## Design record

- [Design](docs/design.md): thesis, atom floor, representation, and scope.
- [Timing](docs/timing.md): authoritative discrete-time execution contract.
- [Component contracts](docs/component-contracts.md): interface, type-family, acceptance, and regression discipline.
- [Roadmap](docs/roadmap.md): component dependencies, proofs, and checkpoints.
- [Decisions](docs/decisions.md): revised direction and withdrawn assumptions.
- [Baseline](docs/baseline.md): inherited IRATA2 and Rule Lab lessons.
- [Status](docs/status.md): what runs today and what comes next.
- [Agent guide](AGENTS.md): contribution workflow and verification rules.
