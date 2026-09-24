#pragma once

#include "loom/structure/ports.h"
#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace loom::structure {

// Bundles are non-owning, ordered views over existing scalar ports. Index 0
// is the least-significant bit. The referenced components own every port.
struct BundleIndexError {
  std::size_t index;
  std::size_t width;
  bool operator==(const BundleIndexError &) const = default;
};

template <std::size_t Width, class Port> class WireBundle {
  static_assert(Width > 0, "wire bundle width must be positive");
  static_assert(std::same_as<Port, Input<1>> || std::same_as<Port, Output<1>>,
                "wire bundles contain one-bit input or output ports");

public:
  static constexpr std::size_t width = Width;
  using port_type = Port;

  explicit WireBundle(
      std::array<std::reference_wrapper<const Port>, Width> bits)
      : bits_(std::move(bits)) {}

  std::expected<std::reference_wrapper<const Port>, BundleIndexError>
  at(std::size_t index) const {
    if (index >= Width)
      return std::unexpected(BundleIndexError{index, Width});
    return bits_[index];
  }

  const auto &bits() const { return bits_; }

private:
  std::array<std::reference_wrapper<const Port>, Width> bits_;
};

template <std::size_t Width> using InputBundle = WireBundle<Width, Input<1>>;
template <std::size_t Width> using OutputBundle = WireBundle<Width, Output<1>>;

template <class First, class... Rest>
  requires((std::same_as<std::remove_cvref_t<First>, Input<1>> &&
            (std::same_as<std::remove_cvref_t<Rest>, Input<1>> && ...)) ||
           (std::same_as<std::remove_cvref_t<First>, Output<1>> &&
            (std::same_as<std::remove_cvref_t<Rest>, Output<1>> && ...)))
auto wire_bundle(const First &first, const Rest &...rest) {
  using Port = std::remove_cvref_t<First>;
  return WireBundle<1 + sizeof...(Rest), Port>{
      std::array<std::reference_wrapper<const Port>, 1 + sizeof...(Rest)>{
          std::cref(first), std::cref(rest)...}};
}

template <std::size_t LowWidth, std::size_t HighWidth, class Port,
          std::size_t... Low, std::size_t... High>
auto concat_impl(const WireBundle<LowWidth, Port> &low,
                 const WireBundle<HighWidth, Port> &high,
                 std::index_sequence<Low...>, std::index_sequence<High...>) {
  return WireBundle<LowWidth + HighWidth, Port>{
      std::array<std::reference_wrapper<const Port>, LowWidth + HighWidth>{
          low.bits()[Low]..., high.bits()[High]...}};
}

// The low bundle keeps indices [0, LowWidth); high follows it.
template <std::size_t LowWidth, std::size_t HighWidth, class Port>
auto concat(const WireBundle<LowWidth, Port> &low,
            const WireBundle<HighWidth, Port> &high) {
  return concat_impl(low, high, std::make_index_sequence<LowWidth>{},
                     std::make_index_sequence<HighWidth>{});
}

template <std::size_t LowWidth, std::size_t Width, class Port,
          std::size_t... Low, std::size_t... High>
auto split_impl(const WireBundle<Width, Port> &bundle,
                std::index_sequence<Low...>, std::index_sequence<High...>) {
  return std::pair{
      WireBundle<LowWidth, Port>{
          std::array<std::reference_wrapper<const Port>, LowWidth>{
              bundle.bits()[Low]...}},
      WireBundle<Width - LowWidth, Port>{
          std::array<std::reference_wrapper<const Port>, Width - LowWidth>{
              bundle.bits()[LowWidth + High]...}}};
}

// Split at a compile-time bit index; both returned bundles are nonempty.
template <std::size_t LowWidth, std::size_t Width, class Port>
  requires(LowWidth > 0 && LowWidth < Width)
auto split(const WireBundle<Width, Port> &bundle) {
  return split_impl<LowWidth>(bundle, std::make_index_sequence<LowWidth>{},
                              std::make_index_sequence<Width - LowWidth>{});
}

template <std::size_t Width> struct BundleConnection {
  static_assert(Width > 0, "wire connection bundle width must be positive");
  std::array<Connection<1>, Width> bits;
};

template <std::size_t Width, std::size_t... Index>
auto connect_bundle_impl(const OutputBundle<Width> &source,
                         const InputBundle<Width> &destination,
                         std::index_sequence<Index...>) {
  return BundleConnection<Width>{std::array<Connection<1>, Width>{
      connect(source.bits()[Index].get(), destination.bits()[Index].get())...}};
}

// A bundle connection is only shorthand: it expands to Width ordinary
// instance-specific scalar connections for finalization and simulation.
template <std::size_t Width>
auto connect(const OutputBundle<Width> &source,
             const InputBundle<Width> &destination) {
  return connect_bundle_impl(source, destination,
                             std::make_index_sequence<Width>{});
}

} // namespace loom::structure
