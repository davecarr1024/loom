#include "loom/components/word_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>

int main() {
  using loom::components::WordRegister;
  using loom::value::Bit;
  const auto initial =
      std::array{Bit{false}, Bit{false}, Bit{false}, Bit{false}};
  const auto definition = loom::simulation::Definition<WordRegister<4>>::create(
      "register", initial);
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto simulation =
      loom::simulation::Simulation<WordRegister<4>>::create(*definition);
  if (!simulation) {
    std::cerr << simulation.error().operation << ": " << simulation.error().path
              << '\n';
    return 1;
  }

  for (unsigned value : {0b1010U, 0b0101U, 0b1111U}) {
    const auto &reg = (*definition)->root();
    const std::array inputs{reg.data_ports()[0].bind(Bit{(value & 1U) != 0}),
                            reg.data_ports()[1].bind(Bit{(value & 2U) != 0}),
                            reg.data_ports()[2].bind(Bit{(value & 4U) != 0}),
                            reg.data_ports()[3].bind(Bit{(value & 8U) != 0})};
    const auto edge = (*simulation)->step(inputs);
    if (!edge) {
      std::cerr << edge.error().operation << ": " << edge.error().path << '\n';
      return 1;
    }
    std::cout << "edge " << edge->index << " loaded=";
    for (auto bit = edge->commits.rbegin(); bit != edge->commits.rend(); ++bit)
      std::cout << bit->new_value.high();
    std::cout << '\n';
  }
}
