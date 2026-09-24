# Fixed-width wire bundles

## Contract

`structure::InputBundle<Width>` and `OutputBundle<Width>` group existing
one-bit ports into an ordered, non-owning view. A bundle owns no components,
ports, values, or simulation state. Its references retain the identity of the
individual endpoints in the circuit tree.

Bit index 0 is the least-significant bit. `wire_bundle(p0, p1, ...)` preserves
the listed order. `concat(low, high)` places `low` at indices starting at 0 and
`high` immediately above it. `split<LowWidth>(bundle)` returns the lower bits
first and the remaining higher bits second; both widths must be positive.
`bundle.at(index)` returns a reference to the requested port or a
`BundleIndexError` containing the invalid index and bundle width.

`connect(OutputBundle<W>, InputBundle<W>)` requires equal widths at compile
time. It expands to W ordinary `Connection<1>` values. Finalization validates
each endpoint against owned component identity, then reports, schedules,
observes, and traces the same scalar wires it would have seen if each had been
connected individually. Bundles introduce no simulator atom or evaluation
behavior.

## Evidence

`tests/wire_bundle_test.cpp` checks one-bit and four-bit bundles, checked index
failure, concat/split identity and ordering, value propagation through an
assembled four-bit pass-through, scalar connection paths, bundle fan-out, and
independent same-width instances. Compile-fail cases prove that mismatched
widths and reversed port roles cannot connect.

Run `bazel run //:wire_bundle_demo` for a four-bit pass-through example. The
input/output bits use the documented least-significant-first index order.
