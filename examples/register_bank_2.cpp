#include "loom/components/register_bank_2.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <ranges>
#include <string>

namespace {
using loom::components::RegisterAddress;
using loom::components::RegisterBank2;
using loom::components::RegisterWriteSource;
using loom::simulation::Observation;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::value::Bit;

template <std::size_t... Index>
auto word_bits(unsigned value, std::index_sequence<Index...>) {
  return std::array<Bit, sizeof...(Index)>{
      Bit{((value >> Index) & 1U) != 0}...};
}

template <std::size_t Width, std::size_t... Index>
auto bindings(const RegisterBank2<Width> &bank, std::index_sequence<Index...>) {
  return std::array<Binding, Width + 4>{
      bank.read_address(RegisterAddress::zero),
      bank.write_source(RegisterWriteSource::selected_register),
      bank.write_address(RegisterAddress::one),
      bank.write_enable_port().bind(Bit{true}),
      bank.write_data_ports()[Index].bind(Bit{false})...};
}

template <std::size_t Width>
unsigned stored_word(const Observation &observation, const std::string &name) {
  unsigned result = 0;
  for (std::size_t bit = 0; bit < Width; ++bit) {
    const auto path =
        "bank." + name + ".storage.bit_" + std::to_string(bit) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << bit;
  }
  return result;
}
} // namespace

int main() {
  constexpr std::size_t width = 8;
  const auto definition =
      loom::simulation::Definition<RegisterBank2<width>>::create(
          "bank", std::array{word_bits(42, std::make_index_sequence<width>{}),
                             word_bits(0, std::make_index_sequence<width>{})});
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto simulation =
      loom::simulation::Simulation<RegisterBank2<width>>::create(*definition);
  if (!simulation) {
    std::cerr << simulation.error().operation << ": " << simulation.error().path
              << '\n';
    return 1;
  }

  const auto input =
      bindings((*definition)->root(), std::make_index_sequence<width>{});
  const auto before = (*simulation)->observe(input);
  if (!before) {
    std::cerr << before.error().operation << ": " << before.error().path
              << '\n';
    return 1;
  }
  const auto edge = (*simulation)->step(input);
  if (!edge) {
    std::cerr << edge.error().operation << ": " << edge.error().path << '\n';
    return 1;
  }
  const auto after = (*simulation)->observe(input);
  if (!after) {
    std::cerr << after.error().operation << ": " << after.error().path << '\n';
    return 1;
  }

  std::cout << "edge " << edge->index << " bank.register_one: "
            << stored_word<width>(*before, "register_one") << " -> "
            << stored_word<width>(*after, "register_one") << '\n';
}
