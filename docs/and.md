# AND: the second construction component

AND extends the exact atom vocabulary used by the [NOT harness](not.md). It
owns two distinct one-bit inputs, `left()` and `right()`, and one output. The
output is their conjunction in the same pure observation. It has no state or
callback. Both dependencies are discovered from their owned port identities;
the scheduler runs AND only after both source nodes are ready.

`simulation::Definition` records each input endpoint separately, so connecting
two same-width sources cannot collapse their identities. Inventory and wire
paths include the AND atom and both connections. Observations expose
`<path>.left` and `<path>.right` values, plus `<path>.out`. The allowed behavior
remains closed: simulation recognizes the exact `components::And` type.

`tests/not_test.cpp` exhausts all four input rows, checks the derived schedule,
and builds a parent containing NOT and AND. The parent output is observed
through the same gates and wires used by the standalone truth table.
Run `bazel run //:and_demo` for the complete truth table through the same
definition and observation path.

The dependency engine represents zero, one, or multiple input endpoints
per node. Existing NOT, boundary, malformed-wiring, cycle, and Phase 1 register
tests continue to pass. [OR](or.md) extends the same multi-input model; a
constant bit and DFF now complete the initial atom floor. Gate-composed
[XOR](xor.md) and [one-bit mux](mux-bit.md) extend those atoms. Fixed-width
bundles are accepted; see [their contract and evidence](wire-bundles.md) and
[the roadmap](roadmap.md) for the next component.
