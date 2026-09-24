#include "loom/components/mux_word.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <ranges>
#include <string>

int main() {
  constexpr std::size_t width = 4;
  const auto definition =
      loom::simulation::Definition<loom::components::MuxWord<width>>::create(
          "mux");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }

  const auto &mux = (*definition)->root();
  constexpr bool select = true;
  constexpr std::array when_false{false, true, true, false};
  constexpr std::array when_true{true, false, false, true};
  std::array<loom::structure::Binding, 1 + 2 * width> inputs{
      mux.select().bind(loom::value::Bit{select}),
      mux.when_false_ports()[0].bind(loom::value::Bit{when_false[0]}),
      mux.when_false_ports()[1].bind(loom::value::Bit{when_false[1]}),
      mux.when_false_ports()[2].bind(loom::value::Bit{when_false[2]}),
      mux.when_false_ports()[3].bind(loom::value::Bit{when_false[3]}),
      mux.when_true_ports()[0].bind(loom::value::Bit{when_true[0]}),
      mux.when_true_ports()[1].bind(loom::value::Bit{when_true[1]}),
      mux.when_true_ports()[2].bind(loom::value::Bit{when_true[2]}),
      mux.when_true_ports()[3].bind(loom::value::Bit{when_true[3]})};
  const auto observed = (*definition)->observe(inputs);
  if (!observed) {
    std::cerr << observed.error().operation << ": " << observed.error().path
              << '\n';
    return 1;
  }

  std::cout << "select=" << select << " false=0110 true=1001 -> ";
  for (std::size_t bit = width; bit > 0; --bit) {
    const auto path = "mux.bit_" + std::to_string(bit - 1) + ".result.out";
    const auto signal = std::ranges::find(observed->signals, path,
                                          &loom::simulation::Signal::path);
    std::cout << (signal != observed->signals.end() && signal->value.high());
  }
  std::cout << '\n';
}
