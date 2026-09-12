# Phase 1: independently composed register circuits

This is the retained register-only baseline from the original roadmap. It
proves ownership and atomic edges but does not implement the restricted atom
floor in the revised [design](design.md). The construction plan supersedes the
old phase sequence; [roadmap](roadmap.md) owns the next work.

## Implemented interface

`Register<W>` declares a named register of width 1..64 with reset bits masked
to its width. `connect(source, destination)` requires equal register widths at
compile time. Explicit `output()` and `input()` endpoints additionally reject
reversed roles. The register-to-register overload remains a compatibility
adapter through those same endpoints. Each register has one data connection; self-connection
is an explicit way to retain a source register's value.

A composite owns its objects and exposes tuples of references through
`children()` and typed wires through `connections()`. `Definition<Root>::create`
constructs that root in stable owned storage, derives the register inventory
and wires, sorts diagnostic paths, and validates driver completeness and
membership. Connections use object identity, not width or names, to bind.
The derived inventory is an execution plan, not another authored hierarchy.

`structure.h` owns registers, endpoints, structural concepts, and type-only
`CircuitFacts<T>`; `circuit.h` owns finalization, simulation, and edge evidence.
`CircuitFacts` adds register counts through the actual child tuple types and
derives one data input/output per register. Connections do not duplicate those
counts. The current facts concern register-only assemblies, not future nodes.
Root references/pointers and child tuples containing copied values are rejected
by concepts. Names are nonempty ASCII letters/digits/underscore/hyphen; dots
are reserved for hierarchy. Bad names at any depth fail finalization.

`Simulation<Root>::create` retains shared const-definition ownership. `step`
takes a span of enabled register paths; omitted registers hold. Invalid or
duplicate enable paths reject the edge before mutation. The operation snapshots
old values, prepares all new values, and commits them together. Returned `Edge`
records contain owned before/after samples and the zero-based edge index.

An optional edge budget bounds a run and prevents cycle-index wraparound. A
budget exhaustion is an unfinished run, not a hardware deadlock claim.

## Evidence and commands

`tests/circuit_test.cpp` exhaustively tests all 256 four-bit register pairs and
checks nesting, child-order independence, separate simulations, topology
diagnostics, retained evidence, atomic input failure, fan-out, and a chain that
advances only one register per edge. Static assertions prove nested graph facts,
root/child constraints, and incompatible endpoint roles. `wrong_width.cpp`
must fail compilation with a conflicting `connect` deduction diagnostic.
`examples/transfer.cpp` is the first headless artifact, transferring 42.

Run `make check`, or separately `make test`, `make coverage`, `make format`.
Then run `./build/transfer`. Local tools are GCC 12.2, CMake, Python 3.11,
lcov 1.16, and clang-format 14. GoogleTest v1.14.0 is fetched by CMake.
CI pairs GCC 12 with `GCOV=gcov-12`; `gcov` must match the compiler that
produced the instrumentation. The first CI run exposed a default-gcov-11
mismatch after its behavioral tests passed. Coverage accepts a `GCOV` override.
An offline checkout can be supplied with
`CMAKE_ARGS=-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/absolute/path/to/googletest`.
Clang-tidy below 16 is explicitly skipped due to std::expected frontend
incompatibility, following Rule Lab; newer versions must run successfully.

Coverage measures instantiated production header lines/functions, excluding
tests, third-party code, and the example CLI adapter. It does not prove every
possible template instantiation. No production coverage exclusions are used.

## Phase checkpoint and limits

Phase 1 now has its register-only execution proof, typed endpoints, structural
concepts, propagated facts, nested composition, invalid-wiring diagnostics,
headless artifact, and coverage/toolchain gates. [NOT](not.md) now implements
typed external ports and pure observation in the new construction model. Later, composed DFF registers
replace atomic wide registers while retaining these behavioral proofs.

What became visible: the same-width registers can swap or form a pipeline
chain solely by changing connections, while edge semantics remain unchanged.
What became load-bearing: stable definition ownership and snapshot reads.
Do not generalize the small register-count trait into a property framework yet.

An assembly must expose actual owned child references and stable endpoint
references; arbitrary user implementations of the structural protocol are
currently trusted to obey that lifetime contract. This transitional API has
no combinational nodes, external data ports, or microcode controls; the separate
NOT construction API supplies the new combinational boundary. Enabled paths
are a narrow scripting adapter; they do not replace the future typed controls.
