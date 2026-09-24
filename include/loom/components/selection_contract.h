#pragma once

#include "loom/structure/wire_bundle.h"
#include <concepts>
#include <cstddef>
#include <type_traits>

namespace loom::components {

template <class Port>
struct SelectionPortWidth : std::integral_constant<std::size_t, 0> {};
template <std::size_t Width, class Port>
struct SelectionPortWidth<structure::WireBundle<Width, Port>>
    : std::integral_constant<std::size_t, Width> {};
template <>
struct SelectionPortWidth<structure::ExternalInput>
    : std::integral_constant<std::size_t, 1> {};
template <>
struct SelectionPortWidth<structure::Output<1>>
    : std::integral_constant<std::size_t, 1> {};

// A selector declares its data width and exposes false/true inputs and output
// ports of that width under one shared one-bit select boundary.
template <class Mux, std::size_t Width>
concept SelectionContract = (Width > 0) && requires(const Mux &mux) {
  requires Mux::width == Width;
  { mux.select() } -> std::same_as<const structure::ExternalInput &>;
  requires SelectionPortWidth<
               std::remove_cvref_t<decltype(mux.when_false())>>::value == Width;
  requires SelectionPortWidth<
               std::remove_cvref_t<decltype(mux.when_true())>>::value == Width;
  requires SelectionPortWidth<
               std::remove_cvref_t<decltype(mux.output())>>::value == Width;
};

} // namespace loom::components
