# Discrete-time contract

This is the target execution specification for the construction plan. The
[NOT harness](not.md) implements stateless combinational observation with explicit
transient inputs. The initial [DFF](d-flip-flop.md) implementation exercises
observation and shared-edge semantics in the same derived plan. The transitional
[Phase 1](phase-1.md) API separately proves register-only snapshot/commit edges.
See [status](status.md) for the current boundary.

## Storage defines the time boundary

The only initial state atom is an initialized one-bit D flip-flop. Q exposes
committed state throughout evaluation; D determines its next value. Every DFF
samples and commits on the same simulator-wide edge. Clock is not a routed
Boolean signal. There are no gate-built latches, derived clocks, asynchronous
reset events, settling iterations, or independently ticking subassemblies.

Composites acquire timing from their children. A word register is parallel DFFs;
an enabled register selects old Q or new data onto D. Memory and controller
storage follow the same rule. Initialization sets each declared stored bit
before edge 0; a later synchronous reset, if needed, is data-selection circuitry.

## One edge, one transition

Let S[n] be committed storage before edge n and I[n] the external inputs held
for that edge. Evaluate pure combinational dependencies to obtain C[n], then
compute every storage element's proposal from S[n], I[n], and C[n]. Validate
all proposals and atomically commit S[n+1]. No proposal reads another proposal.

The step returns the new state plus evidence for edge n. Observing the new
state's combinational outputs requires a separate pure observation using
explicitly supplied inputs; it must not advance time or perform side effects.
The first edge is 0. Reset supplies S[0].

The simulation may mutate owned buffers internally for efficiency, but the
public semantics are an atomic transition. Failure leaves committed state and
cycle index unchanged. Trace records own snapshots or immutable values, never
references to buffers that the next step overwrites.

## Pure observations and transient inputs

Let `Eval(S, I)` be pure combinational evaluation of a finalized circuit.
An observation at committed state S[n] with explicitly supplied inputs I'
returns `Eval(S[n], I')`. I' may be hypothetical and need not equal the inputs
used for any edge. Observation is on demand, not inspection of a cached last
edge result. It does not compute or commit a state transition, advance the
edge index, consume a protocol transaction, or install inputs for a later call.

Each observation and step supplies a complete input snapshot for the assembly's
unconnected external input ports. A nested external input with an inbound parent
wire takes its value from that wire and cannot also receive a binding. The
simulator has no implicit mutable "current input" register and never fills
missing inputs from a previous call. Invalid bindings or input values return
structured diagnostics without changing state or time.
A closed circuit with no external inputs uses an empty snapshot.

`step(I[n])` independently evaluates C[n] = `Eval(S[n], I[n])`, validates the
edge, and commits S[n+1]. Prior observations with any I' must not change that
result. After a successful step, observing I' evaluates `Eval(S[n+1], I')`;
it does not return the pre-commit C[n] stored in that edge's evidence. Returned
observation values own their snapshots, so later observations and edges cannot
alter them. Internal caching is allowed only if it preserves these semantics.

## Initialization and the first edge

Initialization establishes all of S[0] and sets the next edge index to 0. It
performs no clock edge or automatic state proposal/commit. It does not require
an eager combinational evaluation: there is no unique C[0] until inputs I[0]
are supplied. Initialization must not invent zero values for external inputs.

Immediately after initialization, `observe(I[0])` produces the valid
C[0] = `Eval(S[0], I[0])`, before any active edge. Repeated observations with
different snapshots remain at S[0]. The first `step(I[0])` produces edge-0
evidence and S[1], whether or not observation was called first. This guarantees
pre-edge observability without making constructor side effects or a settling
pass part of the circuit's timing contract.

## Evaluation rules

- Combinational dependencies form a directed acyclic graph after state
  boundaries are treated as sources/sinks. Feedback through a register is valid.
- Derive a topological plan during finalization. Reject a combinational cycle
  with an endpoint path; do not iterate toward a guessed fixed point.
- The allowed Boolean atoms declare their input dependencies. Derive a
  composite's dependencies through its actual child connections, rather than
  assuming every composite output depends on every input. Stored Q outputs
  are current-state sources and do not depend on their D inputs at this edge.
  Wiring aliases preserve the dependencies of the bits they reference.
- Enumeration order cannot affect values. Stable hierarchical paths provide
  deterministic tie-breaking for independent evaluations and trace ordering.
- Fan-out is allowed. Multiple ordinary drivers, missing required inputs,
  width mismatches, and cross-root connections are rejected.
- There are no X/Z values. All storage has an explicit reset value; unresolved
  wiring is a diagnostic. Reset is initially whole-assembly initialization,
  not a dynamic reset pin or partial reset protocol.
- Bit arithmetic wraps at the declared width. Shifts, slicing, extension,
  truncation, and signed interpretation require explicit documented semantics;
  host-language undefined behavior must never define circuit behavior.

## Composed registers and memory

An enabled register samples its data when enable is true; otherwise it holds.
An always-loading register samples on every edge. Swapping
two registers on one edge swaps their old values. Register chains advance at
most one register per edge.

Initial memory/register-file circuits use composed registers, decode, and
selection: combinational reads from current storage and writes committed on
the edge. A read and write to the same address therefore observes the old
value for that edge. Bounds and conflicting writes
are checked before any commit. One write port is the initial memory boundary.
Later registered or delayed memory uses a separately named component contract.

## Later multi-cycle work and streams

When the roadmap reaches waiting components, the first multi-cycle unit uses
start/busy/done with one operation in flight.
Acceptance occurs on an edge when start is asserted and the unit is idle.
Its operands are latched at acceptance, so later input changes do not affect
that operation. The proof must define the exact result edge and behavior of a
start while busy; the default is rejection as protocol misuse.

For pipelined communication, a channel carries payload, valid, and ready. A
transfer occurs only when valid and ready are both true in C[n]. A producer
holding valid while ready is false retains both valid and payload across the
edge. A transfer consumes the old payload even if the same edge installs a new
one. The initial channel buffer holds one item and has no empty bypass.

Ready propagation participates in combinational dependency analysis. Buffering
must break a ready/valid cycle explicitly. Lack of progress is observed through
bounded scenarios and wait reasons, not assumed impossible from connectivity.
Deadlock freedom is not a general compile-time guarantee.

## Controllers and architectural effects

A microinstruction is selected from current micro-PC and drives controls for
the current edge. Conditional transitions inspect current-state status. A
flag newly committed at this edge is available to a later edge; same-edge
tests of arithmetic outputs require explicit combinational connections.

Microcode describes simultaneous controls and state transitions. It does not
call procedural bus read/write methods or reproduce IRATA's five phases.
Registers and units embody the timing contract independently of the controller.

Pipeline state includes valid bits and instruction identity. A flush invalidates
younger work according to an explicit age/order rule. Memory writes and halt
are architectural effects allowed only for the valid retiring instruction.
Before implementation, the pipeline phase must specify priorities for coincident
stall, flush, completion, and fault. No implicit C++ statement ordering may
choose those priorities.

## Evidence requirements

Each step can report changed storage, accepted transfers, completed operations,
controller transitions, stalls with reasons, and instruction retirement.
Paths and numeric values are stable; wall-clock timing is absent. Recording is
bounded or streamed so a long run does not require retaining its entire past.
An input script and the same finalized definition/reset state reproduce the run.
Cross-version serialized replay is not promised in v1.

Before the first processor, prove register swap, a two-stage register chain,
fan-out, two independent same-width buses, order-independent results, rejected
combinational feedback, and read-before-write memory behavior. Held output under
backpressure belongs to the later waiting-component gate and must be proved
before adopting streaming components. These are acceptance requirements, not
claims about the transitional implementation.
