#pragma once

#include "loom/components/mux_word.h"
#include "loom/components/word_contracts.h"
#include "loom/components/word_register.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A word register with an explicit synchronous load enable. False selects the
// current Q word and true selects data, through the owned MuxWord child.
template <std::size_t Width> class EnabledWordRegister final {
  static_assert(Width > 0, "enabled word register width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  EnabledWordRegister(std::string label, std::array<value::Bit, Width> initial)
      : name(std::move(label)), selector_{"selector"},
        storage_{"storage", initial} {}

  const structure::ExternalInput &enable_port() const {
    return selector_.select();
  }
  const auto &data_ports() const { return selector_.when_true_ports(); }
  const structure::Input<1> &enable_input() const {
    return selector_.select().input();
  }
  auto data_input() const {
    return input_bundle(selector_.when_true_ports(),
                        std::make_index_sequence<Width>{});
  }
  auto output() const { return storage_.output(); }

  auto children() const { return std::tie(selector_, storage_); }

  auto connections() const {
    return std::tuple{
        structure::connect(storage_.output(),
                           input_bundle(selector_.when_false_ports(),
                                        std::make_index_sequence<Width>{})),
        structure::connect(selector_.output(), storage_.data_input())};
  }

private:
  const MuxWord<Width> selector_;
  const WordRegister<Width> storage_;

  template <std::size_t... Index>
  auto input_bundle(const std::array<structure::ExternalInput, Width> &ports,
                    std::index_sequence<Index...>) const {
    return structure::wire_bundle(ports[Index].input()...);
  }
};

static_assert(ReadableWord<WordRegister<4>, 4>);
static_assert(ReadableWord<EnabledWordRegister<4>, 4>);
static_assert(EnabledWord<EnabledWordRegister<4>, 4>);
static_assert(!EnabledWord<WordRegister<4>, 4>);

} // namespace loom::components
