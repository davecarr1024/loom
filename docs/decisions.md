# Decisions and open questions

These decisions set the initial direction. Changes need a concrete scenario,
an explanation of what the previous choice taught, and synchronized design
and timing documentation. No public C++ API is frozen by this document.

## Settled for the initial implementation

| Question | Decision and reason |
| --- | --- |
| Name? | Loom: typed parts woven into an understandable machine. |
| Primary product? | Headless learning artifacts and tests, culminating in an explainable pipeline. |
| Compatibility with IRATA? | None required; preserve its philosophy and lessons, not its ISA or phase API. |
| Language? | C++23, using concepts, traits, concrete values, and explicit error results. |
| Gate simulation? | Pure logical composition is supported; propagation delay and analog behavior are excluded. |
| Timing? | One discrete clock, current-state reads, simultaneous commit, explicit local latency. |
| Combinational cycles? | Reject during finalization; feedback requires explicit state. |
| Structure? | Concrete ownership plus typed connection references; derived analyses, no parallel hand-authored hierarchy. |
| Identity? | Component instance and port identity; equal payload types do not identify equal endpoints. |
| Mutation? | Immutable finalized definitions; simulation-owned mutable state with atomic public step semantics. |
| Static versus dynamic checks? | Compile-time for type-visible facts, finalization for value topology, execution for protocol/behavior evidence. |
| Initial values? | Explicit reset values, no X/Z or partially initialized machine. |
| Shared buses? | Explicit selection/arbitration with errors for invalid driver selection. |
| Memory collision? | Current-state read, edge-committed write; initially one write port. |
| Trait trust? | Structural facts are derived; behavioral declarations still require tests. |
| Microcode? | Typed control words, micro-PC, explicit transitions; no inherited fixed key shape. |
| ISA initially? | Structured values for a small 16-bit, eight-register machine; no assembler prerequisite. |
| Multiple clocks? | Deferred; unequal latency and stalls are modeled within one clock first. |
| Errors? | Structured expected results for normal invalid construction/execution; no silent repair. |
| License? | No license granted by this skeleton; choose an explicit license before inviting reuse. |

## Questions deliberately left to their proof phase

- **Phase 1: exact handles and state layout.** Compare scoped typed tags with
  typed member-path handles using the nested transfer example. Choose the
  smallest design that distinguishes repeated instances, catches wrong-root
  bindings, and keeps finalized lifetimes safe. Avoid pointer fixup machinery
  or generated schemas without a demonstrated need.
- **Phase 1: value width range.** Start with unsigned widths 1 through 64 if
  standard integer storage suffices. Define width-64 masks and oversized shifts
  explicitly; add arbitrary-width arithmetic only when a machine needs it.
- **Phase 1: compiler and coverage tools.** Pin versions proven locally and in
  CI for C++23 and std::expected. Coverage instrumentation for templates must
  be checked on actual instantiated production paths.
- **Phase 2: dependency granularity.** Whole-component dependency is the
  conservative default. Introduce output-specific dependencies only if valid
  compositions are otherwise rejected.
- **Phase 3: encoding and control conflicts.** Derive resource effects and
  selected-driver checks from actual ports and state. Set control-store limits
  explicitly for the concrete datapath rather than creating unlimited storage.
- **Phase 5: instruction encoding and addressing.** A phase design must fix
  branch targets, memory size/alignment, and fault semantics before CPU code.
  Binary encoding and a text assembler are optional later artifacts.
- **Phase 6: stage boundaries and simultaneous events.** Specify the complete
  priority table before implementation. This cannot safely be guessed by an
  implementation agent while editing tick code.
- **Phase 7: fairness and timeouts.** Bounded scripted memory responses make
  progress testable. An execution step limit reports an unfinished run; it
  does not prove the hardware has deadlocked.

## Risks and early warning signs

Generality is drifting if a new mechanism cannot be justified by a named
circuit. The representation is drifting if adding a child requires registering
it again at the root. Timing is drifting if renaming or reordering children
changes values. Hardware-ish boundaries are drifting if hidden callback state
affects execution. Testing is drifting if local component tests need a CPU.

Template complexity is a real cost: a small public example and a useful
negative diagnostic are phase deliverables, not optional polish. A broad trait
framework must not displace the concrete circuit that gives its facts meaning.
