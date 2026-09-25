# Full adder

`components::FullAdder` adds two one-bit operands and a carry-in. It exposes
sum and carry-out in the same observation. The first owned `HalfAdder` adds the
operands; the second adds that sum to carry-in; an owned OR gate combines the
two carry outputs. The resulting equations follow from child execution, with
no arithmetic behavior in the simulator.

`tests/full_adder_test.cpp` exhausts all eight input combinations, checks the
derived gate inventory (four NOT, six AND, three OR), and connects both outputs
through parent boundaries. Run `bazel run //:full_adder_demo` for the truth
table. The next step is a fixed-width ripple adder that composes these full
adders; see the [roadmap](roadmap.md).
