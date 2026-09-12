#include "loom/circuit.h"
#include <array>
#include <iostream>

struct Transfer {
  const std::string name = "transfer";
  loom::Register<8> source{"source", 42}, destination{"destination", 0};
  auto children() const { return std::tie(source, destination); }
  auto connections() const {
    return std::tuple{loom::connect(source.output(), source.input()),
                      loom::connect(source.output(), destination.input())};
  }
};
int main() {
  const auto definition = loom::Definition<Transfer>::create();
  if (!definition)
    return 1;
  auto simulation = loom::Simulation<Transfer>::create(*definition);
  if (!simulation)
    return 1;
  const std::array enables{std::string("transfer.destination")};
  const auto edge = simulation->step(enables);
  if (!edge)
    return 1;
  for (const auto &sample : edge->registers)
    std::cout << "edge " << edge->index << " " << sample.path << ": "
              << sample.before << " -> " << sample.after << '\n';
}
