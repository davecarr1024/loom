#include "loom/components/decoder2_to4.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <vector>

namespace {
using loom::components::Decoder2To4;
using loom::simulation::ComponentInfo;
using loom::simulation::Definition;
using loom::simulation::Kind;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalOutput;
using loom::value::Bit;

template <std::size_t... Index>
auto make_outputs(std::index_sequence<Index...>) {
  return std::array<ExternalOutput, sizeof...(Index)>{
      ExternalOutput{"out_" + std::to_string(Index)}...};
}

struct DecoderOutputs {
  const std::string name{"boundaries"};
  Decoder2To4 decoder{"decoder"};
  const std::array<ExternalOutput, 4> outputs =
      make_outputs(std::make_index_sequence<4>{});

  auto children() const {
    return std::tie(decoder, outputs[0], outputs[1], outputs[2], outputs[3]);
  }
  auto connections() const {
    return std::tuple{loom::structure::connect(
        decoder.output(),
        loom::structure::wire_bundle(outputs[0].input(), outputs[1].input(),
                                     outputs[2].input(), outputs[3].input()))};
  }
};

std::array<Binding, 2> bind_address(const Decoder2To4 &decoder,
                                    unsigned address) {
  return {decoder.address_ports()[0].bind(Bit{(address & 1U) != 0}),
          decoder.address_ports()[1].bind(Bit{(address & 2U) != 0})};
}

TEST(Decoder2To4, DecodesEveryAddressThroughNotAndAndChildren) {
  const auto definition = Definition<DecoderOutputs>::create();
  ASSERT_TRUE(definition);
  EXPECT_EQ((*definition)->connections().size(), 14U);
  EXPECT_EQ((*definition)->inventory(),
            (std::vector<ComponentInfo>{
                {"boundaries", Kind::composite},
                {"boundaries.decoder", Kind::composite},
                {"boundaries.decoder.address_0", Kind::external_input},
                {"boundaries.decoder.address_1", Kind::external_input},
                {"boundaries.decoder.decode_0", Kind::and_gate},
                {"boundaries.decoder.decode_1", Kind::and_gate},
                {"boundaries.decoder.decode_2", Kind::and_gate},
                {"boundaries.decoder.decode_3", Kind::and_gate},
                {"boundaries.decoder.not_address_0", Kind::not_gate},
                {"boundaries.decoder.not_address_1", Kind::not_gate},
                {"boundaries.out_0", Kind::external_output},
                {"boundaries.out_1", Kind::external_output},
                {"boundaries.out_2", Kind::external_output},
                {"boundaries.out_3", Kind::external_output}}));
  EXPECT_EQ(
      (*definition)->schedule(),
      (std::vector<std::string>{
          "boundaries.decoder.decode_3", "boundaries.decoder.not_address_0",
          "boundaries.decoder.decode_2", "boundaries.decoder.not_address_1",
          "boundaries.decoder.decode_0", "boundaries.decoder.decode_1",
          "boundaries.out_0", "boundaries.out_1", "boundaries.out_2",
          "boundaries.out_3"}));

  const auto &decoder = (*definition)->root().decoder;
  const auto address = decoder.address();
  EXPECT_EQ(&address.bits()[0].get(), &decoder.address_ports()[0].output());
  EXPECT_EQ(&address.bits()[1].get(), &decoder.address_ports()[1].output());
  for (unsigned value = 0; value < 4; ++value) {
    const auto inputs = bind_address(decoder, value);
    const auto observed = (*definition)->observe(inputs);
    ASSERT_TRUE(observed);
    const auto low_inverse = std::ranges::find(
        observed->signals, "boundaries.decoder.not_address_0.out",
        &Signal::path);
    const auto high_inverse = std::ranges::find(
        observed->signals, "boundaries.decoder.not_address_1.out",
        &Signal::path);
    ASSERT_NE(low_inverse, observed->signals.end());
    ASSERT_NE(high_inverse, observed->signals.end());
    EXPECT_EQ(low_inverse->value.high(), (value & 1U) == 0);
    EXPECT_EQ(high_inverse->value.high(), (value & 2U) == 0);
    for (unsigned output = 0; output < 4; ++output) {
      const auto path =
          "boundaries.decoder.decode_" + std::to_string(output) + ".out";
      const auto gate =
          std::ranges::find(observed->signals, path, &Signal::path);
      ASSERT_NE(gate, observed->signals.end()) << path;
      EXPECT_EQ(gate->value.high(), value == output);
      const auto boundary_path =
          "boundaries.out_" + std::to_string(output) + ".in";
      const auto boundary =
          std::ranges::find(observed->signals, boundary_path, &Signal::path);
      ASSERT_NE(boundary, observed->signals.end()) << boundary_path;
      EXPECT_EQ(boundary->value.high(), value == output);
    }
  }
}

} // namespace
