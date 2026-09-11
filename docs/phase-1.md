# Phase 1: first register-only circuit

## Implemented interface

`Register<W>` declares a named register of width 1..64 with reset bits masked
to its width. `connect(source, destination)` requires equal register widths at
compile time. Each register currently has one data connection; self-connection
is an explicit way to retain a source register's value.

A composite owns its objects and exposes tuples of references through
`children()` and typed wires through `connections()`. `Definition<Root>::create`
constructs that root in stable owned storage, derives the register inventory
and wires, sorts diagnostic paths, and validates driver completeness and
membership. Connections use object identity, not width or names, to bind.
The derived inventory is an execution plan, not another authored hierarchy.

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
diagnostics, retained evidence, and atomic input failure. `wrong_width.cpp`
must fail compilation with a conflicting `connect` deduction diagnostic.
`examples/transfer.cpp` is the first headless artifact, transferring 42.

Run `make check`, or separately `make test`, `make coverage`, `make format`.
Then run `./build/transfer`. Local tools are GCC 12.2, CMake, Python 3.11,
lcov 1.16, and clang-format 14. GoogleTest v1.14.0 is fetched by CMake;
an offline checkout can be supplied with
`CMAKE_ARGS=-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/absolute/path/to/googletest`.
Clang-tidy below 16 is explicitly skipped due to std::expected frontend
incompatibility, following Rule Lab; newer versions must run successfully.

Coverage measures instantiated production header lines/functions, excluding
tests, third-party code, and the example CLI adapter. It does not prove every
possible template instantiation. No production coverage exclusions are used.

## Remaining Phase 1 work

The first vocabulary is register-only. Before declaring the phase complete,
tighten structural concepts and name validation, expose explicit input/output
endpoint roles, add compile-time propagated register/port facts, and prove a
register chain and fan-out through the same interface. The initial single
header should split by domain as those boundaries become concrete.

An assembly must expose actual owned child references and stable endpoint
references; arbitrary user implementations of the structural protocol are
currently trusted to obey that lifetime contract. General combinational nodes,
external data ports, and microcode controls are not implemented. Enabled paths
are a narrow scripting adapter; they do not replace the future typed controls.
