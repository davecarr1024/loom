#pragma once

#include "loom/components/enabled_word_register.h"
#include "loom/components/word_contracts.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// Shifts toward increasing bit indices: old bit i moves to i+1, and serial
// input enters bit 0. Enable low holds the current word.
template <std::size_t Width> class ShiftRegister final {
  static_assert(Width > 0, "shift register width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  ShiftRegister(std::string label, std::array<value::Bit, Width> initial)
      : name(std::move(label)), storage_{"storage", initial} {}

  const structure::ExternalInput &enable_port() const { return enable_; }
  const structure::ExternalInput &serial_port() const { return serial_; }
  const structure::Input<1> &enable_input() const { return enable_.input(); }
  const structure::Input<1> &serial_input() const { return serial_.input(); }
  auto output() const { return storage_.output(); }

  auto children() const { return std::tie(enable_, serial_, storage_); }

  auto connections() const {
    return std::tuple_cat(
        std::tuple{
            structure::connect(enable_.output(), storage_.enable_input())},
        shifted_data_connections(std::make_index_sequence<Width - 1>{}));
  }

private:
  const structure::ExternalInput enable_{"shift_enable"};
  const structure::ExternalInput serial_{"serial_in"};
  const EnabledWordRegister<Width> storage_;

  template <std::size_t... Index>
  auto shifted_data_connections(std::index_sequence<Index...>) const {
    const auto next_data = storage_.data_input();
    const auto current = storage_.output();
    return std::tuple_cat(
        std::tuple{
            structure::connect(serial_.output(), next_data.bits()[0].get())},
        std::tuple{structure::connect(current.bits()[Index].get(),
                                      next_data.bits()[Index + 1].get())...});
  }
};

static_assert(ReadableWord<ShiftRegister<4>, 4>);
static_assert(ShiftableWord<ShiftRegister<4>, 4>);

} // namespace loom::components
