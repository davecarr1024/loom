#pragma once

#include "loom/structure/ports.h"

namespace loom::components {
// AND is a pure two-input behavior atom. Both inputs retain their own port
// identity even though they carry the same one-bit value type.
class And final {
public:
  const std::string name;
  explicit And(std::string label) : name(std::move(label)) {}
  const structure::Input<1> &left() const { return left_; }
  const structure::Input<1> &right() const { return right_; }
  const structure::Output<1> &output() const { return output_; }

private:
  const structure::Input<1> left_;
  const structure::Input<1> right_;
  const structure::Output<1> output_;
};
} // namespace loom::components
