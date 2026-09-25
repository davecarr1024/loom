# Current status

Loom now asks whether upward component construction can preserve local reasoning,
reproducible failures, and explanations as digital machines become more complex.
The [design](design.md), [component contracts](component-contracts.md), and
[roadmap](roadmap.md) define that direction. [NOT](not.md), [AND](and.md),
[OR](or.md), [constant bit](constant-bit.md), [DFF](d-flip-flop.md),
[gate-composed XOR](xor.md), [one-bit mux](mux-bit.md), [fixed-width wire
bundles](wire-bundles.md), [word mux](mux-word.md), a [two-to-four
decoder](decoder-2-to-4.md), and a [word register](word-register.md) are
implemented. An [enabled word register](enabled-word-register.md) is also
implemented, as is a [serial shift register](shift-register.md). The remaining
register milestone is to migrate the transfer baseline onto composed storage.

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
and new Q. `bazel run //:xor_demo` prints the XOR truth table from the derived
NOT/AND/OR circuit; `bazel run //:mux_bit_demo` prints all selector/data rows.
`bazel run //:mux_word_demo` shows one shared selector choosing a four-bit
input through four gate-composed bit muxes. The shared selection test exhausts
all 512 four-bit input and selector combinations and checks every bit-mux row.
`bazel run //:decoder2_to4_demo` prints every two-bit address and its one-hot
four-output decode. Tests verify all four addresses and both intermediate
inverted address bits through the actual NOT/AND inventory.
`bazel run //:word_register_demo` loads whole words over shared edges through
four initialized child DFFs. Parent tests connect input and output bundles and
verify the pre-edge initialized value and post-edge loaded value.
`bazel run //:enabled_word_register_demo` demonstrates load and hold. The
enabled register selects old Q or input data through its child word mux; tests
check the gate inventory, DFF commits, both control values, and parent wiring.
`bazel run //:shift_register_demo` shifts a four-bit word toward higher bit
indices and injects the serial bit at index 0. Tests cover repeated shifts,
hold, width one, and parent boundary wiring.
`bazel run //:wire_bundle_demo` routes four bits through an ordered bundle and
prints the resulting bit sequence.

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

Build a word mux and small decoder from accepted gates and the one-bit mux.
Use accepted bundle wiring to express their fixed-width interfaces, and preserve
all gate, constant, DFF, XOR, bit-mux, and register proofs.

The useful lesson from the baseline is that stable ownership and simultaneous
state transitions make small assemblies executable without a CPU. The new
question is whether a restricted floor and tested contract families can retain
that clarity through every subsequent construction level.
