#include "loom/components/word_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

namespace {
using loom::components::WordRegister;
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

std::array<Binding, 4> bind_word(const WordRegister<4> &reg, unsigned value) {
  return {reg.data_ports()[0].bind(Bit{(value & 1U) != 0}),
          reg.data_ports()[1].bind(Bit{(value & 2U) != 0}),
          reg.data_ports()[2].bind(Bit{(value & 4U) != 0}),
          reg.data_ports()[3].bind(Bit{(value & 8U) != 0})};
}

unsigned observed_word(const loom::simulation::Observation &observation) {
  unsigned result = 0;
  for (unsigned index = 0; index < 4; ++index) {
    const auto path = "register.bit_" + std::to_string(index) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << index;
  }
  return result;
}

struct RegisterParent {
  const std::string name{"parent"};
  std::array<ExternalInput, 4> source{
      ExternalInput{"source_0"}, ExternalInput{"source_1"},
      ExternalInput{"source_2"}, ExternalInput{"source_3"}};
  WordRegister<4> reg{"register", bits(3)};
  std::array<ExternalOutput, 4> sink{
      ExternalOutput{"sink_0"}, ExternalOutput{"sink_1"},
      ExternalOutput{"sink_2"}, ExternalOutput{"sink_3"}};

  auto children() const {
    return std::tuple_cat(std::tie(source[0], source[1], source[2], source[3]),
                          std::tie(reg),
                          std::tie(sink[0], sink[1], sink[2], sink[3]));
  }
  auto connections() const {
    return std::tuple{
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

TEST(WordRegister, InitializesAndLoadsEveryBitOnEachSharedEdge) {
  const auto definition =
      Definition<WordRegister<4>>::create("register", bits(0b1010));
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  EXPECT_EQ(
      (*definition)->inventory(),
      (std::vector<ComponentInfo>{{"register", Kind::composite},
                                  {"register.bit_0", Kind::d_flip_flop},
                                  {"register.bit_1", Kind::d_flip_flop},
                                  {"register.bit_2", Kind::d_flip_flop},
                                  {"register.bit_3", Kind::d_flip_flop},
                                  {"register.data_0", Kind::external_input},
                                  {"register.data_1", Kind::external_input},
                                  {"register.data_2", Kind::external_input},
                                  {"register.data_3", Kind::external_input}}));
  EXPECT_EQ((*definition)->connections().size(), 4U);
  EXPECT_EQ((*definition)->root().output().width, 4U);

  const auto simulation =
      loom::simulation::Simulation<WordRegister<4>>::create(*definition);
  ASSERT_TRUE(simulation);
  EXPECT_EQ((*simulation)->edge_index(), 0U);
  const auto initial =
      (*simulation)->observe(bind_word((*definition)->root(), 0));
  ASSERT_TRUE(initial);
  EXPECT_EQ(observed_word(*initial), 0b1010U);

  const auto first =
      (*simulation)->step(bind_word((*definition)->root(), 0b0101));
  ASSERT_TRUE(first);
  EXPECT_EQ(first->index, 0U);
  ASSERT_EQ(first->commits.size(), 4U);
  EXPECT_EQ(first->commits[0].old_value, Bit{false});
  EXPECT_EQ(first->commits[1].old_value, Bit{true});
  EXPECT_EQ(first->commits[2].old_value, Bit{false});
  EXPECT_EQ(first->commits[3].old_value, Bit{true});
  EXPECT_EQ(first->commits[0].data_value, Bit{true});
  EXPECT_EQ(first->commits[1].data_value, Bit{false});
  const auto after_first =
      (*simulation)->observe(bind_word((*definition)->root(), 0));
  ASSERT_TRUE(after_first);
  EXPECT_EQ(observed_word(*after_first), 0b0101U);

  const auto retained = *first;
  const auto second =
      (*simulation)->step(bind_word((*definition)->root(), 0b0011));
  ASSERT_TRUE(second);
  EXPECT_EQ(second->index, 1U);
  EXPECT_EQ(observed_word(retained.evaluated), 0b1010U);
  const auto after_second =
      (*simulation)->observe(bind_word((*definition)->root(), 0));
  ASSERT_TRUE(after_second);
  EXPECT_EQ(observed_word(*after_second), 0b0011U);
}

TEST(WordRegister, AcceptsParentDrivenDataAndExposesOutputBundle) {
  const auto definition = Definition<RegisterParent>::create();
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  const auto &parent = (*definition)->root();
  const auto inputs = std::array{
      parent.source[0].bind(Bit{true}), parent.source[1].bind(Bit{false}),
      parent.source[2].bind(Bit{true}), parent.source[3].bind(Bit{true})};
  const auto invalid = std::array{inputs[0], inputs[1], inputs[2], inputs[3],
                                  parent.reg.data_ports()[0].bind(Bit{false})};
  const auto rejected = (*definition)->observe(invalid);
  ASSERT_FALSE(rejected);
  EXPECT_EQ(rejected.error(), (loom::simulation::Error{
                                  "driven_input", "parent.register.data_0"}));

  const auto simulation =
      loom::simulation::Simulation<RegisterParent>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto edge = (*simulation)->step(inputs);
  ASSERT_TRUE(edge);
  ASSERT_EQ(edge->commits.size(), 4U);
  const auto observed = (*simulation)->observe(inputs);
  ASSERT_TRUE(observed);
  for (unsigned index = 0; index < 4; ++index) {
    const auto path = "parent.sink_" + std::to_string(index) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_EQ(signal->value.high(), index != 1);
  }
  EXPECT_EQ((*definition)->connections().size(), 12U);
  EXPECT_EQ((*definition)->schedule().size(), 8U);
}
} // namespace
