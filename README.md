# Loom

A headless C++23 laboratory for building understandable digital machines from
small, typed circuits.

**Can increasingly complex machines preserve local reasoning, reproducible
failures, and explanations at every level?** Loom explores that question by
building upward: gates and flip-flops, registers and buses, arithmetic and
memory, controllers and successive CPUs.

The abstraction floor keeps evaluation simple: a small Boolean basis and
explicit one-bit storage, with acyclic combinational propagation between shared
clock edges. Larger components must execute as circuits of smaller components.
Hardware-ish modeling and constrained interfaces make state, timing, and valid
choices visible. Tests harden each component and its containing assemblies.

## Status

[The initial atom floor](docs/design.md), [XOR](docs/xor.md), the
[one-bit mux](docs/mux-bit.md), [fixed-width wire bundles](docs/wire-bundles.md),
the [word mux](docs/mux-word.md), a [two-to-four decoder](docs/decoder-2-to-4.md),
a [DFF-composed word register](docs/word-register.md), and an
[enabled word register](docs/enabled-word-register.md) are implemented. A
[shift register](docs/shift-register.md) composes these for serial movement.
The [two-source selected bus](docs/selected-bus.md) routes a chosen word through
the established mux network; a bus-connected register bank is next.
Circuits execute from owned
components and connections; stateful circuits use independent simulation state
with pure observations and atomic shared edges. See [DFF](docs/d-flip-flop.md)
for the state boundary and [the roadmap](docs/roadmap.md) for next components.

The former register-only Phase 1 engine has been retired. Transfer, hold, swap,
and pipeline behavior now runs through DFF-composed word registers. See
[roadmap](docs/roadmap.md) for construction order and [status](docs/status.md)
for current evidence and limitations.
See [AND](docs/and.md) and [OR](docs/or.md) for their two-input contracts and
evidence.

## Build and verify

Requires Bazelisk, Clang/LLVM 19 (including clang-format, clang-tidy,
llvm-cov, and llvm-profdata), and Python 3.9+. Bazel resolves GoogleTest
through Bzlmod.

For a clean, reproducible Linux environment with the pinned toolchain, install
Docker Engine and the Docker Compose plugin, then run:

```sh
LOOM_UID="$(id -u)" LOOM_GID="$(id -g)" \
  docker compose run --build --rm check
```

This builds the Ubuntu 24.04 tool image, mounts the checkout, keeps Bazel's
download cache in a named volume, and runs the same `scripts/check.sh` used by
CI. The first run downloads Bazel 8.4.2 (from `.bazelversion`) and dependencies;
later runs reuse both caches. The image supports x86_64 and ARM64. No project
source is copied into the image; the check may create Bazel's ignored build
outputs in the checkout.

To use the tools directly on Ubuntu 24.04, the CI workflow's package install
commands are the supported native setup.

```sh
./scripts/check.sh
bazel run //:transfer
bazel run //:not_demo
bazel run //:or_demo
bazel run //:and_demo
bazel run //:constant_bit_demo
bazel run //:d_flip_flop_demo
bazel run //:xor_demo
bazel run //:mux_bit_demo
bazel run //:mux_word_demo
bazel run //:decoder2_to4_demo
bazel run //:word_register_demo
bazel run //:enabled_word_register_demo
bazel run //:shift_register_demo
bazel run //:wire_bundle_demo
bazel run //:selected_bus_demo
```

Run `bazel run //:format` to format C++ sources.

The `verify` Actions job runs the same Bazel documentation, behavioral,
compile-fail, formatting, compatible static-analysis, and production coverage gates.

## Design record

- [Design](docs/design.md): thesis, atom floor, representation, and scope.
- [Timing](docs/timing.md): authoritative discrete-time execution contract.
- [Component contracts](docs/component-contracts.md): interface, type-family, acceptance, and regression discipline.
- [Wire bundles](docs/wire-bundles.md): fixed-width, ordered aliases for scalar ports.
- [Word mux](docs/mux-word.md): width-parameterized selection composed from one-bit muxes.
- [Two-to-four decoder](docs/decoder-2-to-4.md): one-hot address decoding composed from gates.
- [Word register](docs/word-register.md): fixed-width edge-loaded storage composed from initialized DFFs.
- [Enabled word register](docs/enabled-word-register.md): synchronous load and hold through mux/DFF composition.
- [Shift register](docs/shift-register.md): enabled serial movement through word-register and mux composition.
- [Selected bus](docs/selected-bus.md): two-source word selection composed from the accepted word mux.
- [Roadmap](docs/roadmap.md): component dependencies, proofs, and checkpoints.
- [Decisions](docs/decisions.md): revised direction and withdrawn assumptions.
- [Baseline](docs/baseline.md): inherited IRATA2 and Rule Lab lessons.
- [Status](docs/status.md): what runs today and what comes next.
- [Agent guide](AGENTS.md): contribution workflow and verification rules.
