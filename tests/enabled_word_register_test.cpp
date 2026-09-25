#include "loom/components/constant_bit.h"
#include "loom/components/enabled_word_register.h"
#include "loom/simulation/logic.h"
#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

namespace {
using loom::components::ConstantBit;
using loom::components::EnabledWordRegister;
using loom::simulation::ComponentInfo;
using loom::simulation::Definition;
using loom::simulation::Kind;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalInput;
using loom::structure::ExternalOutput;
using loom::value::Bit;

auto bits(unsigned value) {
  return std::array{Bit{(value & 1U) != 0}, Bit{(value & 2U) != 0},
                    Bit{(value & 4U) != 0}, Bit{(value & 8U) != 0}};
}

std::array<Binding, 5> bind_word(const EnabledWordRegister<4> &reg, bool enable,
                                 unsigned value) {
  return {reg.enable_port().bind(Bit{enable}),
          reg.data_ports()[0].bind(Bit{(value & 1U) != 0}),
          reg.data_ports()[1].bind(Bit{(value & 2U) != 0}),
          reg.data_ports()[2].bind(Bit{(value & 4U) != 0}),
          reg.data_ports()[3].bind(Bit{(value & 8U) != 0})};
}

unsigned observed_word(const loom::simulation::Observation &observation,
                       const std::string &prefix) {
  unsigned result = 0;
  for (unsigned index = 0; index < 4; ++index) {
    const auto path = prefix + ".storage.bit_" + std::to_string(index) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << index;
  }
  return result;
}

struct EnabledParent {
  const std::string name{"parent"};
  ExternalInput enable{"load"};
  std::array<ExternalInput, 4> source{
      ExternalInput{"data_0"}, ExternalInput{"data_1"}, ExternalInput{"data_2"},
      ExternalInput{"data_3"}};
  EnabledWordRegister<4> reg{"enabled", bits(0b1001)};
  std::array<ExternalOutput, 4> sink{
      ExternalOutput{"out_0"}, ExternalOutput{"out_1"}, ExternalOutput{"out_2"},
      ExternalOutput{"out_3"}};

  auto children() const {
    return std::tuple_cat(
        std::tie(enable, source[0], source[1], source[2], source[3]),
        std::tie(reg), std::tie(sink[0], sink[1], sink[2], sink[3]));
  }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(enable.output(), reg.enable_input()),
        loom::structure::connect(loom::structure::wire_bundle(
                                     source[0].output(), source[1].output(),
                                     source[2].output(), source[3].output()),
                                 reg.data_input()),
        loom::structure::connect(
            reg.output(),
            loom::structure::wire_bundle(sink[0].input(), sink[1].input(),
                                         sink[2].input(), sink[3].input()))};
  }
};

struct AlwaysLoadAdapter {
  const std::string name{"always"};
  ConstantBit enabled{"load", Bit{true}};
  EnabledWordRegister<4> reg{"register", bits(0b1001)};

  auto children() const { return std::tie(enabled, reg); }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(enabled.output(), reg.enable_input())};
  }
};

std::array<Binding, 4> bind_data(const EnabledWordRegister<4> &reg,
                                 unsigned value) {
  return {reg.data_ports()[0].bind(Bit{(value & 1U) != 0}),
          reg.data_ports()[1].bind(Bit{(value & 2U) != 0}),
          reg.data_ports()[2].bind(Bit{(value & 4U) != 0}),
          reg.data_ports()[3].bind(Bit{(value & 8U) != 0})};
}

TEST(EnabledWordRegister, SelectsOldQToHoldAndInputToLoad) {
  const auto definition =
      Definition<EnabledWordRegister<4>>::create("enabled", bits(0b1001));
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  const auto &inventory = (*definition)->inventory();
  EXPECT_EQ(inventory.size(), 52U);
  EXPECT_EQ(
      std::ranges::count(inventory, Kind::d_flip_flop, &ComponentInfo::kind),
      4);
  EXPECT_EQ(std::ranges::count(inventory, Kind::and_gate, &ComponentInfo::kind),
            8);
  EXPECT_EQ(std::ranges::count(inventory, Kind::not_gate, &ComponentInfo::kind),
            4);
  EXPECT_EQ(std::ranges::count(inventory, Kind::or_gate, &ComponentInfo::kind),
            4);
  EXPECT_EQ((*definition)->connections().size(), 52U);

  const auto simulation =
      loom::simulation::Simulation<EnabledWordRegister<4>>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto initial =
      (*simulation)->observe(bind_word((*definition)->root(), false, 0));
  ASSERT_TRUE(initial);
  EXPECT_EQ(observed_word(*initial, "enabled"), 0b1001U);

  const auto held =
      (*simulation)->step(bind_word((*definition)->root(), false, 0b0110));
  ASSERT_TRUE(held);
  ASSERT_EQ(held->commits.size(), 4U);
  EXPECT_EQ(held->commits[0].data_value, Bit{true});
  EXPECT_EQ(held->commits[1].data_value, Bit{false});
  EXPECT_EQ(held->commits[2].data_value, Bit{false});
  EXPECT_EQ(held->commits[3].data_value, Bit{true});
  auto after_hold =
      (*simulation)->observe(bind_word((*definition)->root(), false, 0b1111));
  ASSERT_TRUE(after_hold);
  EXPECT_EQ(observed_word(*after_hold, "enabled"), 0b1001U);

  const auto loaded =
      (*simulation)->step(bind_word((*definition)->root(), true, 0b0110));
  ASSERT_TRUE(loaded);
  EXPECT_EQ(loaded->index, 1U);
  EXPECT_EQ(loaded->commits[0].data_value, Bit{false});
  EXPECT_EQ(loaded->commits[1].data_value, Bit{true});
  const auto after_load =
      (*simulation)->observe(bind_word((*definition)->root(), false, 0));
  ASSERT_TRUE(after_load);
  EXPECT_EQ(observed_word(*after_load, "enabled"), 0b0110U);
}

TEST(EnabledWordRegister, ParentControlsAndObservesComposedStorage) {
  const auto definition = Definition<EnabledParent>::create();
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  const auto &parent = (*definition)->root();
  const auto inputs = std::array{
      parent.enable.bind(Bit{true}), parent.source[0].bind(Bit{false}),
      parent.source[1].bind(Bit{true}), parent.source[2].bind(Bit{true}),
      parent.source[3].bind(Bit{false})};
  const auto invalid = std::array{
      inputs[0], inputs[1], inputs[2],
      inputs[3], inputs[4], parent.reg.enable_port().bind(Bit{false})};
  const auto rejected = (*definition)->observe(invalid);
  ASSERT_FALSE(rejected);
  EXPECT_EQ(rejected.error(),
            (loom::simulation::Error{"driven_input",
                                     "parent.enabled.selector.select"}));

  const auto simulation =
      loom::simulation::Simulation<EnabledParent>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto edge = (*simulation)->step(inputs);
  ASSERT_TRUE(edge);
  ASSERT_EQ(edge->commits.size(), 4U);
  const auto observed = (*simulation)->observe(inputs);
  ASSERT_TRUE(observed);
  for (unsigned index = 0; index < 4; ++index) {
    const auto path = "parent.out_" + std::to_string(index) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_EQ(signal->value.high(), index == 1 || index == 2);
  }
}

TEST(EnabledWordRegister, ConstantHighEnableAdaptsItToAlwaysLoad) {
  const auto definition = Definition<AlwaysLoadAdapter>::create();
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  const auto &adapter = (*definition)->root();
  const auto simulation =
      loom::simulation::Simulation<AlwaysLoadAdapter>::create(*definition);
  ASSERT_TRUE(simulation);

  const auto first = (*simulation)->step(bind_data(adapter.reg, 0b0101));
  ASSERT_TRUE(first);
  EXPECT_EQ(first->commits.size(), 4U);
  const auto after_first = (*simulation)->observe(bind_data(adapter.reg, 0));
  ASSERT_TRUE(after_first);
  EXPECT_EQ(observed_word(*after_first, "always.register"), 0b0101U);

  const auto second = (*simulation)->step(bind_data(adapter.reg, 0b1010));
  ASSERT_TRUE(second);
  const auto after_second = (*simulation)->observe(bind_data(adapter.reg, 0));
  ASSERT_TRUE(after_second);
  EXPECT_EQ(observed_word(*after_second, "always.register"), 0b1010U);
}
} // namespace
