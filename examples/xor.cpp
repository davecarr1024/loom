#include "loom/components/xor.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>

int main() {
  const auto definition =
      loom::simulation::Definition<loom::components::Xor>::create("xor");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  for (const bool left : {false, true})
    for (const bool right : {false, true}) {
      const std::array inputs{
          (*definition)->root().left().bind(loom::value::Bit{left}),
          (*definition)->root().right().bind(loom::value::Bit{right})};
      const auto observed = (*definition)->observe(inputs);
      if (!observed) {
        std::cerr << observed.error().operation << ": " << observed.error().path
                  << '\n';
        return 1;
      }
      const auto output = std::ranges::find(observed->signals, "xor.result.out",
                                            &loom::simulation::Signal::path);
      std::cout << left << " XOR " << right << " = " << output->value.high()
                << '\n';
    }
}
