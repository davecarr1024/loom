# Current status

Loom now asks whether upward component construction can preserve local reasoning,
reproducible failures, and explanations as digital machines become more complex.
The [design](design.md), [component contracts](component-contracts.md), and
[roadmap](roadmap.md) define that direction. [NOT](not.md), [AND](and.md),
[OR](or.md), [constant bit](constant-bit.md), and [DFF](d-flip-flop.md) are
implemented; the first selection components are next.

## What runs today

The new `simulation::Definition` observes circuits of `components::Not`,
`components::And`, `components::Or`, `components::ConstantBit`, and
`components::DFlipFlop`, plus explicit external input/output boundaries. A
separate `simulation::Simulation` owns evolving state per instance. All logic
executes through discovered components and connections. Inventory, wire paths,
schedule, and intermediate signal values explain the same execution. Observation
uses a complete call-scoped input snapshot; each shared edge samples all DFFs
from one pre-edge evaluation and commits them simultaneously. Invalid wiring,
cycles, duplicate ownership, and invalid bindings are rejected.

`tests/not_test.cpp` proves NOT, AND, and OR truth tables, constant zero and one,
source scheduling and parent composition, DFF initial values, sampling,
repeated edges, feedback, simultaneous commits, independent simulations, retained
evidence, and rejected-input atomicity. It also proves NOT/AND and NOT/OR
composition, nested independent inputs, fan-out, child/wire order independence,
empty snapshots, and malformed topology. Four compile-fail cases prove port
widths/roles, explicit Boolean values, and custom-atom rejection.
`bazel run //:not_demo` prints
the inventory and both rows of the two-NOT truth table with intermediate values.
`bazel run //:and_demo` and `bazel run //:or_demo` print the gates' exhaustive
truth tables; `bazel run //:constant_bit_demo` observes both constant values.
`bazel run //:d_flip_flop_demo` prints a four-edge toggle trace with old Q, D,
and new Q.

The retained Phase 1 register-only baseline provides typed registers and
role-specific width-safe connections, nested ownership discovery, compile-time
register/port counts, independent simulation state, and atomic snapshot/commit
edges. [Phase 1](phase-1.md) documents the interface and toolchain.

`tests/circuit_test.cpp` covers exhaustive four-bit transfer/hold/swap, independent
runs, child enumeration, nested same-width instances, invalid topology/names,
atomic rejected inputs, an edge budget, fan-out, and one-register-per-edge chain
movement. Static assertions check endpoint roles, widths, and structural facts.
`tests/wrong_width.cpp` must fail with the intended width-deduction diagnostic.

The executable `bazel run //:transfer` demonstrates the current boundary:

```text
edge 0 transfer.destination: 0 -> 42
edge 0 transfer.source: 42 -> 42
```

`./scripts/check.sh` is the shared Bazel/Clang 19 local/CI gate. It checks docs,
behavioral and negative compilation tests, formatting, compatible static
analysis, and 100% measured production line/function coverage. Structural
register facts use type-level constant expressions and are verified by
compile-time assertions. For any other non-executable expression LLVM reports
as a line, the checker honors generic `LCOV_EXCL_LINE` source annotations; it
contains no per-file or per-line exceptions.

## What is transitional or absent

Atomic wide registers and path-based enable inputs remain baseline mechanisms.
They are not the accepted long-term atom floor. The initial model has NOT, AND,
OR, a constant bit, and a one-bit DFF, with registers and larger components
executed through child circuits. Register-family concepts and typed control
families are planned, not implemented.

The unfinished old Phase 2 arithmetic/scheduler experiment was shelved because
its unrestricted primitives and atomic full adder did not satisfy the revised
construction premise. It had passed selected tests but not the complete gate.
It is not part of the supported build. The fixed-atom logic scheduler and first
stateful shared-edge simulation are implemented, but no ALU, memory, controller,
or CPU exists. Port-level hierarchical observation and per-DFF edge evidence are
available; instruction-level trace expansion remains future work.

## Next component and checkpoint

Build XOR and a one-bit mux through gate composition. Prove their truth tables,
selection semantics, and parent composition while preserving all gate, constant,
DFF, and register proofs.

The useful lesson from the baseline is that stable ownership and simultaneous
state transitions make small assemblies executable without a CPU. The new
question is whether a restricted floor and tested contract families can retain
that clarity through every subsequent construction level.
