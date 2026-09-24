# XOR: the first gate-composed component

`components::Xor` exposes two independently bound one-bit inputs and one output.
It is a composite, not an allowed behavior atom. Its output is
`(left AND NOT right) OR (NOT left AND right)`, executed entirely by the
existing NOT, AND, and OR children. The simulator has no XOR dispatch branch.

The composite owns both input boundaries and all five gates. Its connections
and schedule are derived from those children, so observations expose the two
exclusive terms and the final OR output. A containing XNOR circuit consumes the
XOR output and a NOT through the same plan.

`tests/not_test.cpp` checks the derived child inventory, all four input rows,
the expected child schedule, eight actual wires, both product terms, and parent
composition. Run
`bazel run //:xor_demo` for the truth table through the same observation path.
The one-bit [mux](mux-bit.md) extends the same gates with selection semantics.
Fixed-width wire bundles are accepted; see [their contract and evidence](wire-bundles.md)
and the [roadmap](roadmap.md) for the next component.
