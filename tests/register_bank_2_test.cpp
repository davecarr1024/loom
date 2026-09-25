#include "loom/components/constant_bit.h"
#include "loom/components/register_bank_2.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>

namespace {
using loom::components::RegisterAddress;
using loom::components::RegisterBank2;
using loom::components::RegisterWriteSource;
using loom::simulation::Definition;
using loom::simulation::Error;
using loom::simulation::Kind;
using loom::simulation::Observation;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalOutput;
using loom::value::Bit;

std::array<Bit, 4> bits(unsigned value) {
  return {Bit{(value & 1U) != 0}, Bit{(value & 2U) != 0},
          Bit{(value & 4U) != 0}, Bit{(value & 8U) != 0}};
}

template <std::size_t Width, std::size_t... Index>
auto bindings(const RegisterBank2<Width> &bank, RegisterAddress read_address,
              RegisterWriteSource source, RegisterAddress write_address,
              bool write_enable, const std::array<bool, Width> &write_data,
              std::index_sequence<Index...>) {
  return std::array<Binding, Width + 4>{
      bank.read_address(read_address), bank.write_source(source),
      bank.write_address(write_address),
      bank.write_enable_port().bind(Bit{write_enable}),
      bank.write_data_ports()[Index].bind(Bit{write_data[Index]})...};
}

template <std::size_t Width>
auto bindings(const RegisterBank2<Width> &bank, RegisterAddress read_address,
              RegisterWriteSource source, RegisterAddress write_address,
              bool write_enable, const std::array<bool, Width> &write_data) {
  return bindings(bank, read_address, source, write_address, write_enable,
                  write_data, std::make_index_sequence<Width>{});
}

template <std::size_t Width>
unsigned observed_word(const Observation &observation,
                       const std::string &prefix) {
  unsigned result = 0;
  for (std::size_t bit = 0; bit < Width; ++bit) {
    const auto path = prefix + ".storage.bit_" + std::to_string(bit) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << bit;
  }
  return result;
}

template <std::size_t Width>
unsigned observed_read(const Observation &observation,
                       const std::string &prefix) {
  unsigned result = 0;
  for (std::size_t bit = 0; bit < Width; ++bit) {
    const auto path = prefix + ".read_bus.selector.bit_" + std::to_string(bit) +
                      ".result.out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      result |= 1U << bit;
  }
  return result;
}

TEST(RegisterBank2, ReadsAndWritesThroughSelectedBuses) {
  const auto definition = Definition<RegisterBank2<4>>::create(
      "bank", std::array{bits(3), bits(12)});
  ASSERT_TRUE(definition);
  EXPECT_EQ(std::ranges::count_if((*definition)->inventory(),
                                  [](const auto &item) {
                                    return item.kind == Kind::d_flip_flop;
                                  }),
            8);
  EXPECT_EQ(std::ranges::count_if(
                (*definition)->inventory(),
                [](const auto &item) { return item.kind == Kind::not_gate; }),
            17);
  EXPECT_EQ(std::ranges::count_if(
                (*definition)->inventory(),
                [](const auto &item) { return item.kind == Kind::and_gate; }),
            34);
  EXPECT_EQ(std::ranges::count_if(
                (*definition)->inventory(),
                [](const auto &item) { return item.kind == Kind::or_gate; }),
            16);
  const auto &bank = (*definition)->root();
  const auto simulation =
      loom::simulation::Simulation<RegisterBank2<4>>::create(*definition);
  ASSERT_TRUE(simulation);

  auto input = bindings(
      bank, RegisterAddress::zero, RegisterWriteSource::selected_register,
      RegisterAddress::one, true, std::array{false, false, false, false});
  const auto before = (*simulation)->observe(input);
  ASSERT_TRUE(before);
  EXPECT_EQ(observed_word<4>(*before, "bank.register_zero"), 3U);
  EXPECT_EQ(observed_word<4>(*before, "bank.register_one"), 12U);
  EXPECT_EQ(observed_read<4>(*before, "bank"), 3U);

  const auto edge = (*simulation)->step(input);
  ASSERT_TRUE(edge);
  EXPECT_EQ(edge->commits.size(), 8U);
  const auto after = (*simulation)->observe(input);
  ASSERT_TRUE(after);
  EXPECT_EQ(observed_word<4>(*after, "bank.register_zero"), 3U);
  EXPECT_EQ(observed_word<4>(*after, "bank.register_one"), 3U);
  EXPECT_EQ(observed_read<4>(*after, "bank"), 3U);

  input = bindings(bank, RegisterAddress::one,
                   RegisterWriteSource::external_input, RegisterAddress::zero,
                   true, std::array{false, true, false, true});
  const auto external_write = (*simulation)->step(input);
  ASSERT_TRUE(external_write);
  const auto after_external = (*simulation)->observe(input);
  ASSERT_TRUE(after_external);
  EXPECT_EQ(observed_word<4>(*after_external, "bank.register_zero"), 10U);
  EXPECT_EQ(observed_word<4>(*after_external, "bank.register_one"), 3U);
  EXPECT_EQ(observed_read<4>(*after_external, "bank"), 3U);

  input =
      bindings(bank, RegisterAddress::zero, RegisterWriteSource::external_input,
               RegisterAddress::one, false, std::array{true, true, true, true});
  const auto hold = (*simulation)->step(input);
  ASSERT_TRUE(hold);
  const auto after_hold = (*simulation)->observe(input);
  ASSERT_TRUE(after_hold);
  EXPECT_EQ(observed_word<4>(*after_hold, "bank.register_zero"), 10U);
  EXPECT_EQ(observed_word<4>(*after_hold, "bank.register_one"), 3U);
}

TEST(RegisterBank2, EveryReadAndWriteAddressPairIsSelectable) {
  for (const auto read_address :
       {RegisterAddress::zero, RegisterAddress::one}) {
    for (const auto write_address :
         {RegisterAddress::zero, RegisterAddress::one}) {
      const auto definition = Definition<RegisterBank2<4>>::create(
          "bank", std::array{bits(5), bits(10)});
      ASSERT_TRUE(definition);
      const auto &bank = (*definition)->root();
      const auto simulation =
          loom::simulation::Simulation<RegisterBank2<4>>::create(*definition);
      ASSERT_TRUE(simulation);
      const auto input =
          bindings(bank, read_address, RegisterWriteSource::selected_register,
                   write_address, true, std::array{false, false, false, false});
      const auto before = (*simulation)->observe(input);
      ASSERT_TRUE(before);
      const unsigned selected_value =
          read_address == RegisterAddress::zero ? 5U : 10U;
      EXPECT_EQ(observed_read<4>(*before, "bank"), selected_value);
      ASSERT_TRUE((*simulation)->step(input));
      const auto after = (*simulation)->observe(input);
      ASSERT_TRUE(after);
      EXPECT_EQ(observed_word<4>(*after, "bank.register_zero"),
                write_address == RegisterAddress::zero ? selected_value : 5U);
      EXPECT_EQ(observed_word<4>(*after, "bank.register_one"),
                write_address == RegisterAddress::one ? selected_value : 10U);
    }
  }
}

struct BankParent {
  const std::string name{"parent"};
  RegisterBank2<4> bank{"bank", std::array{bits(6), bits(9)}};
  std::array<ExternalOutput, 4> output{
      ExternalOutput{"out_0"}, ExternalOutput{"out_1"}, ExternalOutput{"out_2"},
      ExternalOutput{"out_3"}};

  auto children() const {
    return std::tuple_cat(std::tie(bank),
                          std::tie(output[0], output[1], output[2], output[3]));
  }
  auto connections() const {
    return std::tuple{loom::structure::connect(
        bank.read_output(),
        loom::structure::wire_bundle(output[0].input(), output[1].input(),
                                     output[2].input(), output[3].input()))};
  }
};

struct MiswiredBankParent {
  const std::string name{"parent"};
  RegisterBank2<4> bank{"bank", std::array{bits(6), bits(9)}};
  loom::components::ConstantBit accidental_driver{"accidental_driver",
                                                  Bit{true}};
  std::array<ExternalOutput, 4> output{
      ExternalOutput{"out_0"}, ExternalOutput{"out_1"}, ExternalOutput{"out_2"},
      ExternalOutput{"out_3"}};

  auto children() const {
    return std::tuple_cat(std::tie(bank, accidental_driver),
                          std::tie(output[0], output[1], output[2], output[3]));
  }
  auto connections() const {
    return std::tuple_cat(
        std::tuple{loom::structure::connect(
            bank.read_output(), loom::structure::wire_bundle(
                                    output[0].input(), output[1].input(),
                                    output[2].input(), output[3].input()))},
        std::tuple{loom::structure::connect(accidental_driver.output(),
                                            output[0].input())});
  }
};

TEST(RegisterBank2, ReadBusConnectsToContainingCircuitBoundary) {
  const auto definition = Definition<BankParent>::create();
  ASSERT_TRUE(definition);
  const auto &bank = (*definition)->root().bank;
  const auto input = bindings(
      bank, RegisterAddress::one, RegisterWriteSource::external_input,
      RegisterAddress::zero, false, std::array{false, false, false, false});
  const auto observed = (*definition)->observe(input);
  ASSERT_TRUE(observed);
  for (std::size_t bit = 0; bit < 4; ++bit) {
    const auto path = "parent.out_" + std::to_string(bit) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_EQ(signal->value.high(), ((9U >> bit) & 1U) != 0);
  }
}

TEST(RegisterBank2, ParentRejectsTwoDriversOnOneReadOutputBit) {
  const auto definition = Definition<MiswiredBankParent>::create();
  ASSERT_FALSE(definition);
  EXPECT_EQ(definition.error(), (Error{"multiple_drivers", "parent.out_0.in"}));
}

TEST(RegisterBank2, InvalidInputsLeaveStateAndEdgeIndexUnchanged) {
  const auto definition = Definition<RegisterBank2<4>>::create(
      "bank", std::array{bits(3), bits(12)});
  ASSERT_TRUE(definition);
  const auto &bank = (*definition)->root();
  const auto simulation =
      loom::simulation::Simulation<RegisterBank2<4>>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto valid = bindings(
      bank, RegisterAddress::zero, RegisterWriteSource::external_input,
      RegisterAddress::one, false, std::array{false, false, false, false});
  const auto before = (*simulation)->observe(valid);
  ASSERT_TRUE(before);

  const auto duplicate =
      std::array{bank.read_address(RegisterAddress::zero),
                 bank.read_address(RegisterAddress::one),
                 bank.write_source(RegisterWriteSource::external_input),
                 bank.write_address(RegisterAddress::one),
                 bank.write_enable_port().bind(Bit{true}),
                 bank.write_data_ports()[0].bind(Bit{false}),
                 bank.write_data_ports()[1].bind(Bit{false}),
                 bank.write_data_ports()[2].bind(Bit{false}),
                 bank.write_data_ports()[3].bind(Bit{false})};
  EXPECT_EQ((*simulation)->step(duplicate).error(),
            (Error{"duplicate_input", "bank.read_bus.selector.select"}));
  EXPECT_EQ((*simulation)->edge_index(), 0U);
  const auto after = (*simulation)->observe(valid);
  ASSERT_TRUE(after);
  EXPECT_EQ(observed_word<4>(*after, "bank.register_zero"), 3U);
  EXPECT_EQ(observed_word<4>(*after, "bank.register_one"), 12U);
}

TEST(RegisterBank2, SimulationsOwnIndependentRegisterState) {
  const auto definition = Definition<RegisterBank2<4>>::create(
      "bank", std::array{bits(3), bits(12)});
  ASSERT_TRUE(definition);
  const auto &bank = (*definition)->root();
  const auto first =
      loom::simulation::Simulation<RegisterBank2<4>>::create(*definition);
  const auto second =
      loom::simulation::Simulation<RegisterBank2<4>>::create(*definition);
  ASSERT_TRUE(first);
  ASSERT_TRUE(second);

  const auto load = bindings(
      bank, RegisterAddress::zero, RegisterWriteSource::external_input,
      RegisterAddress::zero, true, std::array{false, true, true, false});
  const auto hold = bindings(
      bank, RegisterAddress::zero, RegisterWriteSource::external_input,
      RegisterAddress::zero, false, std::array{true, true, true, true});
  ASSERT_TRUE((*first)->step(load));
  ASSERT_TRUE((*second)->step(hold));

  const auto first_state = (*first)->observe(hold);
  const auto second_state = (*second)->observe(hold);
  ASSERT_TRUE(first_state);
  ASSERT_TRUE(second_state);
  EXPECT_EQ(observed_word<4>(*first_state, "bank.register_zero"), 6U);
  EXPECT_EQ(observed_word<4>(*second_state, "bank.register_zero"), 3U);
  EXPECT_EQ((*first)->edge_index(), 1U);
  EXPECT_EQ((*second)->edge_index(), 1U);
}

} // namespace
