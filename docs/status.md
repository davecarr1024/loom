# Current status

Loom's first Phase 1 circuit is implemented. Typed registers, connections,
nested ownership discovery, and simultaneous edge transitions run without a
CPU. See [Phase 1](phase-1.md) for the interface, evidence, and limitations.

Tests exhaust all four-bit pairs for transfer/hold/swap, verify independent
simulations, nested transfers, invalid topology, atomic rejected inputs, and
an edge budget. A negative compilation test rejects a width mismatch.
`make check` enforces measured production line/function coverage. Clang-tidy
14 is explicitly skipped because its frontend cannot parse std::expected.

The next bite tightens structural concepts, endpoint roles, and propagated
facts before completing Phase 1. No CPU or combinational scheduler exists yet.
The larger destination remains an explainable pipelined register machine.

The project deliberately keeps graphics, electrical propagation, and multiple
clock domains outside its initial scope. Different operation latencies and
pipeline stalls remain within scope under one shared discrete clock.
