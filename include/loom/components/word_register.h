#pragma once

#include "loom/components/d_flip_flop.h"
#include "loom/structure/wire_bundle.h"
#include "loom/value/bit.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// An edge-loaded word stored as one initialized DFF per least-significant-first
// bit. It has no enable; every valid edge samples the complete input word.
template <std::size_t Width> class WordRegister final {
  static_assert(Width > 0, "word register width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  WordRegister(std::string label, std::array<value::Bit, Width> initial)
      : name(std::move(label)),
        data_(make_data_impl(std::make_index_sequence<Width>{})),
        bits_(make_bits(initial, std::make_index_sequence<Width>{})) {}

  const auto &data_ports() const { return data_; }
  auto data_input() const {
    return input_bundle(std::make_index_sequence<Width>{});
  }
  auto output() const {
    return output_bundle(std::make_index_sequence<Width>{});
  }

  auto children() const {
    return std::tuple_cat(data_children(std::make_index_sequence<Width>{}),
                          bit_children(std::make_index_sequence<Width>{}));
  }

  auto connections() const {
    return connections_impl(std::make_index_sequence<Width>{});
  }

private:
  const std::array<structure::ExternalInput, Width> data_;
  const std::array<DFlipFlop, Width> bits_;

  template <std::size_t... Index>
  static auto make_data_impl(std::index_sequence<Index...>) {
    return std::array<structure::ExternalInput, Width>{
        structure::ExternalInput{"data_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  static auto make_bits(const std::array<value::Bit, Width> &initial,
                        std::index_sequence<Index...>) {
    return std::array<DFlipFlop, Width>{
        DFlipFlop{"bit_" + std::to_string(Index), initial[Index]}...};
  }

  template <std::size_t... Index>
  auto input_bundle(std::index_sequence<Index...>) const {
    return structure::wire_bundle(data_[Index].input()...);
  }

  template <std::size_t... Index>
  auto output_bundle(std::index_sequence<Index...>) const {
    return structure::wire_bundle(bits_[Index].output()...);
  }

  template <std::size_t... Index>
  auto data_children(std::index_sequence<Index...>) const {
    return std::tie(data_[Index]...);
  }

  template <std::size_t... Index>
  auto bit_children(std::index_sequence<Index...>) const {
    return std::tie(bits_[Index]...);
  }

  template <std::size_t... Index>
  auto connections_impl(std::index_sequence<Index...>) const {
    return std::tuple{
        structure::connect(data_[Index].output(), bits_[Index].data())...};
  }
};

} // namespace loom::components
