# One-bit mux: the second selection component

`components::MuxBit` exposes separate `select`, `when_false`, and `when_true`
one-bit input boundaries plus one output. It chooses `when_false` when the
selector is zero and `when_true` when it is one. The component is a composite:
NOT computes `!select`, two AND gates pass the selected terms, and OR combines
them. The simulator has no mux behavior branch.

The derived inventory, seven connections, and schedule expose the selector
inversion and both gated data terms. `tests/not_test.cpp` checks all eight input
rows and uses the mux in a parent circuit with NOT. Run
`bazel run //:mux_bit_demo` for the exhaustive selection table.

This accepts the first selection pair. Fixed-width bundles of wires are next;
see the [roadmap](roadmap.md).
