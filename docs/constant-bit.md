# Constant bit: the fourth construction component

`components::ConstantBit` owns an immutable explicit `value::Bit`, has no input
ports, and exposes one output. It is a pure source: it carries no evolving
state and is ready before the combinational schedule begins. Observation seeds
the source value from the discovered definition node, so all consumers and
signal evidence use the same execution plan.

The component is admitted as an exact atom type. Its inventory contains the
constant; its output appears in observations; it has no schedule entry because
inputless sources are already ready. `tests/not_test.cpp` checks both values,
empty schedule behavior, and composition with an external input and AND gate.
Run `bazel run //:constant_bit_demo` to observe both fixed values.

The immutable value belongs to the definition, while each observation still
owns its output evidence. The [initialized one-bit DFF](d-flip-flop.md) extends
the same plan with simulation-owned state and shared-edge commits. Gate-composed
selection is next; see the [roadmap](roadmap.md).
