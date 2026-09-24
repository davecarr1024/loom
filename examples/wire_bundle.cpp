#include "loom/structure/wire_bundle.h"
#include "loom/simulation/logic.h"
#include <array>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>

namespace {
using loom::simulation::Definition;
using loom::structure::connect;
using loom::structure::ExternalInput;
using loom::structure::ExternalOutput;
using loom::structure::wire_bundle;
using loom::value::Bit;

template <std::size_t... Index>
auto make_inputs(std::index_sequence<Index...>) {
  return std::array<ExternalInput, sizeof...(Index)>{
      ExternalInput{"in" + std::to_string(Index)}...};
}
template <std::size_t... Index>
auto make_outputs(std::index_sequence<Index...>) {
  return std::array<ExternalOutput, sizeof...(Index)>{
      ExternalOutput{"out" + std::to_string(Index)}...};
}

struct FourBitPass {
  static constexpr std::size_t width = 4;
  const std::string name = "pass";
  const std::array<ExternalInput, width> inputs =
      make_inputs(std::make_index_sequence<width>{});
  const std::array<ExternalOutput, width> outputs =
      make_outputs(std::make_index_sequence<width>{});

  auto children() const {
    return std::tuple_cat(
        std::tie(inputs[0], inputs[1], inputs[2], inputs[3]),
        std::tie(outputs[0], outputs[1], outputs[2], outputs[3]));
  }
  auto connections() const {
    return std::tuple{
        connect(wire_bundle(inputs[0].output(), inputs[1].output(),
                            inputs[2].output(), inputs[3].output()),
                wire_bundle(outputs[0].input(), outputs[1].input(),
                            outputs[2].input(), outputs[3].input()))};
  }
};
} // namespace

int main() {
  const auto definition = Definition<FourBitPass>::create();
  if (!definition) {
    std::cerr << definition.error().operation << ": " << definition.error().path
              << '\n';
    return 1;
  }
  const auto &root = (*definition)->root();
  const std::array inputs{
      root.inputs[0].bind(Bit{true}), root.inputs[1].bind(Bit{false}),
      root.inputs[2].bind(Bit{true}), root.inputs[3].bind(Bit{true})};
  const auto observation = (*definition)->observe(inputs);
  if (!observation) {
    std::cerr << observation.error().operation << ": "
              << observation.error().path << '\n';
    return 1;
  }
  std::cout << "bit 0 is least significant; pass-through values: ";
  for (std::size_t index = 0; index < FourBitPass::width; ++index) {
    const auto path = "pass.out" + std::to_string(index) + ".in";
    const auto signal = std::ranges::find(observation->signals, path,
                                          &loom::simulation::Signal::path);
    std::cout << (signal->value.high() ? '1' : '0');
  }
  std::cout << '\n';
}
