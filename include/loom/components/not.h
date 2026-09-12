#pragma once

#include "loom/structure/ports.h"

namespace loom::components {
// NOT is the first allowlisted behavior atom. It has no state or local clock:
// its output is the complement of its input in the same pure observation.
class Not final {
public:
  const std::string name;
  explicit Not(std::string label) : name(std::move(label)) {}
  const structure::Input<1> &input() const { return input_; }
  const structure::Output<1> &output() const { return output_; }

private:
  const structure::Input<1> input_;
  const structure::Output<1> output_;
};
} // namespace loom::components
