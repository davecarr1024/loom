# Baseline research

Inspected on 2026-09-10. This records source observations separately from Loom
proposals. Links pin the researched revisions; some repositories may require
the owner's GitHub access.

## IRATA2

Revision: `9a405fde0294862ce10ac61a16986433d2bb2e93`.

- [Design](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/docs/design.md):
  hardware-ish behavior, immutable HDL structure, microcode, simulator, and
  layered validation establish the architectural baseline.
- [Component base](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/sim/include/irata2/sim/component.h):
  runtime components expose a concrete Cpu and obtain phase through parents.
- [Register tests](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/sim/test/register_test.cpp)
  and [ALU tests](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/sim/test/alu_test.cpp):
  focused behaviors use full CPU fixtures; ALU cases manually select a phase.
- [CPU assembly](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/sim/src/cpu.cpp):
  central child registration and runtime control-path/order agreement checks
  demonstrate the cost of maintaining HDL and simulator structure separately.
- [Bus validator](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/microcode/src/compiler/bus_validator.cpp):
  Byte and Word distinguish data/address bus groups; this is not arbitrary
  bus-instance validation.
- [Control conflicts](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/microcode/src/compiler/control_conflict_validator.cpp):
  operation names determine conflicts. Loom instead proposes declared resource
  effects and instance identity.
- [ALU implementation](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/sim/src/alu/alu.cpp):
  arithmetic is directly computed in C++, with explicit registers and flag
  references. Hardware-ish does not mean transistor-level simulation.
- [Compiler pipeline](https://github.com/davecarr1024/irata2/blob/9a405fde0294862ce10ac61a16986433d2bb2e93/microcode/src/compiler/compiler.cpp):
  validators run after transformations/optimizers. Preserve this discipline.

IRATA2's fixed phases avoid arbitrary signal settling but do not themselves
give arbitrary composed state updates a simultaneous-edge interpretation.
Loom's timing contract is a deliberate new specification, not a claim about
IRATA's existing semantics.

The root README understates implementation progress; docs/plan.md records 153
instructions across 12 addressing modes and demo infrastructure. The existing
local build was tested with `ctest --test-dir build --output-on-failure -j 8`:
667/668 passed. `asteroids_main_test` observed 0xFE at a thrust check expecting
0xFF. This was not a fresh build or coverage measurement, and is not evidence
of a core architectural defect. No IRATA source was changed for this study.

## Rule Lab

Revision: `421cbe796f666e4263d57c6bb44e22d2d098a860`.

- [Typed graphs](https://github.com/davecarr1024/rule_lab/blob/421cbe796f666e4263d57c6bb44e22d2d098a860/docs/typed-rule-graphs.md):
  concrete objects own strong structure; references add graph relationships;
  finalization resolves and validates rather than inventing the model.
- [Structural adapters](https://github.com/davecarr1024/rule_lab/blob/421cbe796f666e4263d57c6bb44e22d2d098a860/include/rule_lab/core/structure.h):
  concepts and tuples of child references expose existing concrete values.
- [Introspection](https://github.com/davecarr1024/rule_lab/blob/421cbe796f666e4263d57c6bb44e22d2d098a860/include/rule_lab/core/introspection.h):
  type-only graph facts derive from the same structural descriptions.
- [Retrospective](https://github.com/davecarr1024/rule_lab/blob/421cbe796f666e4263d57c6bb44e22d2d098a860/docs/project-retrospective.md):
  derive requirements upward, preserve concrete examples, and close a lab when
  its thesis is answered.

Borrow the representation techniques, not the parser semantics. In particular,
semantic result type identifies canonical parser bindings; it cannot identify
hardware instances when several registers carry the same type. No dependency
on Rule Lab's parser library is planned, and its graph facts do not supply a
hardware scheduler or prove temporal behavior.
