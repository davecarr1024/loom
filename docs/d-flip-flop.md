# D flip-flop: the fifth construction component

`components::DFlipFlop` owns a one-bit D input, a one-bit Q output, and an
immutable initial value. Q exposes committed state during every combinational
evaluation. D is sampled at the shared simulator edge; there is no clock port,
enable, or asynchronous reset.

`simulation::Definition` still owns the immutable circuit. Its `observe` method
evaluates the circuit at each DFF's declared initial value, before edge 0.
Create a `simulation::Simulation` from the shared definition to own evolving
state. Each simulation initializes independently, accepts a complete input
snapshot for `observe` and `step`, and starts with edge index 0. Observation is
pure and transient. A successful step reports the pre-edge evaluation and one
owned commit record per DFF (old Q, sampled D, and new Q), then advances all Q
values together and increments the edge index. Invalid input leaves state and
edge index unchanged.

Finalization treats Q as a ready source and D as a sampled sink, so feedback
through a DFF breaks a combinational cycle. Its D input must still have exactly
one valid driver. The DFF does not appear in the combinational schedule; its
connected logic does. The simulator admits only the exact `DFlipFlop` type and
derives all state locations from owned circuit nodes.

`tests/not_test.cpp` checks both initial values before edge 0, observation versus
edge-0 equivalence, repeated sampling, retained evidence, independent
simulations, rejected-input atomicity, feedback, and simultaneous register
swaps. Run `bazel run //:d_flip_flop_demo` for a four-edge toggle trace through
the same inventory, dependency plan, and simulation API.

This accepts the initial atom floor. The next construction group begins with
gate-composed XOR and a one-bit mux; see the [roadmap](roadmap.md).
