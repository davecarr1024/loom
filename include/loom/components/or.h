#pragma once

#include "loom/structure/ports.h"

namespace loom::components {
// OR is a pure two-input behavior atom. Its input ports retain distinct
// identities even though both carry the same one-bit value type.
class Or final {
public:
  const std::string name;
  explicit Or(std::string label) : name(std::move(label)) {}
  const structure::Input<1> &left() const { return left_; }
  const structure::Input<1> &right() const { return right_; }
  const structure::Output<1> &output() const { return output_; }

private:
  const structure::Input<1> left_;
  const structure::Input<1> right_;
  const structure::Output<1> output_;
};
} // namespace loom::components
