#include "loom/components/mux_bit.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>

int main() {
  const auto definition =
      loom::simulation::Definition<loom::components::MuxBit>::create("mux");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  for (const bool select : {false, true})
    for (const bool when_false : {false, true})
      for (const bool when_true : {false, true}) {
        const auto &root = (*definition)->root();
        const std::array inputs{
            root.select().bind(loom::value::Bit{select}),
            root.when_false().bind(loom::value::Bit{when_false}),
            root.when_true().bind(loom::value::Bit{when_true})};
        const auto observed = (*definition)->observe(inputs);
        if (!observed) {
          std::cerr << observed.error().operation << ": "
                    << observed.error().path << '\n';
          return 1;
        }
        const auto output =
            std::ranges::find(observed->signals, "mux.result.out",
                              &loom::simulation::Signal::path);
        std::cout << "select=" << select << " false=" << when_false
                  << " true=" << when_true << " -> " << output->value.high()
                  << '\n';
      }
}
