#pragma once

#include "loom/components/half_adder.h"
#include "loom/components/or.h"
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A one-bit full adder built from two half adders and an OR carry merge.
class FullAdder final {
public:
  const std::string name;

  explicit FullAdder(std::string label)
      : name(std::move(label)), first_{"first"}, second_{"second"},
        carry_merge_{"carry_merge"} {}

  const structure::ExternalInput &left() const { return left_; }
  const structure::ExternalInput &right() const { return right_; }
  const structure::ExternalInput &carry_in() const { return carry_in_; }
  const structure::Output<1> &sum() const { return second_.sum(); }
  const structure::Output<1> &carry_out() const {
    return carry_merge_.output();
  }

  auto children() const {
    return std::tie(left_, right_, carry_in_, first_, second_, carry_merge_);
  }
  auto connections() const {
    return std::tuple{
        structure::connect(left_.output(), first_.left().input()),
        structure::connect(right_.output(), first_.right().input()),
        structure::connect(first_.sum(), second_.left().input()),
        structure::connect(carry_in_.output(), second_.right().input()),
        structure::connect(first_.carry(), carry_merge_.left()),
        structure::connect(second_.carry(), carry_merge_.right())};
  }

private:
  const structure::ExternalInput left_{"left"};
  const structure::ExternalInput right_{"right"};
  const structure::ExternalInput carry_in_{"carry_in"};
  const HalfAdder first_;
  const HalfAdder second_;
  const Or carry_merge_;
};

} // namespace loom::components
