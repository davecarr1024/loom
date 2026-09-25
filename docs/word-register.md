# Word register

`components::WordRegister<Width>` stores one fixed-width word as `Width`
initialized D flip-flops. Bit 0 is the least-significant bit. The constructor
requires an explicit initial `Bit` for every position; width must be positive.

`data_ports()` exposes the scalar external input boundaries for top-level
bindings. `data_input()` and `output()` provide ordered bundles for parent
connections. A parent drives the register through those input boundaries; the
same nested boundary cannot also receive a separate external binding. Each DFF
samples its corresponding data bit on every simulator edge, with no enable or
composite behavior in the simulator.

Immediately after initialization, `output()` observes the per-bit initial Q
values. An edge samples the entire input word simultaneously. Its evidence
contains one commit per actual DFF and owns its pre-edge observation, which
later steps cannot overwrite. Tests verify all four stored bits, loading,
retained edge evidence, the DFF inventory, and data/output wiring in a parent.
Run `bazel run //:word_register_demo` for a short load trace.

This is the always-loading word contract. The next component adds an explicit
enable through word selection and register composition; see the
[roadmap](roadmap.md).
