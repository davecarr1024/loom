# Loom

A headless C++23 laboratory for building understandable digital machines from
small, typed components.

Loom asks: **can I understand an overlapped, pipelined processor as clearly as
I understand my 6502-style machine?** Its name describes the work: weaving
components and connections into a machine whose behavior can be followed.

The project combines IRATA2's **hardware-ish** philosophy with Rule Lab's
concrete typed composition, propagated traits, and structural inspection.
Combinational functions compute results; explicit registers retain state;
discrete clock edges define when state changes. Every larger assembly should
be independently testable before it becomes part of a processor.

## Status

Phase 1 is underway: a C++ register-transfer circuit now runs independently
of a CPU, with typed connections and snapshot/commit edges. No CPU or assembler
exists yet. The larger destination is a small
register machine with a three-stage pipeline, forwarding, stalls, and branch
flushes, explained through deterministic textual traces.

## Build and verify

Requires GCC 12/C++23, CMake, Make, Python 3.9+, lcov, clang-format, and
clang-tidy. CMake fetches GoogleTest v1.14.0. See [Phase 1](docs/phase-1.md)
for verified tool versions, the clang-tidy compatibility exception, and offline setup.

```sh
make check
./build/transfer
```

This runs documentation checks, behavioral and compile-fail tests, formatting,
compatible static analysis, and production line/function coverage enforcement.

## Design record

- [Design](docs/design.md): goals, architecture, representation, and boundaries.
- [Timing](docs/timing.md): authoritative discrete-time execution contract.
- [Roadmap](docs/roadmap.md): small deliverables, proofs, and checkpoint questions.
- [Decisions](docs/decisions.md): resolved questions and explicitly deferred choices.
- [Baseline](docs/baseline.md): lessons grounded in IRATA2 and Rule Lab source.
- [Status](docs/status.md): current evidence and next work.
- [Agent guide](AGENTS.md): contribution workflow and verification rules.

Status and Phase 1 notes distinguish implemented behavior from the larger design.
