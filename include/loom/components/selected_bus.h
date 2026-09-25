#pragma once

#include "loom/components/mux_word.h"
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A two-source, fixed-width bus. Select low chooses source_zero; select high
// chooses source_one. Both selector values are defined, so there is no idle or
// undriven state. The bus is ordinary mux logic, not tri-state resolution.
template <std::size_t Width> class SelectedBus final {
  static_assert(Width > 0, "selected bus width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  explicit SelectedBus(std::string label)
      : name(std::move(label)), selector_{"selector"} {}

  const structure::ExternalInput &select_port() const {
    return selector_.select();
  }
  const auto &source_zero_ports() const { return selector_.when_false_ports(); }
  const auto &source_one_ports() const { return selector_.when_true_ports(); }
  auto source_zero() const { return selector_.when_false(); }
  auto source_one() const { return selector_.when_true(); }
  auto output() const { return selector_.output(); }

  auto children() const { return std::tie(selector_); }
  auto connections() const { return std::tuple{}; }

private:
  const MuxWord<Width> selector_;
};

} // namespace loom::components
