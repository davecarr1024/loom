# Loom agent guide

Read [README.md](README.md), [design](docs/design.md), [timing](docs/timing.md),
[roadmap](docs/roadmap.md), and [status](docs/status.md) before implementation.
Consult [decisions](docs/decisions.md) for settled boundaries and
[baseline](docs/baseline.md) for inherited lessons. Read
[component contracts](docs/component-contracts.md) before defining a component.

## Current gate

The old register-only Phase 1 implementation has been retired. The revised
construction plan has accepted NOT, AND, OR, constant bit, initialized DFF,
gate-composed XOR, one-bit mux, fixed-width wire bundles, a word mux,
two-to-four decoder, a DFF-composed word register, an enabled word register,
and a shift register. Register/movement group C is complete. Group D's
two-source selected bus and two-register bank are accepted. The half adder and
full adder are accepted group-E components; the small ripple adder is next.
Read the relevant contract under `docs/` before extending a component.
Do not resume the shelved arithmetic prototype.
`./scripts/check.sh` is the presubmit command. It runs `bazel test //...`,
then Bazel LCOV coverage and the 100% production line/function gate.
`bazel test //...` runs behavioral, docs, compile-fail, formatting, and
static-analysis checks. Run `bazel run //:format` to apply C++ formatting. Use
`bazel run //:not_demo`, `bazel run //:and_demo`, `bazel run //:or_demo`, and
`bazel run //:constant_bit_demo`, `bazel run //:d_flip_flop_demo`,
`bazel run //:xor_demo`, `bazel run //:mux_bit_demo`,
`bazel run //:mux_word_demo`,
`bazel run //:decoder2_to4_demo`, `bazel run //:word_register_demo`,
`bazel run //:enabled_word_register_demo`,
`bazel run //:shift_register_demo`,
`bazel run //:wire_bundle_demo`, `bazel run //:selected_bus_demo`,
`bazel run //:register_bank_2_demo`, `bazel run //:half_adder_demo`,
`bazel run //:full_adder_demo`, and
`bazel run //:transfer` to run the examples. The workspace pins Clang 19 through
`.bazelrc`; install Bazelisk and Clang/LLVM 19, including clang-format,
clang-tidy, llvm-cov, and llvm-profdata, first.

Read [Phase 1](docs/phase-1.md) for historical API and toolchain notes. Clang-tidy
below 16 is explicitly skipped for std::expected frontend incompatibility;
do not describe that as a pass. Follow the roadmap one component at a time.

## Invariants

- Upward component construction is the development framework. Define a useful
  code contract, prove the component alone and in a parent, then build above it.
- Restrict behavior atoms to NOT, AND, OR, constant bit, and initialized DFF.
  Larger components execute children and wires, never composite callbacks.
- Separate containment from contract families. Use concepts/traits and explicit
  adapters; claimed specialization must preserve meaning, timing, and valid-use
  rules, with shared contract tests. Do not force an inheritance hierarchy.
- Preserve revealing system regressions and reduce failures to their true leaf
  or integration boundary. Rerun affected parents after every repair.
- Derive hierarchical explanations from the actual execution and port paths;
  do not maintain a separate behavioral story or substitute fast host models.
- Preserve the hardware-ish boundary: pure combinational functions, explicit
  storage, discrete time, and visible control. No analog or gate-delay model.
- Components must run inside a small assembly without a CPU root.
- Concrete immutable definitions own structure; derive traversal and facts
  from them. Never maintain a separate hand-authored simulator hierarchy.
- Connections identify instances and ports, not merely payload types.
- State reads and commits obey docs/timing.md; C++ visitation order must not
  change machine behavior.
- Propagated traits describe structural facts or declared contracts. Tests
  must substantiate behavioral promises.
- Keep error results and traces structured, deterministic, and readable.
- Prefer concepts, traits, const values, and std::expected over deep
  inheritance. Explain non-obvious invariants near their implementation.
- Keep dependencies acyclic, with matching module directories and namespaces.
- Aim for 100% production line/function coverage plus meaningful temporal and
  negative tests. Document narrow exceptions; never silently weaken gates.

## Delivery

Update affected docs with behavior and phase changes. Every push requires
local checks, a fresh-context self review, and independent `agy --mode plan`
code-and-design review of the final intended files and relevant design.
Use `--sandbox` when supported. The review prompt must permit file-reading
tools only and prohibit terminal, shell, Git, test, and file-editing commands.
Never use `--dangerously-skip-permissions`.

Apply obvious findings and repeat checks/review after material fixes. An
explicit quota/usage-limit failure permits a recorded `quota-skipped` review
after full local checks and self review. Other incomplete/unavailable reviews
block pushing unless Dave explicitly waives them. Surface genuine tradeoffs.

Use `/home/davecarr1024/projects/davecarr1024` for durable cross-project
guidance. Promote reusable lessons there without changing unrelated work.
