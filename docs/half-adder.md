# Half adder

`components::HalfAdder` takes two one-bit operands and exposes a one-bit sum
and carry. Both outputs are combinational in the same observation. The sum is
the accepted `Xor` component; carry is the accepted `And` component. It owns
only those children and routes both operands to each, with no storage or hidden
arithmetic evaluator.

`tests/half_adder_test.cpp` exhausts all four operand rows, checks the derived
inventory (two NOT, three AND, one OR), and connects both outputs to parent
boundaries. Run `bazel run //:half_adder_demo` for the truth table. The next
arithmetic component is a full adder built from half adders and gate-level
carry composition; see the [roadmap](roadmap.md).
