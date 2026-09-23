#include "loom/simulation/logic.h"
#include <array>
#include <iostream>

namespace {
struct AndExample {
  const std::string name = "and_example";
  loom::structure::ExternalInput left{"left"}, right{"right"};
  loom::components::And gate{"gate"};
  loom::structure::ExternalOutput output{"output"};
  auto children() const { return std::tie(left, right, gate, output); }
  auto connections() const {
    return std::tuple{loom::structure::connect(left.output(), gate.left()),
                      loom::structure::connect(right.output(), gate.right()),
                      loom::structure::connect(gate.output(), output.input())};
  }
};
} // namespace

int main() {
  const auto definition = loom::simulation::Definition<AndExample>::create();
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  for (const bool left : {false, true})
    for (const bool right : {false, true}) {
      const auto bindings =
          std::array{(*definition)->root().left.bind(loom::value::Bit{left}),
                     (*definition)->root().right.bind(loom::value::Bit{right})};
      const auto observation = (*definition)->observe(bindings);
      if (!observation) {
        std::cerr << observation.error().operation << ": "
                  << observation.error().path << '\n';
        return 1;
      }
      const auto output =
          std::ranges::find(observation->signals, "and_example.output.in",
                            &loom::simulation::Signal::path);
      std::cout << left << " AND " << right << " = " << output->value.high()
                << '\n';
    }
}
