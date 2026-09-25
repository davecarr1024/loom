#pragma once

#include "loom/components/full_adder.h"
#include "loom/structure/wire_bundle.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A fixed-width, least-significant-bit-first ripple adder. Carry-out from each
// FullAdder drives carry-in of the next bit; the final carry is explicit.
template <std::size_t Width> class RippleAdder final {
  static_assert(Width > 0, "ripple adder width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  explicit RippleAdder(std::string label)
      : name(std::move(label)),
        left_(make_inputs("left", std::make_index_sequence<Width>{})),
        right_(make_inputs("right", std::make_index_sequence<Width>{})),
        carry_in_{"carry_in"},
        bits_(make_adders(std::make_index_sequence<Width>{})) {}

  const auto &left_ports() const { return left_; }
  const auto &right_ports() const { return right_; }
  const structure::ExternalInput &carry_in() const { return carry_in_; }
  auto sum() const { return sum_bundle(std::make_index_sequence<Width>{}); }
  const structure::Output<1> &carry_out() const {
    return bits_[Width - 1].carry_out();
  }

  auto children() const {
    return std::tuple_cat(
        std::tie(carry_in_),
        input_children(left_, std::make_index_sequence<Width>{}),
        input_children(right_, std::make_index_sequence<Width>{}),
        adder_children(std::make_index_sequence<Width>{}));
  }

  auto connections() const {
    return std::tuple_cat(
        std::tuple{structure::connect(carry_in_.output(),
                                      bits_[0].carry_in().input())},
        operand_connections(std::make_index_sequence<Width>{}),
        carry_connections(std::make_index_sequence<Width - 1>{}));
  }

private:
  const std::array<structure::ExternalInput, Width> left_;
  const std::array<structure::ExternalInput, Width> right_;
  const structure::ExternalInput carry_in_;
  const std::array<FullAdder, Width> bits_;

  template <std::size_t... Index>
  static auto make_inputs(const std::string &prefix,
                          std::index_sequence<Index...>) {
    return std::array<structure::ExternalInput, Width>{
        structure::ExternalInput{prefix + "_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  static auto make_adders(std::index_sequence<Index...>) {
    return std::array<FullAdder, Width>{
        FullAdder{"bit_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  auto input_children(const std::array<structure::ExternalInput, Width> &ports,
                      std::index_sequence<Index...>) const {
    return std::tie(ports[Index]...);
  }

  template <std::size_t... Index>
  auto adder_children(std::index_sequence<Index...>) const {
    return std::tie(bits_[Index]...);
  }

  template <std::size_t... Index>
  auto sum_bundle(std::index_sequence<Index...>) const {
    return structure::wire_bundle(bits_[Index].sum()...);
  }

  template <std::size_t... Index>
  auto operand_connections(std::index_sequence<Index...>) const {
    return std::tuple_cat(
        std::tuple{structure::connect(left_[Index].output(),
                                      bits_[Index].left().input())...},
        std::tuple{structure::connect(right_[Index].output(),
                                      bits_[Index].right().input())...});
  }

  template <std::size_t... Index>
  auto carry_connections(std::index_sequence<Index...>) const {
    return std::tuple{structure::connect(
        bits_[Index].carry_out(), bits_[Index + 1].carry_in().input())...};
  }
};

} // namespace loom::components
