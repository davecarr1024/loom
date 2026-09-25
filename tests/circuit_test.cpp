#include "loom/components/enabled_word_register.h"
#include "loom/components/word_register.h"
#include "loom/simulation/logic.h"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>

namespace {
using loom::components::EnabledWordRegister;
using loom::components::WordRegister;
using loom::simulation::Definition;
using loom::simulation::Error;
using loom::simulation::Observation;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::value::Bit;

template <std::size_t Width, std::size_t... Index>
auto make_word(std::uint64_t value, std::index_sequence<Index...>) {
  return std::array<Bit, Width>{Bit{((value >> Index) & 1U) != 0}...};
}

template <std::size_t Width> auto make_word(std::uint64_t value) {
  return make_word<Width>(value, std::make_index_sequence<Width>{});
}

std::uint64_t observed_word(const Observation &observation,
                            const std::string &prefix,
                            const std::string &layout, unsigned width) {
  std::uint64_t value = 0;
  for (unsigned index = 0; index < width; ++index) {
    const auto path = prefix + layout + "bit_" + std::to_string(index) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, path, &Signal::path);
    EXPECT_NE(signal, observation.signals.end()) << path;
    if (signal != observation.signals.end() && signal->value.high())
      value |= std::uint64_t{1} << index;
  }
  return value;
}

std::uint64_t observed_enabled(const Observation &observation,
                               const std::string &path) {
  std::uint64_t value = 0;
  for (unsigned index = 0; index < 64; ++index) {
    const auto signal_path =
        path + ".storage.bit_" + std::to_string(index) + ".out";
    const auto signal =
        std::ranges::find(observation.signals, signal_path, &Signal::path);
    if (signal == observation.signals.end())
      break;
    if (signal->value.high())
      value |= std::uint64_t{1} << index;
  }
  return value;
}

struct Pair {
  const std::string name;
  EnabledWordRegister<4> a;
  EnabledWordRegister<4> b;
  const bool reverse_children;

  Pair(std::string label = "pair", unsigned x = 1, unsigned y = 2,
       bool backwards = false)
      : name(std::move(label)), a{"a", make_word<4>(x)},
        b{"b", make_word<4>(y)}, reverse_children(backwards) {}

  auto children() const {
    return reverse_children ? std::tie(b, a) : std::tie(a, b);
  }
  auto connections() const {
    return std::tuple{loom::structure::connect(a.output(), b.data_input()),
                      loom::structure::connect(b.output(), a.data_input())};
  }
};

std::array<Binding, 2> controls(const Pair &pair, bool enable_a,
                                bool enable_b) {
  return {pair.a.enable_port().bind(Bit{enable_a}),
          pair.b.enable_port().bind(Bit{enable_b})};
}

TEST(Circuit, ExhaustiveTransferHoldAndSimultaneousSwap) {
  for (unsigned x = 0; x < 16; ++x)
    for (unsigned y = 0; y < 16; ++y) {
      const auto definition = Definition<Pair>::create("pair", x, y);
      ASSERT_TRUE(definition);
      const auto simulation =
          loom::simulation::Simulation<Pair>::create(*definition);
      ASSERT_TRUE(simulation);

      const auto swap =
          (*simulation)->step(controls((*definition)->root(), true, true));
      ASSERT_TRUE(swap);
      ASSERT_EQ(swap->commits.size(), 8U);
      EXPECT_EQ(swap->index, 0U);
      EXPECT_EQ(swap->commits[0].data_value, Bit{(y & 1U) != 0});
      EXPECT_EQ(swap->commits[4].data_value, Bit{(x & 1U) != 0});
      const auto after_swap =
          (*simulation)->observe(controls((*definition)->root(), false, false));
      ASSERT_TRUE(after_swap);
      EXPECT_EQ(observed_enabled(*after_swap, "pair.a"), y);
      EXPECT_EQ(observed_enabled(*after_swap, "pair.b"), x);

      const auto hold =
          (*simulation)->step(controls((*definition)->root(), false, false));
      ASSERT_TRUE(hold);
      const auto after_hold =
          (*simulation)->observe(controls((*definition)->root(), false, false));
      ASSERT_TRUE(after_hold);
      EXPECT_EQ(observed_enabled(*after_hold, "pair.a"), y);
      EXPECT_EQ(observed_enabled(*after_hold, "pair.b"), x);

      const auto transfer_a =
          (*simulation)->step(controls((*definition)->root(), true, false));
      ASSERT_TRUE(transfer_a);
      const auto after_transfer =
          (*simulation)->observe(controls((*definition)->root(), false, false));
      ASSERT_TRUE(after_transfer);
      EXPECT_EQ(observed_enabled(*after_transfer, "pair.a"), x);
      EXPECT_EQ(observed_enabled(*after_transfer, "pair.b"), x);
    }
}

TEST(Circuit, IndependentSimulationsChildOrderAndAtomicInputFailure) {
  const auto definition = Definition<Pair>::create();
  ASSERT_TRUE(definition);
  const auto reverse_definition = Definition<Pair>::create("pair", 1, 2, true);
  ASSERT_TRUE(reverse_definition);
  const auto first = loom::simulation::Simulation<Pair>::create(*definition);
  const auto second = loom::simulation::Simulation<Pair>::create(*definition);
  const auto reverse =
      loom::simulation::Simulation<Pair>::create(*reverse_definition);
  ASSERT_TRUE(first);
  ASSERT_TRUE(second);
  ASSERT_TRUE(reverse);

  const auto &pair = (*definition)->root();
  const auto &foreign_pair = (*reverse_definition)->root();
  const auto foreign = std::array{pair.a.enable_port().bind(Bit{true}),
                                  foreign_pair.b.enable_port().bind(Bit{true})};
  const auto rejected = (*first)->step(foreign);
  ASSERT_FALSE(rejected);
  EXPECT_EQ(rejected.error(), (Error{"foreign_input", ""}));
  EXPECT_EQ((*first)->edge_index(), 0U);

  const auto duplicate = std::array{pair.a.enable_port().bind(Bit{true}),
                                    pair.a.enable_port().bind(Bit{false}),
                                    pair.b.enable_port().bind(Bit{true})};
  EXPECT_EQ((*first)->step(duplicate).error(),
            (Error{"duplicate_input", "pair.a.selector.select"}));
  EXPECT_EQ((*first)->edge_index(), 0U);
  EXPECT_EQ((*first)->step({}).error(),
            (Error{"missing_input", "pair.a.selector.select"}));
  EXPECT_EQ((*first)->edge_index(), 0U);

  const auto enables = controls(pair, true, true);
  const auto edge = (*first)->step(enables);
  ASSERT_TRUE(edge);
  const auto second_initial = (*second)->observe(controls(pair, false, false));
  ASSERT_TRUE(second_initial);
  EXPECT_EQ(observed_enabled(*second_initial, "pair.a"), 1U);
  const auto second_edge = (*second)->step(enables);
  ASSERT_TRUE(second_edge);
  EXPECT_EQ(*edge, *second_edge);
  EXPECT_EQ(*edge, *(*reverse)->step(controls(foreign_pair, true, true)));
  EXPECT_EQ((*first)->edge_index(), 1U);
  EXPECT_EQ((*second)->edge_index(), 1U);
  EXPECT_EQ((*reverse)->edge_index(), 1U);
  EXPECT_EQ(observed_enabled(*(*first)->observe(controls(pair, false, false)),
                             "pair.a"),
            2U);
  EXPECT_EQ(observed_enabled(*(*second)->observe(controls(pair, false, false)),
                             "pair.a"),
            2U);
}

struct NestedPairs {
  const std::string name{"root"};
  Pair left{"left", 3, 4};
  Pair right{"right", 7, 8};
  auto children() const { return std::tie(left, right); }
  auto connections() const { return std::tuple{}; }
};

TEST(Circuit, NestedInstancesKeepIndependentStateAndConnections) {
  const auto definition = Definition<NestedPairs>::create();
  ASSERT_TRUE(definition);
  const auto &root = (*definition)->root();
  const std::array bindings{root.left.a.enable_port().bind(Bit{true}),
                            root.left.b.enable_port().bind(Bit{false}),
                            root.right.a.enable_port().bind(Bit{false}),
                            root.right.b.enable_port().bind(Bit{true})};
  const auto simulation =
      loom::simulation::Simulation<NestedPairs>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto edge = (*simulation)->step(bindings);
  ASSERT_TRUE(edge);
  const auto observed = (*simulation)->observe(bindings);
  ASSERT_TRUE(observed);
  EXPECT_EQ(observed_enabled(*observed, "root.left.a"), 4U);
  EXPECT_EQ(observed_enabled(*observed, "root.left.b"), 4U);
  EXPECT_EQ(observed_enabled(*observed, "root.right.a"), 7U);
  EXPECT_EQ(observed_enabled(*observed, "root.right.b"), 7U);
}

struct Chain {
  const std::string name{"chain"};
  EnabledWordRegister<4> a{"a", make_word<4>(9)};
  EnabledWordRegister<4> b{"b", make_word<4>(0)};
  EnabledWordRegister<4> c{"c", make_word<4>(0)};
  EnabledWordRegister<4> fan{"fan", make_word<4>(0)};
  auto children() const { return std::tie(a, b, c, fan); }
  auto connections() const {
    return std::tuple{loom::structure::connect(a.output(), a.data_input()),
                      loom::structure::connect(a.output(), b.data_input()),
                      loom::structure::connect(b.output(), c.data_input()),
                      loom::structure::connect(a.output(), fan.data_input())};
  }
};

TEST(Circuit, RegisterChainAdvancesOneWordPerEdgeAndFansOut) {
  const auto definition = Definition<Chain>::create();
  ASSERT_TRUE(definition);
  const auto &chain = (*definition)->root();
  const std::array enables{chain.a.enable_port().bind(Bit{true}),
                           chain.b.enable_port().bind(Bit{true}),
                           chain.c.enable_port().bind(Bit{true}),
                           chain.fan.enable_port().bind(Bit{true})};
  const auto simulation =
      loom::simulation::Simulation<Chain>::create(*definition);
  ASSERT_TRUE(simulation);
  const auto first = (*simulation)->step(enables);
  ASSERT_TRUE(first);
  auto first_state = (*simulation)->observe(enables);
  ASSERT_TRUE(first_state);
  EXPECT_EQ(observed_enabled(*first_state, "chain.a"), 9U);
  EXPECT_EQ(observed_enabled(*first_state, "chain.b"), 9U);
  EXPECT_EQ(observed_enabled(*first_state, "chain.c"), 0U);
  EXPECT_EQ(observed_enabled(*first_state, "chain.fan"), 9U);
  const auto second = (*simulation)->step(enables);
  ASSERT_TRUE(second);
  auto second_state = (*simulation)->observe(enables);
  ASSERT_TRUE(second_state);
  EXPECT_EQ(observed_enabled(*second_state, "chain.c"), 9U);
}

struct Transfer {
  const std::string name{"transfer"};
  WordRegister<8> source{"source", make_word<8>(42)};
  EnabledWordRegister<8> destination{"destination", make_word<8>(0)};
  auto children() const { return std::tie(source, destination); }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(source.output(), destination.data_input())};
  }
};

template <std::size_t... Index>
auto transfer_inputs(const Transfer &transfer, bool enable,
                     std::uint64_t source_value,
                     std::index_sequence<Index...>) {
  return std::array<Binding, 9>{
      transfer.source.data_ports()[Index].bind(
          Bit{((source_value >> Index) & 1U) != 0})...,
      transfer.destination.enable_port().bind(Bit{enable})};
}

std::array<Binding, 9> transfer_inputs(const Transfer &transfer, bool enable,
                                       std::uint64_t source_value) {
  return transfer_inputs(transfer, enable, source_value,
                         std::make_index_sequence<8>{});
}

TEST(Circuit, TransferLoadsAndHoldsThroughTypedWordInterfaces) {
  const auto definition = Definition<Transfer>::create();
  ASSERT_TRUE(definition);
  const auto &transfer = (*definition)->root();
  const auto simulation =
      loom::simulation::Simulation<Transfer>::create(*definition);
  ASSERT_TRUE(simulation);

  auto input = transfer_inputs(transfer, true, 42);
  const auto before = (*simulation)->observe(input);
  ASSERT_TRUE(before);
  EXPECT_EQ(observed_word(*before, "transfer.source", ".", 8), 42U);
  EXPECT_EQ(observed_enabled(*before, "transfer.destination"), 0U);
  const auto load = (*simulation)->step(input);
  ASSERT_TRUE(load);
  EXPECT_EQ(load->commits.size(), 16U);
  const auto after_load = (*simulation)->observe(input);
  ASSERT_TRUE(after_load);
  EXPECT_EQ(observed_word(*after_load, "transfer.source", ".", 8), 42U);
  EXPECT_EQ(observed_enabled(*after_load, "transfer.destination"), 42U);

  input = transfer_inputs(transfer, false, 42);
  const auto hold = (*simulation)->step(input);
  ASSERT_TRUE(hold);
  const auto after_hold = (*simulation)->observe(input);
  ASSERT_TRUE(after_hold);
  EXPECT_EQ(observed_enabled(*after_hold, "transfer.destination"), 42U);
}

} // namespace
