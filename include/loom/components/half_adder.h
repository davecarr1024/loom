#pragma once

#include "loom/components/and.h"
#include "loom/components/xor.h"
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A one-bit half adder: sum is XOR and carry is AND of the two operands.
class HalfAdder final {
public:
  const std::string name;

  explicit HalfAdder(std::string label)
      : name(std::move(label)), sum_{"sum"}, carry_{"carry"} {}

  const structure::ExternalInput &left() const { return left_; }
  const structure::ExternalInput &right() const { return right_; }
  const structure::Output<1> &sum() const { return sum_.output(); }
  const structure::Output<1> &carry() const { return carry_.output(); }

  auto children() const { return std::tie(left_, right_, sum_, carry_); }
  auto connections() const {
    return std::tuple{structure::connect(left_.output(), sum_.left().input()),
                      structure::connect(right_.output(), sum_.right().input()),
                      structure::connect(left_.output(), carry_.left()),
                      structure::connect(right_.output(), carry_.right())};
  }

private:
  const structure::ExternalInput left_{"left"};
  const structure::ExternalInput right_{"right"};
  const Xor sum_;
  const And carry_;
};

} // namespace loom::components
