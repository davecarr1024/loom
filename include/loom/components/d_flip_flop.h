#pragma once

#include "loom/structure/ports.h"
#include "loom/value/bit.h"

namespace loom::components {
// A DFF exposes initialized Q state and samples D on the shared simulator edge.
class DFlipFlop final {
public:
  const std::string name;
  const value::Bit initial_value;
  DFlipFlop(std::string label, value::Bit initial)
      : name(std::move(label)), initial_value(initial) {}
  const structure::Input<1> &data() const { return data_; }
  const structure::Output<1> &output() const { return output_; }

private:
  const structure::Input<1> data_;
  const structure::Output<1> output_;
};
} // namespace loom::components
