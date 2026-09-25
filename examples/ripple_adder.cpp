#include "loom/components/ripple_adder.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <ranges>
#include <string>

namespace {
using loom::components::RippleAdder;
using loom::structure::Binding;
using loom::value::Bit;

template <std::size_t... Index>
auto bindings(const RippleAdder<4> &adder, unsigned left, unsigned right,
              std::index_sequence<Index...>) {
  return std::array<Binding, 9>{
      adder.left_ports()[Index].bind(Bit{((left >> Index) & 1U) != 0})...,
      adder.right_ports()[Index].bind(Bit{((right >> Index) & 1U) != 0})...,
      adder.carry_in().bind(Bit{false})};
}

unsigned word(const loom::simulation::Observation &observation) {
  unsigned value = 0;
  for (unsigned bit = 0; bit < 4; ++bit) {
    const auto path =
        "adder.bit_" + std::to_string(bit) + ".second.sum.result.out";
    const auto signal = std::ranges::find(observation.signals, path,
                                          &loom::simulation::Signal::path);
    if (signal != observation.signals.end() && signal->value.high())
      value |= 1U << bit;
  }
  return value;
}
} // namespace

int main() {
  const auto definition =
      loom::simulation::Definition<RippleAdder<4>>::create("adder");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto &adder = (*definition)->root();
  const auto input = bindings(adder, 9, 7, std::make_index_sequence<4>{});
  const auto observed = (*definition)->observe(input);
  if (!observed) {
    std::cerr << observed.error().operation << ": " << observed.error().path
              << '\n';
    return 1;
  }
  const auto carry =
      std::ranges::find(observed->signals, "adder.bit_3.carry_merge.out",
                        &loom::simulation::Signal::path);
  std::cout << "9 + 7 = " << word(*observed) << " carry "
            << (carry != observed->signals.end() && carry->value.high())
            << '\n';
}
