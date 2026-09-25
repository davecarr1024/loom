#include "loom/components/enabled_word_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <utility>

int main() {
  using loom::components::EnabledWordRegister;
  using loom::value::Bit;
  const auto initial =
      std::array{Bit{false}, Bit{false}, Bit{false}, Bit{false}};
  const auto definition =
      loom::simulation::Definition<EnabledWordRegister<4>>::create("enabled",
                                                                   initial);
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto simulation =
      loom::simulation::Simulation<EnabledWordRegister<4>>::create(*definition);
  if (!simulation) {
    std::cerr << simulation.error().operation << ": " << simulation.error().path
              << '\n';
    return 1;
  }

  const auto &reg = (*definition)->root();
  for (const auto [enable, value] :
       {std::pair{true, 0b1010U}, std::pair{false, 0b0101U},
        std::pair{true, 0b1100U}}) {
    const std::array inputs{reg.enable_port().bind(Bit{enable}),
                            reg.data_ports()[0].bind(Bit{(value & 1U) != 0}),
                            reg.data_ports()[1].bind(Bit{(value & 2U) != 0}),
                            reg.data_ports()[2].bind(Bit{(value & 4U) != 0}),
                            reg.data_ports()[3].bind(Bit{(value & 8U) != 0})};
    const auto edge = (*simulation)->step(inputs);
    if (!edge) {
      std::cerr << edge.error().operation << ": " << edge.error().path << '\n';
      return 1;
    }
    std::cout << "edge " << edge->index << (enable ? " load=" : " hold=");
    for (auto commit = edge->commits.rbegin(); commit != edge->commits.rend();
         ++commit)
      std::cout << commit->new_value.high();
    std::cout << '\n';
  }
}
