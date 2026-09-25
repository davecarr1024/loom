#include "loom/components/shift_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>

int main() {
  using loom::components::ShiftRegister;
  using loom::value::Bit;
  const auto initial =
      std::array{Bit{false}, Bit{false}, Bit{false}, Bit{false}};
  const auto definition =
      loom::simulation::Definition<ShiftRegister<4>>::create("shift", initial);
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto simulation =
      loom::simulation::Simulation<ShiftRegister<4>>::create(*definition);
  if (!simulation) {
    std::cerr << simulation.error().operation << ": " << simulation.error().path
              << '\n';
    return 1;
  }

  const auto &reg = (*definition)->root();
  for (bool serial : {true, false, true, true}) {
    const std::array inputs{reg.enable_port().bind(Bit{true}),
                            reg.serial_port().bind(Bit{serial})};
    const auto edge = (*simulation)->step(inputs);
    if (!edge) {
      std::cerr << edge.error().operation << ": " << edge.error().path << '\n';
      return 1;
    }
    const auto &commits = edge->commits;
    std::cout << "edge " << edge->index << " serial=" << serial << " word=";
    for (auto commit = commits.rbegin(); commit != commits.rend(); ++commit)
      std::cout << commit->new_value.high();
    std::cout << '\n';
  }
}
