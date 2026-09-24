# Current status

Loom now asks whether upward component construction can preserve local reasoning,
reproducible failures, and explanations as digital machines become more complex.
The [design](design.md), [component contracts](component-contracts.md), and
[roadmap](roadmap.md) define that direction. [NOT](not.md), [AND](and.md), and
[OR](or.md) are the first implemented components of the new floor; a constant
bit is next.

## What runs today

The new `simulation::Definition` observes circuits of `components::Not`,
`components::And`, `components::Or`, and explicit external input/output boundaries. All logic executes through discovered
gates and connections. The inventory, wire paths, schedule, and intermediate
signal values explain the same execution. Observation works immediately after
finalization, uses a complete call-scoped input snapshot, and owns its results.
Invalid wiring, cycles, duplicate ownership, and invalid bindings are rejected.

`tests/not_test.cpp` proves NOT, AND, and OR truth tables, NOT/AND and NOT/OR
composition, nested independent inputs, fan-out, child/wire order independence,
evidence lifetime, empty snapshots,
and malformed topology. Four new compile-fail cases prove port widths/roles,
explicit Boolean values, and custom-atom rejection. `bazel run //:not_demo` prints
the inventory and both rows of the two-NOT truth table with intermediate values.
`bazel run //:and_demo` and `bazel run //:or_demo` print the gates' exhaustive
truth tables.


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
They are not the accepted long-term atom floor. The new model has NOT, AND, and
OR; constants and one-bit DFFs remain to be built, with registers and larger components
executed through child circuits. Register-family concepts and typed control
families are planned, not implemented.

The unfinished old Phase 2 arithmetic/scheduler experiment was shelved because
its unrestricted primitives and atomic full adder did not satisfy the revised
construction premise. It had passed selected tests but not the complete gate.
It is not part of the supported build. The new fixed-atom logic scheduler is
implemented, but no DFF, ALU, memory, controller, or CPU exists. Port-level
hierarchical observation is available; state/edge and instruction-level trace
expansion remain future work. The stateless definition has no step operation.

## Next component and checkpoint

Build a constant bit with an immutable explicit value and no input ports. Prove
both values, its source scheduling behavior, and parent composition while
preserving the gate and register proofs. Accept each remaining atom individually;
stateful observation/edge equivalence belongs to the DFF acceptance gate.

The useful lesson from the baseline is that stable ownership and simultaneous
state transitions make small assemblies executable without a CPU. The new
question is whether a restricted floor and tested contract families can retain
that clarity through every subsequent construction level.
