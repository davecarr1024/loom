# Decisions and open questions

## Construction-plan revision: 2026-09-11

The central question is now whether upward circuit construction can preserve
local reasoning, stable tested interfaces, and understandable behavior as
machines grow. Component acceptance and hardening replace vertical slices as
the development framework. CPU examples exercise that vocabulary.

The earlier plan targeted a 16-bit, eight-register machine and a three-stage
pipeline as its defining destination. Those fixed choices are withdrawn.
An IRATA-style accumulator CPU now closes the initial construction proof;
register and pipelined machines follow as distinct learning questions.

The Phase 1 implementation established stable owned definitions, independent
simulation state, typed width-safe wiring, and snapshot/commit behavior. Retain
that tested baseline during migration, explicitly marking its atomic wide
register and path-enabled stepping as temporary. Port roles, structural counts,
chain/fan-out proofs, and name validation remain useful baseline improvements.

The unfinished old Phase 2 experiment showed the usefulness of derived
combinational planning, but permitted arbitrary stateless primitives and treated
a full adder as an atom. It had not passed the full gate. It is shelved rather
than published as the new foundation. Reintroduce scheduling through the first
NOT circuit under the restricted atom policy; build the full adder from gates.
The temporary experiment was saved outside the repository before removal.

## Settled boundaries

| Question | Decision and reason |
| --- | --- |
| Fundamental object? | A circuit with a purposeful, tested code interface; upward component construction is the development unit. |
| Language and representation? | C++23 concrete immutable definitions, concepts, traits, typed values, and explicit error results. |
| Atom floor? | NOT, AND, OR, constant bit, and initialized one-bit D flip-flop; see design for exact contracts. |
| Stateful feedback? | DFFs break same-edge dependencies; all state commits together. Gate-built latches and settling loops are excluded. |
| Wider components? | Registers, arithmetic, buses, memory, and controllers execute their children and connections. No composite behavior callback. |
| New atoms? | Require a concrete inadequacy of composition, a documented boundary decision, and behavioral proof. Convenience is insufficient. |
| Wiring adapters? | Aliases, fixed bundles/slices, and fan-out preserve bit identity; they perform no arithmetic or selection. |
| Clock and reset? | One simulator-wide edge and whole-assembly initialization. Dynamic synchronous reset is a composed selection function. |
| Invalid states? | Prevent statically where possible; validate actual topology at finalization and value/history obligations before edge commit. |
| Ownership? | Finite concrete tree, stable lifetime, instance/port identity, shared const definition, independent simulation storage. |
| Analysis? | Derive inventory, facts, and scheduling from the same owned structure. Traits describe facts/contracts; tests prove behavior. |
| Type relationships? | Separate containment from contract families and explicit refinement; substitutability includes timing and valid-use obligations. |
| Shared buses? | Composed selection with a bounded interface; no electrical resolution or implicit retention. |
| Memory? | Initially small power-of-two arrays of composed registers, combinational reads, one edge-committed write port. |
| Evidence? | Hierarchical views of the same execution with child/port provenance; no second behavioral implementation generating explanations. |
| First CPU? | A small accumulator machine with visible control; exact ISA and widths are decided after its dependencies are tested. |
| Compatibility? | No IRATA ISA/API requirement. Borrow the philosophy and lessons, not simulator phases or inheritance architecture. |
| Errors? | Structured deterministic expected results; rejected edges preserve committed state and cycle. |
| Delivery? | Full local gates, fresh-context self review, and read-only agy review at every push; quota exception remains explicit. |

## Decisions deferred until their component needs them

- Exact C++ interfaces for bits, bundles, endpoint aliases, and typed controls:
  start with the NOT harness; retain proven instance identity and lifetime
  requirements without committing to pointer fixups or a universal schema.
- Contract-family vocabulary: introduce a concept or refinement when concrete
  consumers need it; prove semantic substitution with shared contract tests and
  containing circuits. Do not infer substitutability from matching widths alone.
- Register widths and memory sizes: start small enough for exhaustive proofs;
  define bit order, address domain, and every supported control encoding first.
- Control taxonomy and microcode encoding: derive resource identities and effects
  from real interfaces. Typed records/variants may express register load, bus
  selection, and sequencer transitions without procedural device callbacks.
- CPU ISA, memory layout, halt/fault semantics, and later pipeline partitions:
  require component-level design checkpoints. No fixed pipeline stage count yet.
- Optimization: no circuit substitution or host-memory shortcut is approved.
  Faster scheduling must preserve evaluated structure and observable provenance.
- License: no license granted; choose explicitly before inviting reuse.

## Warning signs

A component is drifting if it hides state, executes a composite callback,
requires a CPU to test it, or needs a second hand-maintained hierarchy. A type
family is drifting if it groups names without giving consumers a useful contract.
A specialization is unsound if it changes promised timing or strengthens input
obligations while claiming interchangeability. A roadmap is drifting if a CPU
demo replaces missing component proofs. A trace is drifting if its explanation
recomputes behavior instead of referencing actual execution evidence.
