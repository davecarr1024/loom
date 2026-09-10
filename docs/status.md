# Current status

Loom is a design-only skeleton created on 2026-09-10. It has a named thesis,
architectural boundaries, a discrete-time contract, a phased learning roadmap,
source-grounded baseline notes, and a documentation verification command.

No machine or C++ interface has been implemented. There are no measured
simulation capabilities, benchmark results, or production coverage numbers.
The design's examples and proof scenarios are planned deliverables.

The next bite is Roadmap Phase 1: two explicitly connected registers that run
without a CPU. That bite must establish the actual C++23 toolchain, safe
definition/instance lifetimes, and a tested current-state/next-state boundary.
The larger destination remains an explainable pipelined register machine.

The project deliberately keeps graphics, electrical propagation, and multiple
clock domains outside its initial scope. Different operation latencies and
pipeline stalls remain within scope under one shared discrete clock.
