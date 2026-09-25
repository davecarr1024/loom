#include "loom/components/half_adder.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>

namespace {
using loom::components::HalfAdder;
using loom::simulation::Definition;
using loom::simulation::Kind;
using loom::simulation::Signal;
using loom::structure::ExternalOutput;
using loom::value::Bit;

TEST(HalfAdder, ComputesEveryOneBitSumAndCarryRow) {
  const auto definition = Definition<HalfAdder>::create("half_adder");
  ASSERT_TRUE(definition);
  EXPECT_EQ(std::ranges::count_if(
                (*definition)->inventory(),
                [](const auto &i) { return i.kind == Kind::not_gate; }),
            2);
  EXPECT_EQ(std::ranges::count_if(
                (*definition)->inventory(),
                [](const auto &i) { return i.kind == Kind::and_gate; }),
            3);
  EXPECT_EQ(std::ranges::count_if(
                (*definition)->inventory(),
                [](const auto &i) { return i.kind == Kind::or_gate; }),
            1);

  const auto &adder = (*definition)->root();
  for (bool left : {false, true}) {
    for (bool right : {false, true}) {
      const std::array inputs{adder.left().bind(Bit{left}),
                              adder.right().bind(Bit{right})};
      const auto observed = (*definition)->observe(inputs);
      ASSERT_TRUE(observed);
      const auto sum = std::ranges::find(
          observed->signals, "half_adder.sum.result.out", &Signal::path);
      const auto carry = std::ranges::find(
          observed->signals, "half_adder.carry.out", &Signal::path);
      ASSERT_NE(sum, observed->signals.end());
      ASSERT_NE(carry, observed->signals.end());
      EXPECT_EQ(sum->value.high(), left != right);
      EXPECT_EQ(carry->value.high(), left && right);
    }
  }
}

struct HalfAdderOutputs {
  const std::string name{"parent"};
  HalfAdder adder{"adder"};
  ExternalOutput sum{"sum"};
  ExternalOutput carry{"carry"};

  auto children() const { return std::tie(adder, sum, carry); }
  auto connections() const {
    return std::tuple{loom::structure::connect(adder.sum(), sum.input()),
                      loom::structure::connect(adder.carry(), carry.input())};
  }
};

TEST(HalfAdder, SumAndCarryConnectToParentOutputBoundaries) {
  const auto definition = Definition<HalfAdderOutputs>::create();
  ASSERT_TRUE(definition);
  const auto &adder = (*definition)->root().adder;
  const std::array inputs{adder.left().bind(Bit{true}),
                          adder.right().bind(Bit{true})};
  const auto observed = (*definition)->observe(inputs);
  ASSERT_TRUE(observed);
  const auto sum =
      std::ranges::find(observed->signals, "parent.sum.in", &Signal::path);
  const auto carry =
      std::ranges::find(observed->signals, "parent.carry.in", &Signal::path);
  ASSERT_NE(sum, observed->signals.end());
  ASSERT_NE(carry, observed->signals.end());
  EXPECT_FALSE(sum->value.high());
  EXPECT_TRUE(carry->value.high());
}

} // namespace
