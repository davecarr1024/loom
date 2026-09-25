# Four-bit ripple adder

`components::RippleAdder<Width>` adds two fixed-width words and a one-bit
carry-in. Bit 0 is least significant. It owns one `FullAdder` per bit; each
carry-out is wired to the next bit's carry-in, and the final carry-out remains
an explicit output. There is no word-level arithmetic shortcut or state.

`tests/ripple_adder_test.cpp` exhausts all 512 pairs of four-bit operands and
carry-in values. It checks every sum bit, each intermediate ripple carry, the
final carry, the derived gate inventory, and parent output wiring. A compile-fail
case rejects zero width. Run `bazel run //:ripple_adder_demo` for a four-bit
overflow trace. The [roadmap](roadmap.md) next adds increment and equality
components before the minimal ALU.
