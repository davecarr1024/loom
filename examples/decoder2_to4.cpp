#include "loom/components/decoder2_to4.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <ranges>
#include <string>

int main() {
  const auto definition =
      loom::simulation::Definition<loom::components::Decoder2To4>::create(
          "decoder");
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }

  const auto &decoder = (*definition)->root();
  for (unsigned address = 0; address < 4; ++address) {
    const std::array inputs{
        decoder.address_ports()[0].bind(loom::value::Bit{(address & 1U) != 0}),
        decoder.address_ports()[1].bind(loom::value::Bit{(address & 2U) != 0})};
    const auto observed = (*definition)->observe(inputs);
    if (!observed) {
      std::cerr << observed.error().operation << ": " << observed.error().path
                << '\n';
      return 1;
    }
    std::cout << "address=" << ((address >> 1U) & 1U) << (address & 1U)
              << " -> ";
    for (unsigned output = 4; output > 0; --output) {
      const auto path = "decoder.decode_" + std::to_string(output - 1) + ".out";
      const auto signal = std::ranges::find(observed->signals, path,
                                            &loom::simulation::Signal::path);
      std::cout << (signal != observed->signals.end() && signal->value.high());
    }
    std::cout << '\n';
  }
}
