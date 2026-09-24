#include "loom/components/mux_word.h"
#include "loom/components/not.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <vector>

namespace {
using loom::components::MuxBit;
using loom::components::MuxWord;
using loom::simulation::Definition;
using loom::simulation::Error;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalInput;
using loom::structure::ExternalOutput;
using loom::value::Bit;

template <std::size_t Width, std::size_t... Index>
auto make_bindings(const MuxWord<Width> &mux, bool select,
                   const std::array<bool, Width> &when_false,
                   const std::array<bool, Width> &when_true,
                   std::index_sequence<Index...>) {
  return std::array<Binding, 1 + 2 * Width>{
      mux.select().bind(Bit{select}),
      mux.when_false_ports()[Index].bind(Bit{when_false[Index]})...,
      mux.when_true_ports()[Index].bind(Bit{when_true[Index]})...};
}

template <std::size_t Width>
auto make_bindings(const MuxWord<Width> &mux, bool select,
                   const std::array<bool, Width> &when_false,
                   const std::array<bool, Width> &when_true) {
  return make_bindings(mux, select, when_false, when_true,
                       std::make_index_sequence<Width>{});
}

template <std::size_t Width>
std::vector<std::string> output_paths(const std::string &root) {
  std::vector<std::string> paths;
  for (std::size_t bit = 0; bit < Width; ++bit)
    paths.push_back(root + ".bit_" + std::to_string(bit) + ".result.out");
  return paths;
}

template <std::size_t Width, class Check>
void for_each_selection_case(Check check) {
  constexpr auto pattern_count = std::size_t{1} << Width;
  for (bool select : {false, true})
    for (std::size_t false_pattern = 0; false_pattern < pattern_count;
         ++false_pattern)
      for (std::size_t true_pattern = 0; true_pattern < pattern_count;
           ++true_pattern) {
        std::array<bool, Width> when_false{}, when_true{};
        for (std::size_t bit = 0; bit < Width; ++bit) {
          when_false[bit] = ((false_pattern >> bit) & 1U) != 0;
          when_true[bit] = ((true_pattern >> bit) & 1U) != 0;
        }
        check(select, when_false, when_true);
      }
}

template <std::size_t Width>
void expect_selected_values(bool select,
                            const std::array<bool, Width> &when_false,
                            const std::array<bool, Width> &when_true,
                            const std::array<bool, Width> &actual) {
  for (std::size_t bit = 0; bit < Width; ++bit)
    EXPECT_EQ(actual[bit], select ? when_true[bit] : when_false[bit])
        << "bit=" << bit;
}

TEST(SelectionContract, OneBitAndWordMuxesShareWidthParameterizedProof) {
  static_assert(loom::components::SelectionContract<MuxBit, 1>);
  static_assert(loom::components::SelectionContract<MuxWord<4>, 4>);

  const auto bit_definition = Definition<MuxBit>::create("bit");
  ASSERT_TRUE(bit_definition);
  for_each_selection_case<1>([&](bool select, const auto &when_false,
                                 const auto &when_true) {
    const auto &bit_mux = (*bit_definition)->root();
    const std::array inputs{bit_mux.select().bind(Bit{select}),
                            bit_mux.when_false().bind(Bit{when_false[0]}),
                            bit_mux.when_true().bind(Bit{when_true[0]})};
    const auto observed = (*bit_definition)->observe(inputs);
    ASSERT_TRUE(observed);
    const auto output =
        std::ranges::find(observed->signals, "bit.result.out", &Signal::path);
    ASSERT_NE(output, observed->signals.end());
    expect_selected_values<1>(select, when_false, when_true,
                              {output->value.high()});
  });

  const auto word_definition = Definition<MuxWord<4>>::create("mux");
  ASSERT_TRUE(word_definition);
  const auto &word_mux = (*word_definition)->root();
  const auto false_bundle = word_mux.when_false();
  const auto true_bundle = word_mux.when_true();
  const auto output_bundle = word_mux.output();
  for (std::size_t bit = 0; bit < 4; ++bit) {
    EXPECT_EQ(&false_bundle.bits()[bit].get(),
              &word_mux.when_false_ports()[bit].output());
    EXPECT_EQ(&true_bundle.bits()[bit].get(),
              &word_mux.when_true_ports()[bit].output());
    EXPECT_EQ(&word_mux.output().bits()[bit].get(),
              &output_bundle.bits()[bit].get());
  }
  EXPECT_EQ((*word_definition)->connections().size(), 40U);
  for_each_selection_case<4>([&](bool select, const auto &when_false,
                                 const auto &when_true) {
    const auto inputs = make_bindings(word_mux, select, when_false, when_true);
    const auto observed = (*word_definition)->observe(inputs);
    ASSERT_TRUE(observed);
    std::array<bool, 4> actual{};
    const auto paths = output_paths<4>("mux");
    for (std::size_t bit = 0; bit < actual.size(); ++bit) {
      const auto signal =
          std::ranges::find(observed->signals, paths[bit], &Signal::path);
      ASSERT_NE(signal, observed->signals.end()) << paths[bit];
      actual[bit] = signal->value.high();
    }
    expect_selected_values<4>(select, when_false, when_true, actual);
  });
}

template <std::size_t Width> struct WordMuxOutputs {
  const std::string name{"outputs"};
  MuxWord<Width> mux{"mux"};
  const std::array<ExternalOutput, Width> outputs =
      make_outputs(std::make_index_sequence<Width>{});

  auto children() const {
    return children_impl(std::make_index_sequence<Width>{});
  }
  auto connections() const {
    return connections_impl(std::make_index_sequence<Width>{});
  }

private:
  template <std::size_t... Index>
  static auto make_outputs(std::index_sequence<Index...>) {
    return std::array<ExternalOutput, Width>{
        ExternalOutput{"out_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  auto children_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(std::tie(mux), std::tie(outputs[Index]...));
  }

  template <std::size_t... Index>
  auto connections_impl(std::index_sequence<Index...>) const {
    return std::tuple{loom::structure::connect(
        mux.output(), loom::structure::wire_bundle(outputs[Index].input()...))};
  }
};

TEST(MuxWord, OutputBundleConnectsToParentBoundaries) {
  const auto definition = Definition<WordMuxOutputs<4>>::create();
  ASSERT_TRUE(definition);
  const auto &mux = (*definition)->root().mux;
  const std::array when_false{false, true, false, true};
  const std::array when_true{true, false, true, false};
  const auto inputs = make_bindings(mux, true, when_false, when_true);
  const auto observed = (*definition)->observe(inputs);
  ASSERT_TRUE(observed);
  for (std::size_t bit = 0; bit < 4; ++bit) {
    const auto path = "outputs.out_" + std::to_string(bit) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_EQ(signal->value.high(), when_true[bit]);
  }
}

template <std::size_t Width> struct InvertedWordMux {
  const std::string name{"inverted"};
  MuxWord<Width> mux{"mux"};
  const std::array<loom::components::Not, Width> invert =
      make_inverters(std::make_index_sequence<Width>{});

  auto children() const {
    return children_impl(std::make_index_sequence<Width>{});
  }
  auto connections() const {
    return connections_impl(std::make_index_sequence<Width>{});
  }

private:
  template <std::size_t... Index>
  static auto make_inverters(std::index_sequence<Index...>) {
    return std::array<loom::components::Not, Width>{
        loom::components::Not{"invert_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  auto children_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(std::tie(mux), std::tie(invert[Index]...));
  }

  template <std::size_t... Index>
  auto connections_impl(std::index_sequence<Index...>) const {
    return std::tuple{loom::structure::connect(mux.output().bits()[Index].get(),
                                               invert[Index].input())...};
  }
};

struct InputBoundaryChild {
  const std::string name{"child"};
  ExternalInput input{"input"};
  loom::components::Not invert{"invert"};
  auto children() const { return std::tie(input, invert); }
  auto connections() const {
    return std::tuple{loom::structure::connect(input.output(), invert.input())};
  }
};

struct RoutedInputBoundary {
  const std::string name{"routed"};
  ExternalInput input{"input"};
  InputBoundaryChild child;
  ExternalOutput output{"output"};
  auto children() const { return std::tie(input, child, output); }
  auto connections() const {
    return std::tuple{
        loom::structure::connect(input.output(), child.input.input()),
        loom::structure::connect(child.invert.output(), output.input())};
  }
};

TEST(ExternalInput, ParentWireDrivesNestedBoundaryInsteadOfASecondBinding) {
  const auto definition = Definition<RoutedInputBoundary>::create();
  ASSERT_TRUE(definition);
  const auto &root = (*definition)->root();
  const std::array false_input{root.input.bind(Bit{false})};
  const std::array true_input{root.input.bind(Bit{true})};
  const auto false_observation = (*definition)->observe(false_input);
  ASSERT_TRUE(false_observation);
  const auto false_output = std::ranges::find(
      false_observation->signals, "routed.output.in", &Signal::path);
  ASSERT_NE(false_output, false_observation->signals.end());
  EXPECT_TRUE(false_output->value.high());

  const auto true_observation = (*definition)->observe(true_input);
  ASSERT_TRUE(true_observation);
  const auto true_output = std::ranges::find(true_observation->signals,
                                             "routed.output.in", &Signal::path);
  ASSERT_NE(true_output, true_observation->signals.end());
  EXPECT_FALSE(true_output->value.high());
  const auto forwarded =
      std::ranges::find((*definition)->connections(),
                        (loom::simulation::WireInfo{"routed.input.out",
                                                    "routed.child.input.in"}));
  EXPECT_NE(forwarded, (*definition)->connections().end());

  const std::array conflicting{root.input.bind(Bit{false}),
                               root.child.input.bind(Bit{true})};
  EXPECT_EQ((*definition)->observe(conflicting).error(),
            (Error{"driven_input", "routed.child.input"}));
}

TEST(MuxWord, SelectedOutputComposesAtParentBoundary) {
  const auto definition = Definition<InvertedWordMux<2>>::create();
  ASSERT_TRUE(definition);
  const auto &mux = (*definition)->root().mux;
  const std::array when_false{false, true};
  const std::array when_true{true, false};
  for (bool select : {false, true}) {
    const auto inputs = make_bindings(mux, select, when_false, when_true);
    const auto observed = (*definition)->observe(inputs);
    ASSERT_TRUE(observed);
    for (std::size_t bit = 0; bit < 2; ++bit) {
      const auto path = "inverted.invert_" + std::to_string(bit) + ".out";
      const auto signal =
          std::ranges::find(observed->signals, path, &Signal::path);
      ASSERT_NE(signal, observed->signals.end()) << path;
      EXPECT_EQ(signal->value.high(),
                !(select ? when_true[bit] : when_false[bit]));
    }
  }
}

} // namespace
