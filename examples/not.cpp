#include "loom/simulation/logic.h"
#include <array>
#include <iostream>

namespace {
struct DoubleNot {
  const std::string name = "double_not";
  loom::structure::ExternalInput input{"input"};
  loom::components::Not first{"first"}, second{"second"};
  loom::structure::ExternalOutput output{"output"};
  auto children() const { return std::tie(input, first, second, output); }
  auto connections() const {
    using loom::structure::connect;
    return std::tuple{connect(input.output(), first.input()),
                      connect(first.output(), second.input()),
                      connect(second.output(), output.input())};
  }
};
} // namespace

int main() {
  const auto definition = loom::simulation::Definition<DoubleNot>::create();
  if (!definition)
    return 1;
  for (const auto &component : (*definition)->inventory())
    std::cout << "component " << component.path << '\n';
  for (const auto &wire : (*definition)->connections())
    std::cout << wire.source << " -> " << wire.destination << '\n';
  for (const bool high : {false, true}) {
    const std::array inputs{
        (*definition)->root().input.bind(loom::value::Bit{high})};
    const auto observation = (*definition)->observe(inputs);
    if (!observation)
      return 1;
    std::cout << "observe input=" << high << " (before edge 0)\n";
    for (const auto &signal : observation->signals)
      std::cout << "  " << signal.path << '=' << signal.value.high() << '\n';
  }
}
