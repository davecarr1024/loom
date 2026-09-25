#include "loom/components/enabled_word_register.h"
#include "loom/components/word_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>

namespace {
using loom::components::EnabledWordRegister;
using loom::components::WordRegister;
using loom::simulation::Observation;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::value::Bit;

template <std::size_t... Index>
auto word_bits(std::uint64_t value, std::index_sequence<Index...>) {
  return std::array<Bit, sizeof...(Index)>{
      Bit{((value >> Index) & 1U) != 0}...};
}

struct Transfer {
  const std::string name{"transfer"};
  WordRegister<8> source{"source",
                         word_bits(42, std::make_index_sequence<8>{})};
  EnabledWordRegister<8> destination{
      "destination", word_bits(0, std::make_index_sequence<8>{})};

  auto children() const { return std::tie(source, destination); }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(source.output(), destination.data_input())};
  }
};

std::array<Binding, 9> bindings(const Transfer &transfer) {
  const auto &source = transfer.source;
  return {source.data_ports()[0].bind(Bit{false}),
          source.data_ports()[1].bind(Bit{true}),
          source.data_ports()[2].bind(Bit{false}),
          source.data_ports()[3].bind(Bit{true}),
          source.data_ports()[4].bind(Bit{false}),
          source.data_ports()[5].bind(Bit{true}),
          source.data_ports()[6].bind(Bit{false}),
          source.data_ports()[7].bind(Bit{false}),
          transfer.destination.enable_port().bind(Bit{true})};
}

std::uint64_t word(const Observation &observation, const std::string &prefix,
                   bool nested) {
  std::uint64_t result = 0;
  for (unsigned index = 0; index < 8; ++index) {
    const auto path = prefix + (nested ? ".storage." : ".") + "bit_" +
                      std::to_string(index) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    if (signal != observation.signals.end() && signal->value.high())
      result |= std::uint64_t{1} << index;
  }
  return result;
}
} // namespace

int main() {
  const auto definition = loom::simulation::Definition<Transfer>::create();
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto simulation =
      loom::simulation::Simulation<Transfer>::create(*definition);
  if (!simulation) {
    std::cerr << simulation.error().operation << ": " << simulation.error().path
              << '\n';
    return 1;
  }

  const auto input = bindings((*definition)->root());
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

  for (const auto &[path, nested] : {std::pair{"transfer.destination", true},
                                     std::pair{"transfer.source", false}})
    std::cout << "edge " << edge->index << ' ' << path << ": "
              << word(*before, path, nested) << " -> "
              << word(*after, path, nested) << '\n';
}
