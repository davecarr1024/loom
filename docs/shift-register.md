# Serial shift register

`components::ShiftRegister<Width>` moves the current word toward increasing bit
indices on an enabled edge. Old bit `i` moves to `i + 1`; the serial input
replaces bit 0, and the old most-significant bit is discarded. A disabled edge
holds the word. Bit 0 is least significant, and width must be positive.

The component owns an `EnabledWordRegister`. Its next-data bundle connects the
serial input to bit 0 and each current Q bit `i` to next-data bit `i + 1`. The
child's enable controls whether that shift word is loaded or the old word is
held. This behavior comes from the same mux and DFF children used by the
enabled register; the simulator has no shift-specific operation.

`enable_port()` and `serial_port()` are root input boundaries;
`enable_input()` and `serial_input()` allow a parent to drive them. The output
is an ordered word bundle. Initialization is explicit, each edge commits one
new word simultaneously, and retained edge evidence remains owned. Tests prove
two consecutive shifts, serial insertion, disabled hold, width one, the DFF
inventory, parent boundary output, and word order. Run
`bazel run //:shift_register_demo` for a repeated-shift trace.

This completes the composed register building blocks. Transfer and movement
regressions now use these components, and the [roadmap](roadmap.md) advances to
selected buses and bus-connected registers.
