#include "loom/components/selected_bus.h"
#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>
#include <ranges>
#include <string>
#include <tuple>

namespace {
using loom::components::SelectedBus;
using loom::simulation::Definition;
using loom::simulation::Signal;
using loom::structure::Binding;
using loom::structure::ExternalOutput;
using loom::value::Bit;

template <std::size_t Width> struct BusParent {
  const std::string name{"parent"};
  SelectedBus<Width> bus{"bus"};
  std::array<ExternalOutput, Width> sinks =
      make_sinks(std::make_index_sequence<Width>{});

  auto children() const {
    return std::tuple_cat(std::tie(bus),
                          sink_children(std::make_index_sequence<Width>{}));
  }
  auto connections() const {
    return std::tuple{loom::structure::connect(
        bus.output(), sink_bundle(std::make_index_sequence<Width>{}))};
  }

private:
  template <std::size_t... Index>
  static auto make_sinks(std::index_sequence<Index...>) {
    return std::array<ExternalOutput, Width>{
        ExternalOutput{"sink_" + std::to_string(Index)}...};
  }
  template <std::size_t... Index>
  auto sink_children(std::index_sequence<Index...>) const {
    return std::tie(sinks[Index]...);
  }
  template <std::size_t... Index>
  auto sink_bundle(std::index_sequence<Index...>) const {
    return loom::structure::wire_bundle(sinks[Index].input()...);
  }
};

template <std::size_t Width, std::size_t... Index>
auto bindings(const SelectedBus<Width> &bus, bool select,
              const std::array<bool, Width> &zero,
              const std::array<bool, Width> &one,
              std::index_sequence<Index...>) {
  return std::array<Binding, 1 + 2 * Width>{
      bus.select_port().bind(Bit{select}),
      bus.source_zero_ports()[Index].bind(Bit{zero[Index]})...,
      bus.source_one_ports()[Index].bind(Bit{one[Index]})...};
}

template <std::size_t Width>
auto bindings(const SelectedBus<Width> &bus, bool select,
              const std::array<bool, Width> &zero,
              const std::array<bool, Width> &one) {
  return bindings(bus, select, zero, one, std::make_index_sequence<Width>{});
}

TEST(SelectedBus, SelectsEveryFourBitSourcePattern) {
  constexpr std::size_t width = 4;
  const auto definition = Definition<SelectedBus<width>>::create("bus");
  ASSERT_TRUE(definition);
  const auto &bus = (*definition)->root();
  EXPECT_EQ((*definition)->inventory().size(), 43U);

  for (std::size_t zero_value = 0; zero_value < 16; ++zero_value) {
    for (std::size_t one_value = 0; one_value < 16; ++one_value) {
      std::array<bool, width> zero{}, one{};
      for (std::size_t bit = 0; bit < width; ++bit) {
        zero[bit] = ((zero_value >> bit) & 1U) != 0;
        one[bit] = ((one_value >> bit) & 1U) != 0;
      }
      for (bool select : {false, true}) {
        const auto observed =
            (*definition)->observe(bindings(bus, select, zero, one));
        ASSERT_TRUE(observed);
        for (std::size_t bit = 0; bit < width; ++bit) {
          const auto path =
              "bus.selector.bit_" + std::to_string(bit) + ".result.out";
          const auto signal =
              std::ranges::find(observed->signals, path, &Signal::path);
          ASSERT_NE(signal, observed->signals.end()) << path;
          EXPECT_EQ(signal->value.high(), select ? one[bit] : zero[bit]);
        }
      }
    }
  }
}

TEST(SelectedBus, RoutesSelectedWordToParentBoundary) {
  constexpr std::size_t width = 4;
  const auto definition = Definition<BusParent<width>>::create();
  ASSERT_TRUE(definition);
  const auto &bus = (*definition)->root().bus;
  const auto zero = std::array{false, true, false, true};
  const auto one = std::array{true, false, true, false};
  const auto observed = (*definition)->observe(bindings(bus, true, zero, one));
  ASSERT_TRUE(observed);
  for (std::size_t bit = 0; bit < width; ++bit) {
    const auto path = "parent.sink_" + std::to_string(bit) + ".in";
    const auto signal =
        std::ranges::find(observed->signals, path, &Signal::path);
    ASSERT_NE(signal, observed->signals.end()) << path;
    EXPECT_EQ(signal->value.high(), one[bit]);
  }
}

} // namespace
