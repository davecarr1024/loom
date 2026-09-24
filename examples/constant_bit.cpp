#include "loom/simulation/logic.h"
#include <iostream>

int main() {
  for (const bool high : {false, true}) {
    const auto definition =
        loom::simulation::Definition<loom::components::ConstantBit>::create(
            "constant", loom::value::Bit{high});
    if (!definition) {
      std::cerr << definition.error().operation << ": "
                << definition.error().path << '\n';
      return 1;
    }
    const auto observation = (*definition)->observe({});
    if (!observation) {
      std::cerr << observation.error().operation << ": "
                << observation.error().path << '\n';
      return 1;
    }
    std::cout << "constant " << high << " -> "
              << observation->signals.front().value.high() << '\n';
  }
}
