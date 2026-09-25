# Two-register bank with one read and one write port

## Contract

`components::RegisterBank2<Width>` stores two fixed-width words, with bit 0 as
the least-significant bit. It has one combinational read port and one
synchronous write port. `RegisterAddress::{zero, one}` selects a register for
the read or write address. `RegisterWriteSource::{external_input,
selected_register}` selects whether write data comes from the explicit external
input bundle or from the currently selected register on the read bus. Each
enum choice maps to one bit pattern, and both patterns are valid. There is no
idle address, invalid selector, tri-state value, or retained undriven bus.

The read bus selects the two stored Q words. A second selected bus chooses the
external write-data word or the read-bus word, and its output fans out to both
register data inputs. The write address and write enable form each register's
load control through NOT/AND children. The bank therefore has one write port:
at most one register changes per edge, while either register may be observed.
Initial contents are passed to the two child `EnabledWordRegister`s; no shadow
host-side storage exists.

Read output is combinational from the selected register's current Q state. On
a shared edge, write data is evaluated from the same pre-edge Q values, so a
register-to-register copy reads the old source value. When write enable is
false, both registers hold. Missing, duplicated, foreign, or otherwise invalid
external bindings are rejected by the simulation boundary before any state or
edge index changes.

## Evidence

`tests/register_bank_2_test.cpp` checks all source/destination address pairs,
external and internal write-data selection, load and hold, old-value reads on
an edge, output routing, independent simulations, invalid-input atomicity, and
a parent that accidentally connects two drivers to one output bit.
The derived inventory contains eight DFFs, 17 NOT gates, 34 AND gates, and 16
OR gates, including both selected buses, register enables, and storage. A parent
test connects the read port to an output boundary. Run
`bazel run //:register_bank_2_demo` for a register-to-register transfer trace.
