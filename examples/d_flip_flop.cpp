#include "loom/simulation/logic.h"
#include <iostream>

namespace {
struct Toggle {
  const std::string name = "toggle";
  loom::components::DFlipFlop state{"state", loom::value::Bit{false}};
  loom::components::Not invert{"invert"};
  auto children() const { return std::tie(state, invert); }
  auto connections() const {
    return std::tuple{loom::structure::connect(state.output(), invert.input()),
                      loom::structure::connect(invert.output(), state.data())};
  }
};
} // namespace

int main() {
  const auto definition = loom::simulation::Definition<Toggle>::create();
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto simulation =
      loom::simulation::Simulation<Toggle>::create(*definition);
  if (!simulation) {
    std::cerr << simulation.error().operation << ": " << simulation.error().path
              << '\n';
    return 1;
  }
  for (int i = 0; i < 4; ++i) {
    const auto edge = (*simulation)->step({});
    if (!edge) {
      std::cerr << edge.error().operation << ": " << edge.error().path << '\n';
      return 1;
    }
    const auto &commit = edge->commits.front();
    std::cout << "edge " << edge->index << ' ' << commit.path << ": "
              << commit.old_value.high() << " --D=" << commit.data_value.high()
              << "--> " << commit.new_value.high() << '\n';
  }
}
