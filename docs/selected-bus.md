# Two-source selected bus

`components::SelectedBus<Width>` gives two word sources one shared output.
Bit index 0 is the least-significant bit. A low selector chooses source zero;
a high selector chooses source one. Both selector values are defined, so the
component has no idle state, invalid encoding, or undriven output. It models
ordinary mux selection and fan-out, not tri-state resolution.

The bus owns one `MuxWord<Width>` child and exposes its source boundaries,
selector, and output bundle. The selector therefore expands into one actual
`MuxBit` per data bit. It has no storage: observation immediately reflects the
currently bound source words. Consumers connect source output bundles to the
bus input boundaries and connect the bus output to a destination input.

`tests/selected_bus_test.cpp` exhausts both selector values and all pairs of
four-bit source words, checks the 43-component derived inventory, and routes
the chosen word into a parent output boundary. Run
`bazel run //:selected_bus_demo` for a four-bit boundary-to-output example.
This is the first group-D component; the register bank that consumes it
remains ahead.
