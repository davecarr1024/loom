# Component contracts and hardening

This is the acceptance discipline for the [construction roadmap](roadmap.md).
It specifies future deliverables; it does not claim the transitional baseline
already provides typed controls, gates, or hierarchical trace expansion.

## Contract before implementation

For each named component, put a concise contract beside its public C++ interface
and add a component document when the timing or composition needs explanation.
Use purposeful constructors, port types, and constrained control values so the
supported choices are visible in code. Include:

| Contract item | Required answer |
| --- | --- |
| Purpose and dependencies | What learning question or parent needs this component? Which established components construct it? |
| Interface | What does each port mean, in which direction, at what width and bit order? Which instance owns it? |
| State and initialization | Which child flip-flops store values, and what is their initial state? |
| Timing | What is observable immediately after initialization and before/after an edge? What is sampled? Follow the call-scoped observation rules in [timing](timing.md). |
| Input domain | What combinations/sequences are accepted? What do unused encodings mean? |
| Failure | Which layer rejects invalid construction or use, with what structured diagnostic and state-preservation guarantee? |
| Evidence | Which inventory, boundary values, and child paths explain one execution? |

Name operations and interface roles rather than passing loosely related numeric
arguments or raw string commands. Connection types must distinguish repeated
same-width instances. Public interfaces expose intentional ports; containing
circuits should not depend on incidental internal child layout.

## Contract families and specialization

Construction answers what a component contains. A contract family answers what
a consumer may assume about it. These are independent relationships: a register
contains DFFs but is not a subtype of a single DFF. A word mux and a bit mux can
be instances of a width-parameterized selection family without sharing storage
or a runtime base class.

Use a small vocabulary as concrete components require it:

| Relationship | Representation to explore | Proof obligation |
| --- | --- | --- |
| Parameterization | Width/capacity template parameters with bounded domains | Each supported shape has the declared semantics; test small exhaustive and representative wider instances |
| Capability | Concepts for a meaningful consumer interface, such as a readable word output | Compile-time shape check plus shared behavioral contract tests |
| Structural facts | Traits derived from actual children, such as stored-bit count | Inventory agrees with structure; no second authored taxonomy |
| Refinement | A named contract adding guarantees to a weaker one | Preserve the weaker contract's observations, timing, and accepted uses |
| Adaptation | Explicit circuit or interface adapter between different contracts | Test the mapping and any added latency/obligations; do not claim automatic substitution |

The supported API should distinguish atomic components from composites and
identify combinational versus state-containing structure. Those structural
classifications do not alone establish semantic substitutability. Similar port
shapes can have different meanings or latency.

For a claimed replacement, corresponding ports must retain meaning, direction,
width, and timing; accepted inputs/sequences must not narrow; promised outputs,
initialization, and failure behavior must remain valid. Additional ports require
an explicit binding or adapter. For example, an enabled register does not simply
replace an always-loading register: tying enable high is an explicit adaptation.
A registered adder cannot silently replace a combinational adder.

Register families may distinguish readable storage, edge-loaded words,
enabled words, and shiftable words. Introduce only the concepts needed by actual
consumers, and use shared contract tests to show what is common. Keep differences
visible instead of forcing every register into a large inheritance tree.

Controls deserve similarly purposeful types. A load control targets a particular
register interface; a bus selector chooses among that bus's sources; a sequencer
transition chooses an allowed next-state action. Typed records, variants, and
resource-bound handles can express these families. Composition of controls must
retain resource identity and make conflicting effects either unconstructible or
explicitly rejected. Their encoding drives real circuit ports; it must not invoke
host-side device methods. Temporal meaning is part of the control contract.

At each family introduction, include positive consumer examples, intentional
compile-fail examples for incompatible shapes/roles, shared behavior tests, and
at least one counterexample or adapter showing the family's boundary. This is a
practical contract algebra earned through circuits, not a general type-theory
framework or proof assistant built in advance.

## Construction and acceptance loop

1. Write the contract and choose an existing parent scenario or a minimal harness.
2. Build the component from the allowed atoms or already tested composites.
   Only the atom allowlist may have simulator behavior implementations.
3. Prove its public behavior: exhaustive small domains where practical, boundary
   cases at wider sizes, initialization, hold/update, and exact edge sequences.
4. Prove structural fidelity: inventory contains the intended children and
   dependencies; there is no alternate composite evaluator or hidden state.
5. Exercise it inside a containing circuit. Check fan-out, repeated instances,
   independent simulations, and equivalent results under valid enumeration changes
   where relevant. Cover meaningful invalid wiring and protocol cases.
6. Produce a runnable evidence artifact and a concise explanation that can expand
   a parent boundary into the responsible child ports/state on the same edge.
7. Run `./scripts/check.sh`, update docs and status, perform the push-boundary reviews,
   and publish only the checked contents. The component then becomes a dependency.

Do not require a CPU to test a leaf. Do not call a declared latency a proof.
Coverage remains 100% measured production lines/functions with narrowly
justified exceptions only; meaningful behavioral tests are a separate obligation.
   Check type guarantees with negative compilation cases that fail for the intended
reason. Prefer independent Boolean/arithmetic oracles to expected values copied
from the circuit's implementation.

## Regression drill-down

Given a failing assembly and input script:

1. Retain its initialization, inputs, and earliest incorrect boundary observation.
2. Use ownership and connection provenance to find the responsible child or
   boundary contract. Compare current-state inputs and proposed next state at
   the relevant edge before looking at later symptoms.
3. Reduce to the smallest circuit and shortest input sequence that reproduces
   the defect. Preserve at least one failing case before applying the fix.
4. Add a leaf, temporal, or integration regression at the actual fault boundary.
   If both children satisfy their contracts, repair the parent or its contract.
5. Fix the implementation or explicitly revise the contract and all affected
   consumers. Rerun the original scenario, affected parents, and
   `./scripts/check.sh`.
6. Record what the failure taught about composition and whether the interface
   prevented, exposed, or obscured the mistake.

If no natural defect has arisen, use a documented test-only wiring/control
mutation to demonstrate the method. Never leave the fault in production or
present an injected failure as a historical bug.

## Stability and evolution

An accepted interface is a tested dependency, not an eternal compatibility
promise. A change must name its reason, affected parents, migration, and proofs.
Keep semantically equivalent implementations behind the same meaningful boundary
only when a concrete use earns them. Avoid speculative plugins or strategy hooks.

Types prevent invalid states when the model permits it. Dynamic topology and
input histories still require validation. A typed selector at a harness boundary
cannot certify arbitrary bit patterns emitted by an internal controller; validate
or define those patterns at the component boundary before architectural effects.
