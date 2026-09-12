# NOT: the first accepted construction component

NOT and its typed observation harness implement the first component in
[roadmap group A](roadmap.md). AND, OR, constants, and DFFs remain next work;
this is not completion of the entire atomic floor. The register-only
[Phase 1](phase-1.md) API remains a separate transitional baseline.

## Contract and public interface

`loom::components::Not` owns one `structure::Input<1>` and one
`structure::Output<1>`. Its output is the Boolean complement of its input in
the same observation. It has no stored state, initialization bits, latency,
local clock, or behavioral callback. Simulation admits this exact final type;
a custom type with an `evaluate` function is not an atom.

`value::Bit` explicitly accepts a bool. It does not implicitly convert integers
or silently mask numeric input. `structure::connect(output, input)` requires
matching widths and roles. Ports are non-copyable/non-movable owned objects;
connections retain the actual port identities rather than identifying a source
by its value type or diagnostic name.

`structure::ExternalInput` exposes an output to the circuit and creates a typed
`Binding` via `bind(Bit{false})`. `structure::ExternalOutput` exposes an input
sink to the circuit. These boundary objects route bits; they are not additional
behavior atoms. Multiple boundary inputs, nesting, and fan-out are supported.
A parent's meaningful interface may return references to its owned child ports.

A composite owns its children and exposes const tuples of lvalue references
through `children()` and typed connections through `connections()`. Its name
is nonempty ASCII letters/digits/underscore/hyphen; dots delimit hierarchy.
Composite names are declared as `std::string` values, matching the validation
interface. A composite has no simulator callback. The public example in
[examples/not.cpp](../examples/not.cpp) connects two NOTs through that protocol.

`simulation::Definition<Root>::create(...)` constructs the immutable root at
its final address and returns an expected shared pointer to a const definition.
Use `definition->root()` to bind the same external-input instances discovered
by finalization. The definition cannot move or copy. Retain the definition
while using its port references or bindings; bindings do not own a definition.

## Observation and initialization

The stateless harness observes directly through the const definition:

```cpp
const auto inputs = std::array{
    definition->root().input.bind(loom::value::Bit{false})};
const auto observation = definition->observe(inputs);
```

There is no evolving simulation object or step operation until state atoms are
introduced. The committed state is empty and every observation is before edge 0.
All evaluation buffers belong to the call. Both truth-table inputs can be queried
immediately after finalization, repeatedly, without retaining a current input.
An empty circuit accepts an empty snapshot. Missing, duplicated, or foreign
bindings return structured errors; an omitted binding cannot reuse an earlier
value. Returned observations own every signal path and bit value.

This implements the stateless part of [timing](timing.md)'s transient-observation
contract. Observation-versus-edge-0 equivalence and post-commit state tests remain
requirements for the DFF component, not claims about this component.

## Derived structure and execution

The modules have acyclic dependencies: `value/bit.h`, `structure/ports.h`,
`components/not.h`, then `simulation/logic.h`, with matching namespaces. The
simulation module recognizes exact supported types and owns NOT's semantics.
It does not accept arbitrary callables or replace composites with shortcuts.

Ownership discovery records each node, its kind, and its actual input/output
port objects. It rejects repeated ownership before recursion can revisit an
ancestor. Finalization sorts diagnostic paths, rejects duplicate paths, resolves
connections by port membership, and requires exactly one driver per input.
Foreign and null endpoints fail before evaluation.

For each NOT or output sink, the derived driver identifies the preceding
output. External inputs are ready sources. Repeatedly select the ready node
with the lexically first path and evaluate it; if none is ready, follow the
unresolved predecessor chain into a cycle and report an input on that cycle.
The schedule includes output sinks because they copy a routed bit. Authored
child and wire enumeration do not determine execution order.

`inventory()`, `connections()`, and `schedule()` expose immutable derived
records, cached during finalization. Observation records every actual leaf port,
including intermediate gate inputs and outputs, in canonical path order.
The example formats those records without recomputing the circuit's behavior.
No second hierarchy or separately authored execution schedule exists.

The structural protocol still requires honest ownership: `children()` must
reference actual stable owned children and `connections()` their stable ports.
C++ concepts cannot prove that an arbitrary user-written function returns a
live subobject. The implementation checks discovered membership and duplicate
identity; it does not claim to prove arbitrary C++ lifetime behavior. Module
`detail` types are implementation internals, not an alternate construction API.

## Proof and artifact

`tests/not_test.cpp` proves the single-gate truth table, call-scoped inputs,
retained observations, rejected bindings, two-gate composition, intermediate
fan-out, derived inventory/schedule, reversed child and wire order, nested
independent inputs, retained definition lifetime, invalid names/ownership/wiring,
self-feedback, and a multi-node cycle with a blocked downstream output. A
documented test-only mutation bypasses the second NOT at the parent output:
all gate observations still agree, while only that parent boundary differs.
The test localizes the fault to its connection and verifies the corrected
parent for both inputs. This is an injected integration fault, not a historical
production defect.
Compile-fail fixtures reject wrong widths, reversed roles, numeric bit construction,
and custom behavior atoms for the intended diagnostics.

Run `make check` and `./build/not_demo`. The two observations include:

```text
observe input=0 (before edge 0)
  double_not.first.in=0
  double_not.first.out=1
  double_not.input.out=0
  double_not.output.in=0
  double_not.second.in=1
  double_not.second.out=0
observe input=1 (before edge 0)
  double_not.first.in=1
  double_not.first.out=0
  double_not.input.out=1
  double_not.output.in=1
  double_not.second.in=0
  double_not.second.out=1
```

The executable also prints the owned inventory and connection paths. The
containing-circuit test uses reversed lexical gate names to prove that it is
the connections, rather than naming or visitation, that determine values.

## Checkpoint

What became visible: the parent output and the intermediate inversion are two
views of one circuit evaluation. What became load-bearing: stable port identity,
complete call-scoped input snapshots, and a schedule derived from actual wires.

The type-family boundary is modest: typed port roles/widths, exact behavior atoms,
and a structural composite protocol. NOT is not interchangeable with a boundary
source/sink despite all carrying one bit. There is no generic behavior interface
to bypass the atom floor. Richer capability/refinement families remain for the
concrete consumers in the roadmap.

The next component is AND, through the same definition and observation boundary.
It must extend the fixed atom semantics and multi-input dependency representation
with a truth table and a containing circuit; it must not introduce a generic
callback registry or prematurely implement the remaining floor.
