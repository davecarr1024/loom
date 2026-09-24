#pragma once

#include "loom/structure/ports.h"
#include "loom/value/bit.h"

namespace loom::components {
// A constant bit is an immutable, inputless source in a circuit.
class ConstantBit final {
public:
  const std::string name;
  const value::Bit fixed_value;
  ConstantBit(std::string label, value::Bit value)
      : name(std::move(label)), fixed_value(value) {}
  const structure::Output<1> &output() const { return output_; }

private:
  const structure::Output<1> output_;
};
} // namespace loom::components
