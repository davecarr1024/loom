# Current status

Loom now asks whether upward component construction can preserve local reasoning,
reproducible failures, and explanations as digital machines become more complex.
The [design](design.md), [component contracts](component-contracts.md), and
[roadmap](roadmap.md) define that direction. This update is a planning reset,
not an implementation of the new atom floor.

## What runs today

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
production line/function coverage. For this planning update, all nine CTest
checks pass with 101/101 measured production lines and 63/63 functions covered.
Clang-tidy 14 is explicitly skipped because
its frontend cannot parse `std::expected`; this is not a static-analysis pass.

## What is transitional or absent

Atomic wide registers and path-based enable inputs remain baseline mechanisms.
They are not the accepted long-term atom floor. The new model will use NOT,
AND, OR, constants, and one-bit DFFs, with registers and larger components
executed through child circuits. Register-family concepts and typed control
families are planned, not implemented.

The unfinished old Phase 2 arithmetic/scheduler experiment was shelved because
its unrestricted primitives and atomic full adder did not satisfy the revised
construction premise. It had passed selected tests but not the complete gate.
It is not part of the supported build. There is currently no combinational
scheduler, external data-port harness, gate/DFF implementation, ALU, memory,
controller, CPU, or hierarchical trace expansion.

## Next component and checkpoint

Build NOT with typed external input/output bindings, pure observation, derived
inventory and scheduling, and a two-NOT containing circuit. Prove its truth
table, invalid topology rejection, and enumeration independence. Preserve the
register baseline until composed DFF registers can replace it with equivalent
behavioral proofs. Then continue upward one accepted component at a time.

The useful lesson from the baseline is that stable ownership and simultaneous
state transitions make small assemblies executable without a CPU. The new
question is whether a restricted floor and tested contract families can retain
that clarity through every subsequent construction level.
