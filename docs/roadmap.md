# Component construction roadmap

The development framework is upward component building. A verified component
with a purposeful code interface is the unit of progress. Containing circuits
prove composition and feed regression cases back down; a machine milestone does
not replace its components' individual acceptance gates.

Every component follows [contracts and hardening](component-contracts.md): define,
construct, test alone, test in a parent, explain the execution, run `./scripts/check.sh`,
and complete the [push-boundary review](../AGENTS.md). Work through each group's
components individually. The groups below express dependencies and learning
questions, not permission to implement a whole library at once.

## Current starting point and next task

The original Phase 1 register-only baseline is retained and tested. Its atomic
wide registers and path-based enables are temporary mechanisms, not the new
allowed floor. The uncompleted old Phase 2 arithmetic prototype is shelved; its
unrestricted primitive interface and primitive full adder are not adopted.
See [status](status.md) and [decisions](decisions.md).

**NOT is implemented and accepted:** see [its contract and evidence](not.md).
The first typed harness provides pure pre-edge observation, derived inventory,
connection/schedule inspection, two-gate composition, invalid topology rejection,
enumeration independence, transient snapshots, and retained values. The existing
register baseline remains tested. These are the completed NOT requirements,
not completion of group A.

**NOT, AND, OR, constant bit, initialized DFF, gate-composed XOR, and one-bit
mux are implemented and accepted:**
see [NOT](not.md), [AND](and.md), [OR](or.md), [constant bit](constant-bit.md),
and [DFF](d-flip-flop.md), plus [XOR](xor.md) and [mux](mux-bit.md). The
combinational atoms prove
their truth tables and parent composition. The DFF proves initial state,
transient observation, edge-0 equivalence, repeated sampling, feedback,
independent simulations, and simultaneous commit. XOR proves its truth table,
gate inventory, schedule, and parent composition. Mux proves every selector/data
row and parent composition. Preserve these and the transitional register proofs.
**Fixed-width wire bundles are implemented and accepted:** see
[their contract and evidence](wire-bundles.md). Bundle views preserve scalar
endpoint identity; simulation finalizes each expanded wire through the existing
plan. **The width-parameterized word mux is implemented and accepted:** see
[its contract and evidence](mux-word.md). The shared selection contract is
proved for bit and word muxes; all one-bit rows and all 512 four-bit input and
selector combinations pass, including parent composition. **The two-to-four
one-hot decoder is implemented and accepted:** see
[its contract and evidence](decoder-2-to-4.md). All four addresses produce one
high output at the matching index, and its NOT/AND inventory and parent-boundary
composition are tested. **Group B is complete; next: word register and movement.**
Do not reintroduce the old unrestricted primitive API.

## A. The atomic digital floor

Build NOT, AND, OR, constant bit, then one-bit D flip-flop, each with an explicit
contract. The first five atoms are accepted; selection components are next.
NOT's harness grows into a CPU-independent runner usable at every
level. External inputs are supplied by value per observation/edge; all machine
state is declared storage. Admit only the [design allowlist](design.md).

Proof: exhaustive gate truth tables; constants have no evolving state; D/Q
initialization, sampling, repeated edges, and independent simulations. Derive
inventory and dependency order from owned objects. Test invalid ports, duplicate
ownership/paths, missing/multiple drivers, foreign bindings, and combinational
cycles. A DFF breaks a feedback dependency and all DFFs commit simultaneously.
An invalid input request leaves every stored bit and the edge index unchanged.
With DFFs present, compare identical edge-0 steps with and without preceding
observations using different inputs. They must produce identical S[1] and edge
evidence. Prove post-edge observation reads the newly committed state, while
retained pre-edge observations and edge evidence keep their original values.

Artifact: truth-table output and a one-bit edge trace with D, old Q, and new Q.
Checkpoint: can the whole evaluation algorithm be explained using only this floor?

## B. Selection and bit bundles

XOR and one-bit muxes are accepted as gate-composed components. Fixed-width
bundles, the word mux, and the two-to-four one-hot decoder are accepted. Mux and
decoder behavior must be gate composition. Bit indexing,
concatenation, splitting, and fan-out are explicit wiring with documented ordering.

Proof: exhaustive one-bit and four-bit selection, every decoder address and
exactly one active output,
representative bundle widths, independent same-width paths, and no behavior
introduced by adapters.
Inventory demonstrates the gates used; a parent trace expands a selected output
into its input ports. Define all selector patterns or reject reserved encodings.

Artifact: a selected word traced through the actual bit muxes and a decoded
address expanded to its one-hot gates. The width-parameterized selection
contract has shared tests for bit and word muxes. Checkpoint: do bundles make
interfaces readable without hiding computation?

## C. Registers and movement

**A DFF-composed word register is implemented and accepted:** see
[its contract and evidence](word-register.md). It exposes least-significant-first
data/output bundles and explicit per-bit initialization; tests cover initial Q,
whole-word edge loading, retained evidence, and parent wiring. Next construct an
**An enabled word register is implemented and accepted:** see
[its contract and evidence](enabled-word-register.md). It loads or holds through
owned MuxWord and WordRegister children; a shared readable-word contract captures
the shape consumers use. Next construct a shift register. Establish typed
data/enable interfaces and explicit initialization. Migrate the transfer baseline
to this implementation:
its wide atomic storage and enabled-path script adapter must then disappear
from the supported production model. Preserve the old transfer/hold/swap
behavior through tests at the replacement interface, not a permanent second engine.

Proof: bit independence, whole-word sampling, enable/hold, simultaneous swap,
one-register-per-edge chain movement, fan-out, nested repeated instances, and
owned evidence that later edges cannot overwrite. Verify DFF counts from the
actual register structure. Demonstrate a boundary-to-bit trace expansion.

Define the first register contract families around actual consumer needs:
readable word, edge-loaded word, enabled word, and shift behavior as needed.
Test shared guarantees and explicit adaptations (such as enable tied high);
prove that incompatible timing/control shapes are not accepted accidentally.

Artifact: transfer, swap, and shift traces with register and DFF views.
Checkpoint: can register behavior be explained entirely through child circuits?

## D. Selected buses and bus-connected registers

Construct a small explicitly selected bus from word muxes and a register bank
that can send and receive through it. Use a bounded source-selection interface;
if idle is useful, represent it explicitly with validity and define invalid
consumption. Do not model tri-state resolution or retained values on an undriven bus.

Proof: each source/destination, independent same-width buses, fan-out, transfer
and hold, and rejected invalid selection/consumption with atomic state behavior.
An ordinary input still has exactly one driver; selection happens inside the
bus circuit. Test a parent wiring mistake at its integration boundary.

Artifact: a transfer expanded into selection, mux outputs, and destination D/Q.
Checkpoint: do controls describe meaningful resource choices in code?

## E. Arithmetic and the ALU

Construct half adder, full adder, small ripple adder, incrementer, equality
comparison, and a small ALU using Boolean and selection components. Choose the
ALU's minimal operation set for the forthcoming accumulator datapath; do not
add speculative operations. Build a counter from incrementer and register.

Proof: exhaustive full-adder inputs and four-bit operands/carry, wrapping and
carry-out, equality boundaries, all defined ALU selections, register feedback,
and combinational versus registered latency. Each full adder must expand to
gates. Test formulas are independent oracles, never composite execution shortcuts.

Artifact: arithmetic and counter traces at ALU, adder, gate, and storage levels.
Checkpoint: do meaningful interfaces survive several levels of composition?

## F. Small storage assemblies

Construct a small one-write-port memory and register file from registers,
write decoding, and read selection. Start with a power-of-two capacity and a
matching address width; select concrete small sizes before implementation.
Initial contents initialize child storage, not a parallel hidden memory array.

Proof: every address, write-enable/hold, unaffected locations, initialization,
old-value read on a same-edge write, and simultaneous operand reads for the
register file. Define invalid encodings if later capacities introduce them.
Inventory accounts for stored bits and selection logic. No host-array memory
atom is allowed merely to make examples larger.

Artifact: a read/write expanded into address decode, one selected word, and its
stored bits. Checkpoint: does scaling expose understandable circuits or motivate
better views rather than behavioral shortcuts?

## G. Controller and accumulator datapath

Build a bounded sequencer, control store, and conditional next-address circuit
from established components. Typed control definitions encode actual resource
choices; constants/wiring and the composed storage/selection circuits implement
the control store. Develop control families for resource-bound loads, bus selection, and
sequencer transitions. Prove shared effect contracts and reject incompatible
resource bindings; use explicit adaptation for differing timing. A host
instruction handler must not execute the datapath.
Compose an accumulator datapath using the bus, registers, and ALU before adding
instruction decode.

Proof: load, transfer, add, retain, and stop sequences; old-state conditional
branching; all control encodings or reserved-pattern rejection; resource conflicts;
exact edge timing. If encoding tooling is introduced, verify encode/decode and
validate after every transformation. No optimizer or text microcode DSL is needed.

Artifact: control-state transitions expanded into bus and register activity.
Checkpoint: can the controller and datapath each be tested without a CPU?

## H. An IRATA-style accumulator CPU

Use the established datapath, controller, memory, and decoding components to
construct one small accumulator CPU. Before implementation, specify its width,
minimal ISA, addressing, initialization, halt/fault behavior, and a bounded demo
program. IRATA-style means visible bus movement and explicit control, not 6502
compatibility or inherited simulator phases.

Proof: instruction semantics against an independent test oracle, arithmetic,
load/store, branch, halt, illegal encodings, and a short loop with asserted
architectural state. Reuse component tests and show at least one reduced defect
or explicitly injected test-only fault whose repair is verified through parents.

Artifact: one execution viewed as instructions, controller steps, transfers,
and selected child gates/DFFs. Close the initial construction proof with a
capability tour, interface/bug retrospective, and known limitations.
Checkpoint: can someone explain a machine failure by descending through its circuits?

## I. Successive machine questions

These are planned extensions after H, each requiring its own component contracts
and design checkpoint. They do not fix today's ISA or component interfaces.

| Construction | New question | Required evidence before acceptance |
| --- | --- | --- |
| Sequential register CPU | How does explicit operand organization change the datapath? | Tested register-file integration and instruction-oracle agreement |
| Multi-cycle unit and one-item channel | How do composed components retain work and wait? | Latched operands, exact completion edge, stable blocked payload, consume/replace, bounded progress, rejected ready loops |
| Conservative pipeline | How can overlap remain correct? | Declared stage state and priorities, valid/age handling, retirement-oracle agreement, dependency stalls, branch squash and halt/fault effects |
| Forwarding and delayed memory | Which waits are necessary? | Each forwarding priority, load-use waits, held requests, exactly-once stores/retirement, deterministic schedules and before/after cycle evidence |

Use the same circuit model and admit new atoms only through the floor-change
policy. Each CPU must teach a named question; an open-ended CPU tournament,
performance contest, caches, alternate clocks, synthesis, and out-of-order
execution are outside this plan.
