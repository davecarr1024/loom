# Current status

Loom now asks whether upward component construction can preserve local reasoning,
reproducible failures, and explanations as digital machines become more complex.
The [design](design.md), [component contracts](component-contracts.md), and
[roadmap](roadmap.md) define that direction. [NOT](not.md) is the first
implemented component of the new floor; AND is next.

## What runs today

The new `simulation::Definition` observes circuits of `components::Not` and
explicit external input/output boundaries. All logic executes through discovered
gates and connections. The inventory, wire paths, schedule, and intermediate
signal values explain the same execution. Observation works immediately after
finalization, uses a complete call-scoped input snapshot, and owns its results.
Invalid wiring, cycles, duplicate ownership, and invalid bindings are rejected.

`tests/not_test.cpp` proves single- and two-gate behavior, nested independent
inputs, fan-out, child/wire order independence, evidence lifetime, empty snapshots,
and malformed topology. Four new compile-fail cases prove port widths/roles,
explicit Boolean values, and custom-atom rejection. `./build/not_demo` prints
the inventory and both rows of the two-NOT truth table with intermediate values.


The retained Phase 1 register-only baseline provides typed registers and
role-specific width-safe connections, nested ownership discovery, compile-time
register/port counts, independent simulation state, and atomic snapshot/commit
edges. [Phase 1](phase-1.md) documents the interface and toolchain.

`tests/circuit_test.cpp` covers exhaustive four-bit transfer/hold/swap, independent
runs, child enumeration, nested same-width instances, invalid topology/names,
atomic rejected inputs, an edge budget, fan-out, and one-register-per-edge chain
movement. Static assertions check endpoint roles, widths, and structural facts.
`tests/wrong_width.cpp` must fail with the intended width-deduction diagnostic.

The executable `./build/transfer` demonstrates the current boundary:

```text
edge 0 transfer.destination: 0 -> 42
edge 0 transfer.source: 42 -> 42
```

`make check` is the shared local/CI gate. It checks docs, behavioral and negative
compilation tests, formatting, compatible static analysis, and 100% measured
production line/function coverage. All 23 CTest checks pass with 254/254
measured production lines and 167/167 functions covered.
Clang-tidy 14 is explicitly skipped because
its frontend cannot parse `std::expected`; this is not a static-analysis pass.

## What is transitional or absent

Atomic wide registers and path-based enable inputs remain baseline mechanisms.
They are not the accepted long-term atom floor. The new model has NOT;
AND, OR, constants, and one-bit DFFs remain to be built, with registers and larger components
executed through child circuits. Register-family concepts and typed control
families are planned, not implemented.

The unfinished old Phase 2 arithmetic/scheduler experiment was shelved because
its unrestricted primitives and atomic full adder did not satisfy the revised
construction premise. It had passed selected tests but not the complete gate.
It is not part of the supported build. The new fixed-atom NOT scheduler is
implemented, but no DFF, ALU, memory, controller, or CPU exists. Port-level
hierarchical observation is available; state/edge and instruction-level trace
expansion remain future work. The stateless definition has no step operation.

## Next component and checkpoint

Build AND with two distinct typed bit inputs, one output, and derived dependency
analysis for both inputs. Prove its four-row truth table and a containing NOT/AND
circuit while preserving existing proofs. Accept each remaining atom individually;
stateful observation/edge equivalence belongs to the DFF acceptance gate.

The useful lesson from the baseline is that stable ownership and simultaneous
state transitions make small assemblies executable without a CPU. The new
question is whether a restricted floor and tested contract families can retain
that clarity through every subsequent construction level.
