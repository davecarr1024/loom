# Loom agent guide

Read README.md, docs/design.md, docs/timing.md, docs/roadmap.md, and
docs/status.md before implementation. Consult docs/decisions.md for settled
boundaries and docs/baseline.md for inherited lessons.

## Current gate

Design-only skeleton. `make check` is the current presubmit command. There
is no production C++ code to cover. Do not report documentation checks as
hardware verification or invent passing test/coverage commands.

The first code phase must add CMake, C++23, GoogleTest/CTest, compile-fail
checks, clang-format, compatible clang-tidy, coverage, and CI gates. Document
exact commands and tool versions then. Follow the roadmap one bite at a time.

## Invariants

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
