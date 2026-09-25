#include "loom/components/full_adder.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>

namespace {
using loom::components::FullAdder;
using loom::simulation::Definition;
using loom::simulation::Kind;
using loom::simulation::Signal;
using loom::structure::ExternalOutput;
using loom::value::Bit;

TEST(FullAdder, ComputesAllEightOperandAndCarryRows) {
  const auto definition = Definition<FullAdder>::create("full_adder");
  ASSERT_TRUE(definition);
  const auto inventory = (*definition)->inventory();
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::not_gate; }),
      4);
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::and_gate; }),
      6);
  EXPECT_EQ(
      std::ranges::count_if(
          inventory, [](const auto &i) { return i.kind == Kind::or_gate; }),
      3);

  const auto &adder = (*definition)->root();
  for (bool left : {false, true}) {
    for (bool right : {false, true}) {
      for (bool carry_in : {false, true}) {
        const std::array inputs{adder.left().bind(Bit{left}),
                                adder.right().bind(Bit{right}),
                                adder.carry_in().bind(Bit{carry_in})};
        const auto observed = (*definition)->observe(inputs);
        ASSERT_TRUE(observed);
        const auto sum = std::ranges::find(observed->signals,
                                           "full_adder.second.sum.result.out",
                                           &Signal::path);
        const auto carry = std::ranges::find(
            observed->signals, "full_adder.carry_merge.out", &Signal::path);
        ASSERT_NE(sum, observed->signals.end());
        ASSERT_NE(carry, observed->signals.end());
        const unsigned total = static_cast<unsigned>(left) +
                               static_cast<unsigned>(right) +
                               static_cast<unsigned>(carry_in);
        EXPECT_EQ(sum->value.high(), (total & 1U) != 0);
        EXPECT_EQ(carry->value.high(), (total & 2U) != 0);
      }
    }
  }
}

struct FullAdderOutputs {
  const std::string name{"parent"};
  FullAdder adder{"adder"};
  ExternalOutput sum{"sum"};
  ExternalOutput carry{"carry"};

  auto children() const { return std::tie(adder, sum, carry); }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(adder.sum(), sum.input()),
        loom::structure::connect(adder.carry_out(), carry.input())};
  }
};

TEST(FullAdder, RoutesComposedOutputsThroughParentBoundaries) {
  const auto definition = Definition<FullAdderOutputs>::create();
  ASSERT_TRUE(definition);
  const auto &adder = (*definition)->root().adder;
  const std::array inputs{adder.left().bind(Bit{true}),
                          adder.right().bind(Bit{true}),
                          adder.carry_in().bind(Bit{true})};
  const auto observed = (*definition)->observe(inputs);
  ASSERT_TRUE(observed);
  const auto sum =
      std::ranges::find(observed->signals, "parent.sum.in", &Signal::path);
  const auto carry =
      std::ranges::find(observed->signals, "parent.carry.in", &Signal::path);
  ASSERT_NE(sum, observed->signals.end());
  ASSERT_NE(carry, observed->signals.end());
  EXPECT_TRUE(sum->value.high());
  EXPECT_TRUE(carry->value.high());
}

} // namespace
