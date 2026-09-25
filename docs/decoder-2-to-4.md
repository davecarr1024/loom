# Two-to-four one-hot decoder

`components::Decoder2To4` maps a two-bit address to four one-hot outputs. Address
bit 0 is least significant, so exactly output `address` is high for every
address from 0 through 3. There are no reserved encodings.

The decoder is a composite of two NOT gates and four AND gates. Each AND gets
the direct or inverted form of both address bits required for its output. The
simulator has no decoder behavior branch. Its address and output bundles retain
the identity of the existing scalar ports.

The tests check each address, both intermediate inverted bits, all four gate
outputs, the derived inventory and schedule, and routing into a parent's output
boundaries. Run `bazel run //:decoder2_to4_demo` for the complete one-hot table.
This completes selection and bit-bundle group B; see the [roadmap](roadmap.md)
for the word-register milestone next.
