#pragma once

#include "loom/components/and.h"
#include "loom/components/not.h"
#include "loom/components/or.h"
#include "loom/components/selection_contract.h"
#include "loom/structure/ports.h"
#include <cstddef>
#include <tuple>

namespace loom::components {
// A one-bit mux selects when_true for select=1 and when_false for select=0.
class MuxBit final {
public:
  static constexpr std::size_t width = 1;
  const std::string name;
  explicit MuxBit(std::string label) : name(std::move(label)) {}

  const structure::ExternalInput &select() const { return select_; }
  const structure::ExternalInput &when_false() const { return when_false_; }
  const structure::ExternalInput &when_true() const { return when_true_; }
  const structure::Output<1> &output() const { return result_.output(); }
  auto children() const {
    return std::tie(select_, when_false_, when_true_, invert_select_,
                    pass_false_, pass_true_, result_);
  }
  auto connections() const {
    return std::tuple{
        structure::connect(select_.output(), invert_select_.input()),
        structure::connect(when_false_.output(), pass_false_.left()),
        structure::connect(invert_select_.output(), pass_false_.right()),
        structure::connect(when_true_.output(), pass_true_.left()),
        structure::connect(select_.output(), pass_true_.right()),
        structure::connect(pass_false_.output(), result_.left()),
        structure::connect(pass_true_.output(), result_.right())};
  }

private:
  const structure::ExternalInput select_{"select"}, when_false_{"when_false"},
      when_true_{"when_true"};
  const Not invert_select_{"invert_select"};
  const And pass_false_{"pass_false"}, pass_true_{"pass_true"};
  const Or result_{"result"};
};
} // namespace loom::components

static_assert(loom::components::SelectionContract<loom::components::MuxBit, 1>);
