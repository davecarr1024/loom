#include "loom/components/ripple_adder.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>

namespace {
using loom::components::RippleAdder;
using loom::simulation::Definition;
using loom::simulation::Kind;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalOutput;
using loom::value::Bit;

template <std::size_t... Index>
auto bindings(const RippleAdder<4> &adder, unsigned left, unsigned right,
              bool carry_in, std::index_sequence<Index...>) {
  return std::array<Binding, 9>{
      adder.left_ports()[Index].bind(Bit{((left >> Index) & 1U) != 0})...,
      adder.right_ports()[Index].bind(Bit{((right >> Index) & 1U) != 0})...,
      adder.carry_in().bind(Bit{carry_in})};
}

auto bindings(const RippleAdder<4> &adder, unsigned left, unsigned right,
              bool carry_in) {
  return bindings(adder, left, right, carry_in, std::make_index_sequence<4>{});
}

unsigned observe_word(const loom::simulation::Observation &observation,
                      const std::string &prefix) {
  unsigned result = 0;
  for (unsigned bit = 0; bit < 4; ++bit) {
    const auto path =
        prefix + ".bit_" + std::to_string(bit) + ".second.sum.result.out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << bit;
  }
  return result;
}

TEST(RippleAdder, ExhaustsFourBitOperandsAndCarryInThroughFullAdderChildren) {
  const auto definition = Definition<RippleAdder<4>>::create("adder");
  ASSERT_TRUE(definition);
  const auto inventory = (*definition)->inventory();
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::d_flip_flop; }),
      0);
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::not_gate; }),
      16);
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::and_gate; }),
      24);
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::or_gate; }),
      12);

  const auto &adder = (*definition)->root();
  for (unsigned left = 0; left < 16; ++left) {
    for (unsigned right = 0; right < 16; ++right) {
      for (bool carry_in : {false, true}) {
        const auto input = bindings(adder, left, right, carry_in);
        const auto observed = (*definition)->observe(input);
        ASSERT_TRUE(observed);
        const unsigned total = left + right + static_cast<unsigned>(carry_in);
        EXPECT_EQ(observe_word(*observed, "adder"), total & 0xFU);
        const auto final_carry = std::ranges::find(
            observed->signals, "adder.bit_3.carry_merge.out", &Signal::path);
        ASSERT_NE(final_carry, observed->signals.end());
        EXPECT_EQ(final_carry->value.high(), (total & 0x10U) != 0);

        for (unsigned bit = 0; bit < 4; ++bit) {
          const unsigned mask = (1U << (bit + 1)) - 1U;
          const unsigned low_total =
              (left & mask) + (right & mask) + static_cast<unsigned>(carry_in);
          const auto carry_path =
              "adder.bit_" + std::to_string(bit) + ".carry_merge.out";
          const auto carry =
              std::ranges::find(observed->signals, carry_path, &Signal::path);
          ASSERT_NE(carry, observed->signals.end()) << carry_path;
          EXPECT_EQ(carry->value.high(), ((low_total >> (bit + 1)) & 1U) != 0);
        }
      }
    }
  }
}

template <std::size_t... Index>
auto make_outputs(std::index_sequence<Index...>) {
  return std::array<ExternalOutput, sizeof...(Index)>{
      ExternalOutput{"sum_" + std::to_string(Index)}...};
}

struct RippleAdderBoundary {
  const std::string name{"parent"};
  RippleAdder<4> adder{"adder"};
  std::array<ExternalOutput, 4> sum_ports =
      make_outputs(std::make_index_sequence<4>{});
  ExternalOutput carry_out{"carry_out"};

  auto children() const {
    return std::tuple_cat(std::tie(adder),
                          std::tie(sum_ports[0], sum_ports[1], sum_ports[2],
                                   sum_ports[3], carry_out));
  }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(
            adder.sum(), loom::structure::wire_bundle(
                             sum_ports[0].input(), sum_ports[1].input(),
                             sum_ports[2].input(), sum_ports[3].input())),
        loom::structure::connect(adder.carry_out(), carry_out.input())};
  }
};

TEST(RippleAdder, SumBundleAndCarryRouteThroughParentBoundaries) {
  const auto definition = Definition<RippleAdderBoundary>::create();
  ASSERT_TRUE(definition);
  const auto &adder = (*definition)->root().adder;
  const auto input = bindings(adder, 9, 7, false);
  const auto observed = (*definition)->observe(input);
  ASSERT_TRUE(observed);
  for (unsigned bit = 0; bit < 4; ++bit) {
    const auto path = "parent.sum_" + std::to_string(bit) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_FALSE(signal->value.high());
  }
  const auto carry = std::ranges::find(observed->signals, "parent.carry_out.in",
                                       &Signal::path);
  ASSERT_NE(carry, observed->signals.end());
  EXPECT_TRUE(carry->value.high());
}

} // namespace
