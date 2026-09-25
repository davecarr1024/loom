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
implemented, as is a [serial shift register](shift-register.md). Register and
movement group C is complete; selected buses and register banks are next.

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
check the gate inventory, DFF commits, both control values, parent wiring, and
the constant-high adaptation to always-load behavior.
`bazel run //:shift_register_demo` shifts a four-bit word toward higher bit
indices and injects the serial bit at index 0. Tests cover repeated shifts,
hold, width one, and parent boundary wiring.
`bazel run //:wire_bundle_demo` routes four bits through an ordered bundle and
prints the resulting bit sequence.

The archived [Phase 1](phase-1.md) document records the removed wide-register
API and toolchain history. `tests/circuit_test.cpp` now exhausts all four-bit
transfer/hold/swap pairs and verifies independent simulations, enumeration
order, nested instances, atomic invalid-input handling, fan-out, and one-word-
per-edge chain movement through `EnabledWordRegister` children. The top-level
transfer example uses `WordRegister<8>` and `EnabledWordRegister<8>`.

`bazel run //:transfer` demonstrates an eight-bit transfer through an
always-loading source and an enabled destination, both composed from DFFs:

```text
edge 0 transfer.destination: 0 -> 42
edge 0 transfer.source: 42 -> 42
```

`./scripts/check.sh` is the shared Bazel/Clang 19 local/CI gate. It checks docs,
behavioral and negative compilation tests, formatting, compatible static
analysis, and 100% measured production line/function coverage. For any
non-executable expression LLVM reports as a line, the checker honors generic
`LCOV_EXCL_LINE` source annotations; it contains no per-file or per-line
exceptions.

## What is absent

The supported model has no wide atomic register or path-based enable adapter.
The initial model has NOT, AND, OR, a constant bit, and a one-bit DFF, with
registers and larger components executed through child circuits. Readable-word
and enabled-word contract shapes are implemented; bus and controller control
families will follow their concrete consumers.

The unfinished old Phase 2 arithmetic/scheduler experiment was shelved because
its unrestricted primitives and atomic full adder did not satisfy the revised
construction premise. It had passed selected tests but not the complete gate.
It is not part of the supported build. The fixed-atom logic scheduler and first
stateful shared-edge simulation are implemented, but no ALU, memory, controller,
or CPU exists. Port-level hierarchical observation and per-DFF edge evidence are
available; instruction-level trace expansion remains future work.

## Next component and checkpoint

Build a bounded selected bus from word muxes, then a register bank that can
send and receive through it. Define valid idle/selection semantics without
tri-state resolution and preserve all gate, selection, and register proofs.

The useful lesson from the baseline is that stable ownership and simultaneous
state transitions make small assemblies executable without a CPU. The new
question is whether a restricted floor and tested contract families can retain
that clarity through every subsequent construction level.
