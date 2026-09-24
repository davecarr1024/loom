# Loom design

## Thesis

Can a small digital foundation support increasingly complex machines while
preserving local reasoning, reproducible failures, and explanations at every
level?

Loom is a headless C++23 laboratory for upward component construction. Its
fundamental object is a circuit with a constrained, purposeful interface.
A component earns its place through independent tests, actual composition,
and understandable evidence. Registers, buses, ALUs, memories, and controllers
become the vocabulary from which successive CPUs are constructed.

The name still fits: small parts and connections are woven into larger
machines. IRATA2 contributes hardware-ish structure and visible control;
Rule Lab contributes concrete typed composition and derived structural facts.
This is a new experiment, without IRATA instruction or API compatibility.

This document specifies the target architecture. [Status](status.md) and
[Phase 1](phase-1.md) describe the transitional register-only implementation;
[NOT](not.md), [AND](and.md), [OR](or.md), [constant bit](constant-bit.md), and
[DFF](d-flip-flop.md) document the initial atom floor; [XOR](xor.md) documents
the first gate-composed component; [one-bit mux](mux-bit.md) documents the
first selector.
[Roadmap](roadmap.md) owns construction order; [timing](timing.md) owns execution
semantics; [component contracts](component-contracts.md) owns the acceptance
and regression discipline. [Decisions](decisions.md) records the change in direction.

## The abstraction floor

Keep circuit evaluation simple by choosing explicit digital atoms. The initial
allowlist is deliberately small:

| Atom | Ports and behavior | Persistent state |
| --- | --- | --- |
| NOT | One bit in, its complement out | None |
| AND | Two bits in, conjunction out | None |
| OR | Two bits in, disjunction out | None |
| Constant bit | No input, one fixed 0 or 1 output | None; immutable definition value |
| D flip-flop | D input, Q output; Q exposes current state, D is sampled on the shared edge | One bit with an explicit initial value |

XOR, muxes, decoders, adders, enabled registers, counters, buses, register files,
small memories, ALUs, and controllers are composites. A flip-flop does not
have primitive enable, dynamic reset, or clock-gating inputs. Enable and
synchronous reset can be built with data selection. Initialization supplies
all stored bits before edge 0.

Port aliases, fixed bit ordering, bundling/unbundling, and fan-out are wiring
operations. They carry identity and route bits; they do not execute arithmetic,
selection, decoding, or mutable behavior. External input/output bindings and
initial-state loading are simulation boundaries, not hidden machine components.
Fixed-width bundles are non-owning views over existing scalar ports. Index 0 is
the least-significant bit; concatenation takes the lower bundle first, and
splitting preserves that order. A bundle connection expands to individual
instance-specific scalar wires before finalization.

Adding an atom requires a design decision naming its semantics, why composition
is inadequate for the learning question, and the proof that keeps evaluation
understandable. The simulator must not accept arbitrary user-defined behavior
as a way around the allowlist. C++ arithmetic functions may serve as independent
test oracles, but a composite's execution comes from its children and wires.
A host-array memory shortcut or direct ALU evaluator is outside the initial floor.

This boundary models digital logic and explicit state, not voltages, currents,
transistors, propagation delays, metastability, or gate-built storage loops.
Gate-count minimality is not a goal; the selected Boolean basis is small enough
to explain and sufficient to build the planned components.

## Functional circuit representation

Combinational atoms are pure total functions of their declared inputs. The
flip-flop is an explicit state transition. A composite owns child components
and connects their ports; it supplies no behavioral callback, local tick loop,
or second implementation of the assembled operation.

Definitions are concrete immutable C++ values with typed interfaces. A
simulation separately owns all evolving state. Two simulations may share one
const definition without sharing storage. No persistent machine value may hide
in a callback, static variable, controller handler, or diagnostic adapter.

Flattening a definition into an evaluation plan is allowed when it preserves
child and port provenance. Replacing an assembled circuit with a monolithic
behavioral shortcut is not. Observations and traces must still describe the
circuit that was authored and evaluated.

## Interfaces and invalid states

Interfaces express meaning as well as shape. Port direction, width, component
instance, and semantic role must be clear in code. A bus selection is a choice
among its actual sources; it is not an unconstrained collection of procedural
read/write calls. A memory address names a location in its declared capacity;
an ALU operation chooses among its defined functions.

Use C++23 concepts, traits, constrained functions, immutable values, and
`std::expected`. The enforcement layers have distinct responsibilities:

| Layer | Guarantees or checks |
| --- | --- |
| Types and construction | Port roles, compatible widths, valid interface choices, explicit initialization, immutable ownership |
| Finalization | Actual endpoint membership, unique ownership/paths, driver completeness, allowed atoms, combinational acyclicity |
| Execution | Value-dependent protocol obligations and rejected-edge atomicity |
| Tests | Truth tables, temporal promises, integration, and regression behavior |

A constrained C++ control value does not prove that arbitrary internal wires
encode a valid choice. Every encoding declares the meaning of all bit patterns,
or validates reserved patterns before commit. Start with small power-of-two
memory capacities and total selectors where that makes invalid encodings
unnecessary. Do not silently coerce an invalid request into a valid operation.

Types cannot prove purity of arbitrary host code, ownership lifetime for an
unrestricted structural protocol, deadlock freedom, or every temporal promise.
Restrict the supported construction API and test its guarantees rather than
claiming those properties from names or traits alone.

## Component families and refinement

Containment, capability, and specialization are different relationships. A
component can be built from DFFs, expose a readable-word capability, and satisfy
an enabled-register contract without inheriting from a universal Register base.
Use concepts for consumer-facing capabilities, parameterization for widths and
capacities, and derived traits for structural facts. Typed controls bind choices
and effects to the actual interfaces they operate.

A claimed specialization must preserve the weaker interface's meaning, timing,
accepted input sequences, and promised observations. Shared contract tests and
parent scenarios substantiate that claim; C++ concept satisfaction alone does
not. Different latency or additional required control ports call for explicit
adapters. Develop the register and control taxonomies as their concrete families
arrive, following [component contracts](component-contracts.md).

## Ownership and structural analysis

Concrete objects form a finite ownership tree. Connections reference specific
ports on specific owned instances; equal payload types do not imply identity.
Connections across siblings use their explicit interfaces. Repeated ownership,
dangling references, cross-root bindings, and ambiguous diagnostic paths are
invalid. Names aid explanations; operations do not acquire meaning from names.

Finalization discovers existing children and derives validation and evaluation
facts. Never maintain a second hand-authored simulator hierarchy. Definitions
retain stable storage and remain non-copyable/non-movable unless relocation is
explicitly made safe. Simulations retain shared ownership of const definitions.

Independent analyses derive counts, port inventories, connections, and
combinational dependencies from that same structure. Counts add over ownership;
connections preserve endpoint identity; traversal order does not define time.
Compile-time facts describe type-visible structure. Value-dependent wiring is
checked at finalization. Neither kind of fact substitutes for behavior tests.

## Time and feedback

There is one simulator-wide discrete edge. Current flip-flop outputs and
external inputs feed an acyclic combinational graph. After evaluating that
graph, all D inputs are sampled and all Q values commit together. Feedback
through storage is valid; same-edge combinational feedback is rejected.

A counter therefore has feedback wiring while keeping each evaluation
feed-forward. Its incrementer sees the current count, not another component's
partially committed next value. C++ enumeration, nesting, and diagnostic names
must not change machine results. Details and future component timing contracts
are in [timing](timing.md).

## Building and hardening components

The development unit is a verified component with a useful interface. Build it
from established components, prove it alone, then exercise it inside a parent.
Its interface becomes stable through that evidence, not by freezing speculative
APIs. Each component must answer a concrete learning question or serve a named
assembly. The project is not an open-ended catalog of interchangeable strategies.

Every bug investigation should preserve the revealing assembly scenario and
seek the smallest failing component or connection contract. Compare the first
incorrect boundary value, descend through the same recorded execution, reduce
the input sequence, and add a regression at the appropriate level. A defect
between two correct components belongs to their integration contract.

Hardening propagates upward by rerunning affected parents and the complete
presubmit gate. If a contract changes, update its consumers, examples, and
documentation together. A local fix with a passing leaf test is not sufficient
proof that the containing machine is repaired.

## Understandable behavior

Every completed component exposes a derived inventory and an executable
scenario. Evidence includes ports, values, current storage, proposals, committed
changes, edge indices, and the definition paths connecting them. Parent views
summarize that evidence without inventing a separate execution narrative.

A bus transfer can expand into source selection, mux inputs, register D inputs,
and committed Q values. Later an instruction event can expand into controller
state and datapath activity. Explanatory labels map to real child/port identities;
they must not recompute the machine's behavior in a second interpreter.
Independent interpreters are test oracles, not the source of simulation traces.

Begin with bounded deterministic text/structured records and hierarchy filters.
There is no requirement for a GUI, waveform viewer, universal debugger, or
cross-version replay format. The same definition, initialization, and input
script must reproduce the same evidence within a version.

## Modules and dependencies

Introduce directories and matching namespaces as their first component requires
them. This is a dependency direction, not a demand to scaffold empty modules:

| Area | Responsibility | Depends on |
| --- | --- | --- |
| Value and structure | Bits/bundles, typed ports, ownership, contracts, structural analysis | Standard library |
| Simulation | Allowed atom semantics, state, scheduling, observation, atomic edges | Value and structure |
| Components | Gate-composed logic, storage assemblies, buses, arithmetic, controllers | Structure and simulation contracts |
| Machines | Concrete accumulator, register, and pipelined machines | Tested components |
| Tools | Headless scenarios and evidence formatting | Public component/machine interfaces |

The core knows nothing about addition, memory addresses, instruction opcodes,
accumulator names, microcode, or CPU stages. Optional microcode tooling works
through declared controller resources and does not add simulator mechanisms.

## Learning destinations and scope

An IRATA-style accumulator CPU will exercise bus movement and explicit control.
A register machine will exercise operand organization. A later pipeline will
exercise overlap, forwarding, waiting, and cancellation. Each must justify its
new components and reuse established ones. Exact widths, ISA, memory layout,
and pipeline partition are deferred to their construction checkpoints.

The initial construction proof closes with tested components through a small
accumulator CPU, a hierarchical capability tour, and a retrospective showing
that failures can be localized and repairs verified upward. Successive CPU
forms are planned extensions with their own acceptance gates, not a prerequisite
for calling a register or ALU complete.

Deferred: analog/event-delay simulation, gate-built latches, multiple clocks,
synthesis, text HDL, CPU compatibility, out-of-order/superscalar execution,
virtual memory, operating systems, graphics, and performance claims. None may
be introduced to bypass the explicit circuit-construction premise.
