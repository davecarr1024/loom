# Learning roadmap

Only the documentation skeleton is currently delivered. Phases below are
ordered learning checkpoints, not permission to implement the entire plan
at once. Each phase closes with tests, inspectable evidence, docs, local gates,
and the push-boundary review in AGENTS.md.

## Phase 0: Named and reviewable design

Deliver README, design, timing contract, baseline research, decisions, this
roadmap, status, agent guidance, and a documentation check with CI. Verify the
internal links and coherence of representation, timing, and scope. Publish a
new repository. No production-code coverage claim applies.

## Phase 1: An assembly smaller than a CPU

First bite: establish C++23/CMake and a width-checked register transfer over an
explicit connection. Add the smallest structural adapter, finalized assembly,
state holder, and edge transition needed to execute it independently.

Subsequent bites: two-register swap, two independent same-width buses, and
nesting the circuit inside another assembly using the same public interface.
Settle stable instance handles and finalized-definition lifetime with tests.

Proof: exhaustive tiny-width transfer/hold; compile rejection of incompatible
port widths; runtime invalid binding diagnostics where values require them;
multiple simulations with independent state; identical result under different
child enumeration; atomic failure. Artifact: a textual transfer trace.

Tooling gate: normal tests, compile-fail cases that fail for the intended
reason, clang-format, compatible clang-tidy, 100% production line/function
coverage, and a single presubmit command shared by CI. Pin a working compiler
and test dependency version then; do not claim untested compiler support.

Checkpoint: can a reader understand the fixture without knowing any CPU?

## Phase 2: Arithmetic assembled from smaller parts

Build a Boolean full adder, compose a small ripple adder, and place it between
explicit registers. Derive combinational dependencies and state/port facts
through separate structural analyses. Reject combinational cycles.

Proof: exhaustive four-bit operands/carry, overflow wrapping, feedback through
a register, and the different latency of combinational and registered paths.
Artifact: inspectable component inventory and edge-by-edge arithmetic trace.

Checkpoint: are facts derived from the actual objects, and is the primitive
floor useful without introducing gate-delay simulation?

## Phase 3: A controller that teaches microcode

Compose an accumulator datapath, explicit shared-bus selection, micro-PC,
control store, and conditional next-address logic. Typed C++ control words
precede any text DSL. First program: load operands, add, retain a result, stop.

Proof: accepted controls target real resources; conflicting selected bus
drivers and conflicting state writes are rejected; branches inspect the
documented state edge; encoded/decoded controls round-trip; a trace explains
each transfer. A validator runs after any future transformation pass.

Checkpoint: can the controller run against this small datapath without a
global CPU model? No optimizer or assembler is required yet.

## Phase 4: Explicit latency and waiting

Build one small multi-cycle arithmetic unit (a shift/add multiplier is the
default) with latched operands and start/busy/done. Then compose a one-item
ready/valid buffer with a scripted consumer that stalls.

A separate small bite introduces a one-write-port memory and proves
current-state reads, same-address read/write behavior, and atomic rejection
of out-of-range access before that component enters a processor.

Proof: precise completion edge, busy-start rejection, unchanged latched operands,
stable blocked payload, simultaneous consumption/replacement, no lost or
duplicated values, deterministic bounded progress. Reject combinational
ready loops. Artifact: a transaction trace with wait reasons.

Checkpoint: does time remain understandable when local progress differs?

## Phase 5: A sequential register machine

Finalize the tiny ISA and memory layout described in design.md. Add a register
file, instruction/data memories, decoding, and sequential control. Build a
small independent instruction interpreter for architectural assertions.

Proof: arithmetic, load/store, equal branch, illegal instruction and bounds
faults, and a short summation loop with asserted memory/register results.
Read/write collision behavior follows timing.md. Artifact: instruction trace.

Checkpoint: can the datapath be drawn and explained? Concrete differences from
the microcoded accumulator must be accommodated without CPU-specific core code.

## Phase 6: Overlap with conservative stalls

Design the proposed three-stage pipeline in a short phase document first.
Specify stage contents, operand-read timing, retirement, instruction ages,
and priorities among stall, flush, halt, and fault. Implement conservative
dependency stalls before forwarding. Branches discard younger work.

Proof: retirement state agrees with the interpreter; RAW dependencies stall;
taken branches squash younger stores; halt prevents younger side effects;
invalid stage payloads cannot execute. Artifact: cycle table with reasons.

Checkpoint: can every idle stage and discarded instruction be explained?

## Phase 7: Forwarding and delayed memory

Add one forwarding path at a time, proving which stall it eliminates. Then
replace fixed-response data memory with a bounded deterministic response
schedule and an explicit request/response interface.

Proof: forwarding priority, load-use stalls, held requests, exactly-once stores,
no duplicated retirement, branch interactions during a wait, and the same
architectural result across tested latency schedules. Assert cycle counts for
named cases, not host performance. Artifact: before/after cycle evidence.

Checkpoint: can the processor's lost cycles now be explained on a whiteboard?

## Phase 8: Close the learning proof

Curate a headless capability tour: component transfer, composed arithmetic,
microcoded sequencing, waiting, pipelined dependencies, and memory stalls.
Link every claim to tests and derive example traces from executable scenarios.
Write regrets, remaining limitations, and an assessment of which abstractions
earned their place. The v1 goal is complete only when the pipeline can be
explained and its architectural effects verified, not when the library has
accumulated many components.

Caches, alternate clocks, synthesis, out-of-order execution, text HDL syntax,
and interactive visualization require a new concrete learning question.
