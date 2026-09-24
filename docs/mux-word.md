# Fixed-width word mux

`components::MuxWord<Width>` selects one of two fixed-width inputs using one
shared one-bit selector. `Width` must be positive. Input and output bundle index
0 is the least-significant bit, matching the [wire-bundle contract](wire-bundles.md).
The `SelectionContract` concept records the shared width-parameterized shape of
the one-bit and word mux interfaces.

Each output bit is an owned `MuxBit`. The word mux has no selection behavior of
its own: it fans the selector and input bits into those children, and its output
bundle aliases their output ports. The derived execution plan therefore
contains the same NOT, AND, and OR gates as the one-bit mux for every bit.
Nested external-input boundaries route the parent's single selector and data
bundles into those bit-mux children. A top-level word mux therefore takes one
binding per selector and data bit, not one selector binding per child.

The shared selection test proves every one-bit row and all 512 combinations of
four-bit inputs and selector. A nested-boundary test verifies parent-driven
inputs cannot also be bound, and a parent test inverts selected bits through
ordinary NOT children. Run `bazel run //:mux_word_demo` for a four-bit trace.

This accepts fixed-width selection. The next group-B component is a small
one-hot decoder; see the [roadmap](roadmap.md).
