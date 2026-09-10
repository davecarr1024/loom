# Loom design

## Thesis and major goals

Loom is a learning project. The desired outcome is the ability to draw and
explain a pipelined register machine, including why instructions advance,
wait, forward results, or disappear after a branch.

The system must make that learning gradual:

1. Build larger components from smaller components with visible boundaries.
2. Execute and test any closed assembly without constructing a CPU.
3. Describe different datapaths using concrete C++ types and values.
4. Derive structural inspection and validation from those definitions.
5. Specify time precisely while retaining a manageable hardware-ish model.
6. Explain behavior through deterministic state, events, and textual traces.
7. Prove each increment before proceeding to the next architectural idea.

Loom is a short proper-noun project name: components are woven into a machine.
It is a new experiment informed by IRATA2 and Rule Lab, not an IRATA rewrite
or a compatibility release. Existing projects remain independent baselines.

## Hardware-ish principle

Model hardware mechanisms at the level that explains the machine. Compute
combinational results directly, represent storage explicitly, and define
exactly when state changes. Build larger machines through typed composition,
with inspection and validation derived from their structure.

A full adder may be composed from Boolean functions; a wide adder may compose
smaller adders. This describes logical behavior without simulating transistors,
electrical settling, propagation delays, or waveforms. Arithmetic primitives
are acceptable when their boundary is declared and independently proved.

No persistent value may hide inside a combinational callable or control
handler. Memory, pending requests, pipeline validity, counters, and controller
state must appear in the assembly's declared storage.

## Representation

Use C++23 concepts, traits, constrained functions, and typed value composition.
Component definitions are immutable after construction/finalization. Runtime
state is separate and belongs to a simulation instance, allowing multiple
independent runs of the same definition.

The conceptual vocabulary is intentionally small:

- **Value:** fixed-width unsigned bits with explicit arithmetic semantics.
- **Port:** a typed input or output on a particular component instance.
- **Connection:** an explicit directed binding between compatible ports.
- **Component:** combinational behavior, declared state transition, or a
  composite owning smaller components.
- **Assembly:** a closed root with exposed external ports, usable at any scale.
- **Simulation:** state, cycle index, inputs, and deterministic execution.

These are design concepts, not final C++ class names or an inheritance tree.
An ALU must not require a Processor base class, global registry, or singleton.

### Ownership, connectivity, and identity

Concrete objects form a finite ownership tree. Connections add references
between ports and may cross sibling boundaries through explicit interfaces.
Ownership determines lifetime; connectivity determines data dependence.

Two identically typed registers are distinct instances. Use typed hierarchical
handles or scoped tags to identify each endpoint. Payload type alone cannot
identify it. Human-readable paths are diagnostics, not the semantic meaning
of operations. Cross-assembly references and dangling targets are invalid.

Finalization discovers existing owned objects, validates connections, and
derives an execution plan; it does not invent a second authoritative model.
The finalized definition must own stable storage for bound references and be
non-copyable/non-movable unless relocation is explicitly made safe. Simulations
retain shared ownership of the const definition so independent simulations
can safely use the same instance. Exact handles are proved in Phase 1.

### Propagated facts

Following Rule Lab, structural adapters expose child references from actual
objects. Independent analyses aggregate facts such as ports, state elements,
connections, combinational dependencies, clock membership, control effects,
and diagnostic identity.

Each property defines its combination law. Counts add; owned paths must be
unique; connection sets preserve endpoint identity; mutually exclusive effects
retain their resource identities. Child traversal is not a timing schedule.

Type-visible properties can be checked at compile time. Value-dependent
topology is checked during finalization. Do not call a declared latency a
proof of that latency: behavior tests must establish it. Do not import a
universal property framework before two concrete analyses need common code.

### Primitive and composite contracts

Combinational primitives declare all inputs and outputs and are pure and
total for valid input values. Clocked primitives declare all storage and
compute next state from the current snapshot. Composite implementations
expose their children and connections; they do not secretly reimplement them
in a monolithic execution function.

Use one declared driver per ordinary input. Shared buses are explicit
arbitrated/multiplexed components, with selected-driver validity checks;
multiple driver connections never imply an electrical resolution rule.
Reading an undriven bus is an error, not a retained previous value.

## Timing and communication

[Timing](timing.md) owns the detailed contract. Initially there is one shared
clock, simultaneous state commit, and acyclic combinational dependencies.
Different latency does not imply different clocks. A multi-cycle unit retains
explicit work state; a pipeline retains stage registers and validity.

No independently advancing nested simulators are allowed inside one assembly.
The root coordinates clock edges while modules own their local transitions.
Externally scripted inputs are indexed by cycle, never wall-clock time.

## Architecture and dependencies

Planned modules, introduced only when their first proof requires them:

| Module | Responsibility | Dependencies |
| --- | --- | --- |
| `value` | widths, bit operations, explicit signed interpretation | standard library |
| `structure` | concepts, ports, composition, structural facts, finalization | value |
| `simulation` | state storage, combinational plan, atomic steps, trace records | structure |
| `components` | registers, muxes, adders, register files, memory, controllers | simulation public contracts |
| `microcode` | typed control programs, validation, encoding/decoding | component contracts |
| `machines` | concrete datapaths, ISA definitions, sequential and pipelined CPUs | components and optional microcode |
| `tools` | headless execution and textual reports | public machine/simulation APIs |

The core knows nothing about opcodes, accumulator names, stack pages, status
layouts, or instruction fetch. A microcoded controller is a composed machine
component. A pipelined controller need not use microcode merely to share the
same simulator.

The initial control-store proof addresses words by micro-PC and uses explicit
next/jump/conditional transitions. Widths derive from declared signals and
storage limits. Instruction decode later maps an opcode to an entry point.
There is no inherited 8/8/8 opcode-step-status key or 128-bit control ceiling.
IRATA's microcode validator-after-transformation discipline is retained.
Optimization is deferred until a concrete redundant program motivates it.

## Concrete learning destination

The first CPU will be a deliberately small 16-bit register machine with eight
general registers and a Harvard instruction/data interface. Start with
structured instruction values, modular unsigned arithmetic, explicit register
operands, and equality branches; fixed binary encoding is a later decision.
No implicit 6502 flag register or instruction compatibility is required.

The first instruction subset is immediate load, add, load, store, equal
branch, and halt. Precise encodings and memory layout are settled at the CPU
phase before implementation. Illegal instructions and out-of-range accesses
produce structured faults, not undefined host behavior.

A sequential machine establishes instruction meaning. A three-stage
fetch/decode, execute, and memory/retire pipeline then makes overlap visible.
Stage boundaries may be revised at its design checkpoint if the small ISA
requires a clearer partition. Forwarding and stalls arrive only after a
correct stalled baseline. Branch resolution discards younger work before it
can have architectural effects. Stores and halt take effect at retirement.

The microcoded datapath and pipelined machine are complementary concrete
consumers of the component model. This is not a CPU tournament framework.
The sequential interpreter serves as a correctness oracle for architectural
results, not a second timing specification.

## Proof and scope

Tests prove value semantics, invalid wiring rejection, local transitions,
composition, temporal contracts, replay, and instruction retirement behavior.
Small widths permit exhaustive arithmetic cases. Negative compilation tests
verify selected static guarantees. Every capability narrative links to an
executable scenario once implemented. Text/JSON traces are derived evidence;
neither a GUI nor a waveform viewer is needed for completion.

Deferred: analog simulation, gate delays, metastability, multiple clocks,
hardware synthesis, HDL text languages, out-of-order execution, superscalar
issue, speculation beyond flushing younger pipeline work, virtual memory,
operating systems, graphics, and performance claims. Cache experiments follow
only if the completed pipeline makes memory latency an interesting question.
