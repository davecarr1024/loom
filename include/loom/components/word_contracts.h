#pragma once

#include "loom/structure/wire_bundle.h"
#include <concepts>
#include <cstddef>

namespace loom::components {

// Shared observable shape used by register consumers: a fixed width and an
// ordered bundle of committed Q outputs.
template <class Component, std::size_t Width>
concept ReadableWord = requires(const Component &component) {
  requires(Component::width == Width);
  { component.output() } -> std::same_as<structure::OutputBundle<Width>>;
};

// Additional typed controls required from an enabled word register.
template <class Component, std::size_t Width>
concept EnabledWord =
    ReadableWord<Component, Width> && requires(const Component &component) {
      { component.enable_input() } -> std::same_as<const structure::Input<1> &>;
      { component.data_input() } -> std::same_as<structure::InputBundle<Width>>;
    };

} // namespace loom::components
