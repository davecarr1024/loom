# Enabled word register

`components::EnabledWordRegister<Width>` retains an initialized word while its
enable is low and loads the input word on an edge while enable is high. It is
composed from a `MuxWord<Width>` feeding a `WordRegister<Width>`: the false mux
input is the current Q bundle, the true input is the requested data, and enable
is the shared selector. The simulator has no enabled-register behavior branch.

Bit 0 is least significant. `enable_port()` and `data_ports()` expose the
unconnected input boundaries for root bindings. `enable_input()` and
`data_input()` let a parent drive those same boundaries through typed scalar
and bundle connections. `output()` satisfies the shared `ReadableWord` shape;
`EnabledWord` adds the enable and data input requirements. A parent-driven nested
boundary cannot receive a second binding.

Initialization is visible from Q before edge 0. Every edge commits every DFF;
when disabled, the mux routes old Q to D, and when enabled it routes the input
word. Tests check load and hold data, all child DFFs and mux gates in the
inventory, parent wiring and observation, and the driven-input diagnostic. Run
`bazel run //:enabled_word_register_demo` for a load/hold trace.

The [shift register](shift-register.md) uses this load/hold component to move a
word one position toward the most-significant bit while accepting serial input
at bit 0. See the [roadmap](roadmap.md).
