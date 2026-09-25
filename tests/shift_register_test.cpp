#include "loom/components/shift_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>

namespace {
using loom::components::ShiftRegister;
using loom::simulation::Definition;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalInput;
using loom::structure::ExternalOutput;
using loom::value::Bit;

auto initial_word(unsigned value) {
  return std::array{Bit{(value & 1U) != 0}, Bit{(value & 2U) != 0},
                    Bit{(value & 4U) != 0}, Bit{(value & 8U) != 0}};
}

std::array<Binding, 2> inputs(const ShiftRegister<4> &reg, bool enable,
                              bool serial) {
  return {reg.enable_port().bind(Bit{enable}),
          reg.serial_port().bind(Bit{serial})};
}

unsigned observed_word(const loom::simulation::Observation &observation,
                       const std::string &prefix) {
  unsigned result = 0;
  for (unsigned index = 0; index < 4; ++index) {
    const auto path =
        prefix + ".storage.storage.bit_" + std::to_string(index) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << index;
  }
  return result;
}

struct ShiftParent {
  const std::string name{"parent"};
  ExternalInput enable{"shift"};
  ExternalInput serial{"serial"};
  ShiftRegister<4> reg{"shift_register", initial_word(0b1010)};
  std::array<ExternalOutput, 4> output{
      ExternalOutput{"out_0"}, ExternalOutput{"out_1"}, ExternalOutput{"out_2"},
      ExternalOutput{"out_3"}};

  auto children() const {
    return std::tuple_cat(std::tie(enable, serial, reg),
                          std::tie(output[0], output[1], output[2], output[3]));
  }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(enable.output(), reg.enable_input()),
        loom::structure::connect(serial.output(), reg.serial_input()),
        loom::structure::connect(reg.output(),
                                 loom::structure::wire_bundle(
                                     output[0].input(), output[1].input(),
                                     output[2].input(), output[3].input()))};
  }
};

TEST(ShiftRegister, ShiftsTowardHighBitsAndLoadsSerialIntoBitZero) {
  const auto definition =
      Definition<ShiftRegister<4>>::create("shift", initial_word(0b1010));
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  EXPECT_EQ((*definition)->connections().size(), 57U);
  EXPECT_EQ(std::ranges::count((*definition)->inventory(),
                               loom::simulation::Kind::d_flip_flop,
                               &loom::simulation::ComponentInfo::kind),
            4);

  const auto simulation =
      loom::simulation::Simulation<ShiftRegister<4>>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto initial =
      (*simulation)->observe(inputs((*definition)->root(), false, false));
  ASSERT_TRUE(initial);
  EXPECT_EQ(observed_word(*initial, "shift"), 0b1010U);

  const auto first =
      (*simulation)->step(inputs((*definition)->root(), true, true));
  ASSERT_TRUE(first);
  EXPECT_EQ(observed_word(first->evaluated, "shift"), 0b1010U);
  auto after_first =
      (*simulation)->observe(inputs((*definition)->root(), false, false));
  ASSERT_TRUE(after_first);
  EXPECT_EQ(observed_word(*after_first, "shift"), 0b0101U);

  const auto second =
      (*simulation)->step(inputs((*definition)->root(), true, false));
  ASSERT_TRUE(second);
  auto after_second =
      (*simulation)->observe(inputs((*definition)->root(), false, false));
  ASSERT_TRUE(after_second);
  EXPECT_EQ(observed_word(*after_second, "shift"), 0b1010U);

  const auto held =
      (*simulation)->step(inputs((*definition)->root(), false, true));
  ASSERT_TRUE(held);
  const auto after_hold =
      (*simulation)->observe(inputs((*definition)->root(), false, false));
  ASSERT_TRUE(after_hold);
  EXPECT_EQ(observed_word(*after_hold, "shift"), 0b1010U);
  EXPECT_EQ(observed_word(first->evaluated, "shift"), 0b1010U);
}

TEST(ShiftRegister, HandlesSingleBitAndParentBoundaryComposition) {
  const auto one_bit =
      Definition<ShiftRegister<1>>::create("one", std::array{Bit{false}});
  ASSERT_TRUE(one_bit);
  const auto one_simulation =
      loom::simulation::Simulation<ShiftRegister<1>>::create(*one_bit);
  ASSERT_TRUE(one_simulation);
  const auto &single = (*one_bit)->root();
  const std::array one_inputs{single.enable_port().bind(Bit{true}),
                              single.serial_port().bind(Bit{true})};
  const auto one_edge = (*one_simulation)->step(one_inputs);
  ASSERT_TRUE(one_edge);
  EXPECT_EQ(one_edge->commits.size(), 1U);
  EXPECT_EQ(one_edge->commits[0].data_value, Bit{true});

  const auto definition = Definition<ShiftParent>::create();
  ASSERT_TRUE(definition) << definition.error().operation << ": "
                          << definition.error().path;
  const auto &parent = (*definition)->root();
  const auto bindings =
      std::array{parent.enable.bind(Bit{true}), parent.serial.bind(Bit{false})};
  const auto simulation =
      loom::simulation::Simulation<ShiftParent>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto edge = (*simulation)->step(bindings);
  ASSERT_TRUE(edge);
  const auto observed = (*simulation)->observe(bindings);
  ASSERT_TRUE(observed);
  for (unsigned index = 0; index < 4; ++index) {
    const auto path = "parent.out_" + std::to_string(index) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_EQ(signal->value.high(), index == 2);
  }
}
} // namespace
