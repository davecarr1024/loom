#pragma once

#include "loom/components/and.h"
#include "loom/components/not.h"
#include "loom/components/or.h"
#include "loom/structure/ports.h"
#include <tuple>

namespace loom::components {
// XOR is the exclusive-or circuit a&!b | !a&b, built only from accepted gates.
class Xor final {
public:
  const std::string name;
  explicit Xor(std::string label) : name(std::move(label)) {}

  const structure::ExternalInput &left() const { return left_; }
  const structure::ExternalInput &right() const { return right_; }
  const structure::Output<1> &output() const { return result_.output(); }
  auto children() const {
    return std::tie(left_, right_, invert_left_, invert_right_, left_only_,
                    right_only_, result_);
  }
  auto connections() const {
    return std::tuple{
        structure::connect(left_.output(), invert_left_.input()),
        structure::connect(right_.output(), invert_right_.input()),
        structure::connect(left_.output(), left_only_.left()),
        structure::connect(invert_right_.output(), left_only_.right()),
        structure::connect(invert_left_.output(), right_only_.left()),
        structure::connect(right_.output(), right_only_.right()),
        structure::connect(left_only_.output(), result_.left()),
        structure::connect(right_only_.output(), result_.right())};
  }

private:
  const structure::ExternalInput left_{"left"}, right_{"right"};
  const Not invert_left_{"invert_left"}, invert_right_{"invert_right"};
  const And left_only_{"left_only"}, right_only_{"right_only"};
  const Or result_{"result"};
};
} // namespace loom::components
