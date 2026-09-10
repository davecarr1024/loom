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

This is a **design-only skeleton**. There is no C++ library, simulator, CPU,
assembler, or claimed hardware proof yet. The first implementation bite is a
standalone register-transfer circuit. The larger destination is a small
register machine with a three-stage pipeline, forwarding, stalls, and branch
flushes, explained through deterministic textual traces.

## Check the skeleton

Requires Python 3.9+ and Make; no third-party packages are needed.

```sh
make check
```

This checks documentation structure and local links. It does not test a
machine. C++ build, behavioral tests, compile-fail tests, formatting, static
analysis, and coverage gates arrive with the first implementation phase.

## Design record

- [Design](docs/design.md): goals, architecture, representation, and boundaries.
- [Timing](docs/timing.md): authoritative discrete-time execution contract.
- [Roadmap](docs/roadmap.md): small deliverables, proofs, and checkpoint questions.
- [Decisions](docs/decisions.md): resolved questions and explicitly deferred choices.
- [Baseline](docs/baseline.md): lessons grounded in IRATA2 and Rule Lab source.
- [Status](docs/status.md): current evidence and next work.
- [Agent guide](AGENTS.md): contribution workflow and verification rules.

The design describes intended behavior, not implemented capabilities.
