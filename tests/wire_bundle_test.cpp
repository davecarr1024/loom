#include "loom/simulation/logic.h"
#include "loom/structure/wire_bundle.h"
#include <array>
#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <utility>

namespace {
using loom::simulation::Definition;
using loom::structure::connect;
using loom::structure::ExternalInput;
using loom::structure::ExternalOutput;
using loom::structure::split;
using loom::structure::wire_bundle;
using loom::value::Bit;

template <std::size_t... Index>
auto make_inputs(std::index_sequence<Index...>) {
  return std::array<ExternalInput, sizeof...(Index)>{
      ExternalInput{"in" + std::to_string(Index)}...};
}

template <std::size_t... Index>
auto make_outputs(std::index_sequence<Index...>,
                  const std::string &prefix = "out") {
  return std::array<ExternalOutput, sizeof...(Index)>{
      ExternalOutput{prefix + std::to_string(Index)}...};
}

template <std::size_t Width> struct BundlePass {
  static_assert(Width > 0);
  const std::string name;
  const std::array<ExternalInput, Width> inputs;
  const std::array<ExternalOutput, Width> outputs;

  explicit BundlePass(std::string label = "pass")
      : name(std::move(label)),
        inputs(make_inputs(std::make_index_sequence<Width>{})),
        outputs(make_outputs(std::make_index_sequence<Width>{})) {}

  auto input_bundle() const {
    return input_bundle_impl(std::make_index_sequence<Width>{});
  }
  auto output_bundle() const {
    return output_bundle_impl(std::make_index_sequence<Width>{});
  }

  template <std::size_t... Index>
  auto children_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(std::tie(inputs[Index]...),
                          std::tie(outputs[Index]...));
  }
  auto children() const {
    return children_impl(std::make_index_sequence<Width>{});
  }
  auto connections() const {
    return std::tuple{connect(input_bundle(), output_bundle())};
  }

private:
  template <std::size_t... Index>
  auto input_bundle_impl(std::index_sequence<Index...>) const {
    return wire_bundle(inputs[Index].output()...);
  }
  template <std::size_t... Index>
  auto output_bundle_impl(std::index_sequence<Index...>) const {
    return wire_bundle(outputs[Index].input()...);
  }
};

template <std::size_t Width> struct ParallelPasses {
  const std::string name = "parallel";
  BundlePass<Width> left{"left"};
  BundlePass<Width> right{"right"};
  auto children() const { return std::tie(left, right); }
  auto connections() const { return std::tuple{}; }
};

template <std::size_t Width> struct BundleFork {
  const std::string name = "fork";
  const std::array<ExternalInput, Width> inputs =
      make_inputs(std::make_index_sequence<Width>{});
  const std::array<ExternalOutput, Width> left =
      make_outputs(std::make_index_sequence<Width>{}, "left");
  const std::array<ExternalOutput, Width> right =
      make_outputs(std::make_index_sequence<Width>{}, "right");

  template <std::size_t... Index>
  auto children_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(std::tie(inputs[Index]...), std::tie(left[Index]...),
                          std::tie(right[Index]...));
  }
  auto children() const {
    return children_impl(std::make_index_sequence<Width>{});
  }
  auto connections() const {
    return connections_impl(std::make_index_sequence<Width>{});
  }

private:
  template <std::size_t... Index>
  auto connections_impl(std::index_sequence<Index...>) const {
    const auto source = wire_bundle(inputs[Index].output()...);
    return std::tuple{connect(source, wire_bundle(left[Index].input()...)),
                      connect(source, wire_bundle(right[Index].input()...))};
  }
};

template <std::size_t Width>
std::array<loom::structure::Binding, Width>
bind(const std::array<ExternalInput, Width> &ports,
     const std::array<bool, Width> &values) {
  std::array<loom::structure::Binding, Width> result =
      [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::array<loom::structure::Binding, Width>{
            ports[I].bind(Bit{values[I]})...};
      }(std::make_index_sequence<Width>{});
  return result;
}

template <std::size_t Width>
std::array<loom::structure::Binding, Width>
bind(const BundlePass<Width> &pass, const std::array<bool, Width> &values) {
  return bind(pass.inputs, values);
}

TEST(WireBundle, OneBitAndCheckedRuntimeIndex) {
  BundlePass<1> pass;
  const auto input = pass.input_bundle();
  const auto output = pass.output_bundle();
  static_assert(decltype(input)::width == 1);
  ASSERT_TRUE(input.at(0));
  EXPECT_EQ(&input.at(0)->get(), &pass.inputs[0].output());
  ASSERT_TRUE(output.at(0));
  EXPECT_EQ(&output.at(0)->get(), &pass.outputs[0].input());
  EXPECT_EQ(input.at(1).error(), (loom::structure::BundleIndexError{1, 1}));
}

TEST(WireBundle, ConcatenationAndSplitPreserveLeastSignificantFirstOrder) {
  BundlePass<4> pass;
  const auto original = pass.input_bundle();
  const auto [low, high] = split<2>(original);
  const auto rebuilt = loom::structure::concat(low, high);
  static_assert(decltype(rebuilt)::width == 4);
  for (std::size_t index = 0; index < 4; ++index) {
    ASSERT_TRUE(rebuilt.at(index));
    ASSERT_TRUE(original.at(index));
    EXPECT_EQ(&rebuilt.at(index)->get(), &original.at(index)->get());
  }
  EXPECT_EQ(&low.at(0)->get(), &pass.inputs[0].output());
  EXPECT_EQ(&high.at(0)->get(), &pass.inputs[2].output());
}

TEST(WireBundle, ConnectionsExpandToScalarPathsAndCarryValues) {
  const auto definition = Definition<BundlePass<4>>::create();
  ASSERT_TRUE(definition);
  const auto &root = (*definition)->root();
  EXPECT_EQ((*definition)->connections(),
            (std::vector<loom::simulation::WireInfo>{
                {"pass.in0.out", "pass.out0.in"},
                {"pass.in1.out", "pass.out1.in"},
                {"pass.in2.out", "pass.out2.in"},
                {"pass.in3.out", "pass.out3.in"}}));

  const std::array<bool, 4> bits{true, false, true, true};
  const auto inputs = bind(root, bits);
  const auto observed = (*definition)->observe(inputs);
  ASSERT_TRUE(observed);
  for (std::size_t index = 0; index < bits.size(); ++index) {
    const auto path = "pass.out" + std::to_string(index) + ".in";
    const auto signal = std::ranges::find(observed->signals, path,
                                          &loom::simulation::Signal::path);
    ASSERT_NE(signal, observed->signals.end());
    EXPECT_EQ(signal->value.high(), bits[index]);
  }
}

TEST(WireBundle, SameWidthInstancesKeepTheirOwnEndpointIdentity) {
  const auto definition = Definition<ParallelPasses<4>>::create();
  ASSERT_TRUE(definition);
  const auto &root = (*definition)->root();
  const std::array left_values{true, false, false, true};
  const std::array right_values{false, true, true, false};
  const auto left = bind(root.left, left_values);
  const auto right = bind(root.right, right_values);
  const std::array inputs{left[0],  left[1],  left[2],  left[3],
                          right[0], right[1], right[2], right[3]};
  const auto observed = (*definition)->observe(inputs);
  ASSERT_TRUE(observed);
  for (std::size_t index = 0; index < 4; ++index) {
    const auto left_path = "parallel.left.out" + std::to_string(index) + ".in";
    const auto right_path =
        "parallel.right.out" + std::to_string(index) + ".in";
    const auto left_signal = std::ranges::find(observed->signals, left_path,
                                               &loom::simulation::Signal::path);
    const auto right_signal = std::ranges::find(
        observed->signals, right_path, &loom::simulation::Signal::path);
    ASSERT_NE(left_signal, observed->signals.end());
    ASSERT_NE(right_signal, observed->signals.end());
    EXPECT_EQ(left_signal->value.high(), left_values[index]);
    EXPECT_EQ(right_signal->value.high(), right_values[index]);
  }
}

TEST(WireBundle, FanoutRoutesEveryBitToBothDestinations) {
  constexpr std::size_t width = 4;
  const auto definition = Definition<BundleFork<width>>::create();
  ASSERT_TRUE(definition);
  EXPECT_EQ((*definition)->connections().size(), 2 * width);
  const auto &root = (*definition)->root();
  const std::array<bool, width> values{false, true, true, false};
  const auto inputs = bind(root.inputs, values);
  const auto observed = (*definition)->observe(inputs);
  ASSERT_TRUE(observed);
  for (std::size_t index = 0; index < width; ++index) {
    for (const auto *prefix : {"fork.left", "fork.right"}) {
      const auto path = std::string(prefix) + std::to_string(index) + ".in";
      const auto signal = std::ranges::find(observed->signals, path,
                                            &loom::simulation::Signal::path);
      ASSERT_NE(signal, observed->signals.end());
      EXPECT_EQ(signal->value.high(), values[index]);
    }
  }
}
} // namespace
