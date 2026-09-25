#include "loom/components/half_adder.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <ranges>

int main() {
  const auto definition =
      loom::simulation::Definition<loom::components::HalfAdder>::create(
          "half_adder");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }

  const auto &adder = (*definition)->root();
  for (bool left : {false, true}) {
    for (bool right : {false, true}) {
      const std::array inputs{adder.left().bind(loom::value::Bit{left}),
                              adder.right().bind(loom::value::Bit{right})};
      const auto observed = (*definition)->observe(inputs);
      if (!observed) {
        std::cerr << observed.error().operation << ": " << observed.error().path
                  << '\n';
        return 1;
      }
      const auto sum =
          std::ranges::find(observed->signals, "half_adder.sum.result.out",
                            &loom::simulation::Signal::path);
      const auto carry =
          std::ranges::find(observed->signals, "half_adder.carry.out",
                            &loom::simulation::Signal::path);
      std::cout << left << " + " << right << " = " << sum->value.high()
                << " carry " << carry->value.high() << '\n';
    }
  }
}
