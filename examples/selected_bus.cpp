#include "loom/components/selected_bus.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <ranges>

int main() {
  constexpr std::size_t width = 4;
  const auto definition = loom::simulation::Definition<
      loom::components::SelectedBus<width>>::create("bus");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }

  const auto &bus = (*definition)->root();
  const std::array inputs{
      bus.select_port().bind(loom::value::Bit{true}),
      bus.source_zero_ports()[0].bind(loom::value::Bit{false}),
      bus.source_zero_ports()[1].bind(loom::value::Bit{false}),
      bus.source_zero_ports()[2].bind(loom::value::Bit{true}),
      bus.source_zero_ports()[3].bind(loom::value::Bit{true}),
      bus.source_one_ports()[0].bind(loom::value::Bit{true}),
      bus.source_one_ports()[1].bind(loom::value::Bit{false}),
      bus.source_one_ports()[2].bind(loom::value::Bit{true}),
      bus.source_one_ports()[3].bind(loom::value::Bit{false})};
  const auto observed = (*definition)->observe(inputs);
  if (!observed) {
    std::cerr << observed.error().operation << ": " << observed.error().path
              << '\n';
    return 1;
  }

  std::cout << "select=source_one -> ";
  for (std::size_t bit = width; bit > 0; --bit) {
    const auto path =
        "bus.selector.bit_" + std::to_string(bit - 1) + ".result.out";
    const auto signal = std::ranges::find(observed->signals, path,
                                          &loom::simulation::Signal::path);
    std::cout << (signal != observed->signals.end() && signal->value.high());
  }
  std::cout << '\n';
}
