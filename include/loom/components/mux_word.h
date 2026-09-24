#pragma once

#include "loom/components/mux_bit.h"
#include "loom/components/selection_contract.h"
#include "loom/structure/wire_bundle.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A fixed-width selector. Bit 0 is the least-significant bit; every output bit
// is selected by the shared one-bit select input through its own MuxBit.
template <std::size_t Width> class MuxWord final {
  static_assert(Width > 0, "word mux width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  explicit MuxWord(std::string label)
      : name(std::move(label)), when_false_(make_inputs("when_false")),
        when_true_(make_inputs("when_true")),
        bits_(make_bits(std::make_index_sequence<Width>{})) {}

  const structure::ExternalInput &select() const { return select_; }
  const auto &when_false_ports() const { return when_false_; }
  const auto &when_true_ports() const { return when_true_; }
  auto when_false() const { return input_bundle(when_false_); }
  auto when_true() const { return input_bundle(when_true_); }
  auto output() const {
    return output_bundle(std::make_index_sequence<Width>{});
  }

  auto children() const {
    return children_impl(std::make_index_sequence<Width>{});
  }

  auto connections() const {
    return connections_impl(std::make_index_sequence<Width>{});
  }

private:
  const structure::ExternalInput select_{"select"};
  const std::array<structure::ExternalInput, Width> when_false_;
  const std::array<structure::ExternalInput, Width> when_true_;
  const std::array<MuxBit, Width> bits_;

  static auto make_inputs(const std::string &prefix) {
    return make_inputs_impl(prefix, std::make_index_sequence<Width>{});
  }

  template <std::size_t... Index>
  static auto make_inputs_impl(const std::string &prefix,
                               std::index_sequence<Index...>) {
    return std::array<structure::ExternalInput, Width>{
        structure::ExternalInput{prefix + "_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  static auto make_bits(std::index_sequence<Index...>) {
    return std::array<MuxBit, Width>{MuxBit{"bit_" + std::to_string(Index)}...};
  }

  auto
  input_bundle(const std::array<structure::ExternalInput, Width> &ports) const {
    return input_bundle_impl(ports, std::make_index_sequence<Width>{});
  }

  template <std::size_t... Index>
  auto
  input_bundle_impl(const std::array<structure::ExternalInput, Width> &ports,
                    std::index_sequence<Index...>) const {
    return structure::wire_bundle(ports[Index].output()...);
  }

  template <std::size_t... Index>
  auto output_bundle(std::index_sequence<Index...>) const {
    return structure::wire_bundle(bits_[Index].output()...);
  }

  template <std::size_t... Index>
  auto children_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(std::tie(select_), std::tie(when_false_[Index]...),
                          std::tie(when_true_[Index]...),
                          std::tie(bits_[Index]...));
  }

  template <std::size_t... Index>
  auto connections_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(
        std::tuple{structure::connect(select_.output(),
                                      bits_[Index].select().input())...},
        std::tuple{structure::connect(when_false_[Index].output(),
                                      bits_[Index].when_false().input())...},
        std::tuple{structure::connect(when_true_[Index].output(),
                                      bits_[Index].when_true().input())...});
  }
};

static_assert(SelectionContract<MuxBit, 1>);
static_assert(SelectionContract<MuxWord<4>, 4>);

} // namespace loom::components
