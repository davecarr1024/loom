# OR: the third construction component

OR extends the exact atom vocabulary used by the [NOT harness](not.md). It
owns two distinct one-bit inputs, `left()` and `right()`, and one output. The
output is their disjunction in the same pure observation. It has no state,
clock, latency, or callback. Both dependencies are discovered from their
owned port identities, and the scheduler runs OR only after both sources are
ready.

`simulation::Definition` retains each input endpoint separately. Inventory,
connections, and schedule include the OR atom and both dependencies;
observations expose `<path>.left`, `<path>.right`, and `<path>.out`. Simulation
admits this exact `components::Or` type, keeping composite behavior closed to
the documented atom set.

`tests/not_test.cpp` checks all four input rows, endpoint provenance, and the
derived schedule. A containing circuit composes NOT into OR and checks the
parent output over all input pairs. Run `bazel run //:or_demo` for the complete
truth table through the same definition and observation path.

The existing multi-input dependency representation serves both AND and OR.
Their local semantics remain explicit and separate in the exact atom dispatch.
The [constant bit](constant-bit.md) and [DFF](d-flip-flop.md) complete the
initial atom floor. [XOR](xor.md) is the first gate-composed component; a
one-bit mux follows under [the roadmap](roadmap.md).
