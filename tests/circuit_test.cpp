#include "loom/circuit.h"
#include <array>
#include <gtest/gtest.h>

namespace {
struct Pair {
  const std::string name;
  loom::Register<4> a, b;
  bool reverse;
  Pair(std::string label = "pair", unsigned x = 1, unsigned y = 2,
       bool backwards = false)
      : name(std::move(label)), a("a", x), b("b", y), reverse(backwards) {}
  auto children() const { return reverse ? std::tie(b, a) : std::tie(a, b); }
  auto connections() const {
    return std::tuple{loom::connect(a, b), loom::connect(b, a)};
  }
};

TEST(Circuit, ExhaustiveTransferHoldAndSwap) {
  for (unsigned x = 0; x < 16; ++x)
    for (unsigned y = 0; y < 16; ++y) {
      auto definition = loom::Definition<Pair>::create("pair", x, y);
      ASSERT_TRUE(definition);
      auto sim = loom::Simulation<Pair>::create(*definition);
      const std::array enables{std::string("pair.a"), std::string("pair.b")};
      const auto edge = sim->step(enables);
      ASSERT_TRUE(edge);
      EXPECT_EQ(edge->index, 0U);
      EXPECT_EQ(edge->registers, (std::vector<loom::Sample>{{"pair.a", x, y},
                                                            {"pair.b", y, x}}));
      const auto held = sim->step({});
      EXPECT_EQ(held->registers[0].after, y);
      EXPECT_EQ(held->registers[1].after, x);
      const std::array only_a{std::string("pair.a")};
      EXPECT_EQ(sim->step(only_a)->registers[0].after, x);
    }
}

TEST(Circuit, IndependentRunsOrderAndAtomicFailure) {
  auto definition = loom::Definition<Pair>::create();
  auto sim = loom::Simulation<Pair>::create(*definition);
  auto other = loom::Simulation<Pair>::create(*definition);
  auto reversed = loom::Simulation<Pair>::create(
      *loom::Definition<Pair>::create("pair", 1, 2, true));
  const std::array enables{std::string("pair.a"), std::string("pair.b")};
  const std::array bad{std::string("pair.a"), std::string("absent")};
  EXPECT_EQ(sim->step(bad).error(), (loom::Error{"unknown_enable", "absent"}));
  const std::array duplicate{std::string("pair.a"), std::string("pair.a")};
  EXPECT_EQ(sim->step(duplicate).error().operation, "duplicate_enable");
  const auto first = sim->step(enables);
  EXPECT_EQ(*first, *other->step(enables));
  EXPECT_EQ(*first, *reversed->step(enables));
  EXPECT_EQ(first->index, 0U);
  sim->step(enables);
  EXPECT_EQ(first->registers[0].after, 2U);
  EXPECT_EQ(loom::Simulation<Pair>::create(nullptr).error().operation,
            "null_definition");
}

struct Nested {
  const std::string name = "root";
  Pair left{"left", 3, 4}, right{"right", 7, 8};
  auto children() const { return std::tie(left, right); }
  auto connections() const { return std::tuple{}; }
};
TEST(Circuit, NestedSameWidthTransfersAndLifetime) {
  auto sim =
      loom::Simulation<Nested>::create(*loom::Definition<Nested>::create());
  const std::array enables{std::string("root.left.b"),
                           std::string("root.right.b")};
  const auto edge = sim->step(enables);
  ASSERT_TRUE(edge);
  EXPECT_EQ(edge->registers[1].after, 3U);
  EXPECT_EQ(edge->registers[3].after, 7U);
}

struct Invalid {
  const std::string name = "bad";
  loom::Register<4> a{"a", 0}, b;
  loom::Register<4> foreign{"foreign", 0};
  int mode;
  explicit Invalid(int choice) : b(choice == 0 ? "a" : "b", 0), mode(choice) {}
  auto children() const { return std::tie(a, b); }
  auto connections() const {
    return std::tuple{loom::connect(a, a), mode == 1 ? loom::connect(foreign, b)
                                                     : loom::connect(b, a)};
  }
};
TEST(Circuit, RejectsInvalidTopology) {
  EXPECT_EQ(loom::Definition<Invalid>::create(0).error().operation,
            "duplicate_or_empty_path");
  EXPECT_EQ(loom::Definition<Invalid>::create(1).error().operation,
            "foreign_endpoint");
  EXPECT_EQ(loom::Definition<Invalid>::create(2).error().operation,
            "multiple_drivers");
}
struct Missing {
  const std::string name = "root";
  loom::Register<1> a{"a", 2};
  auto children() const { return std::tie(a); }
  auto connections() const { return std::tuple{}; }
};
TEST(Circuit, RejectsMissingDriverAndMasksReset) {
  EXPECT_EQ(loom::Definition<Missing>::create().error().operation,
            "missing_driver");
  const loom::Register<64> wide{"wide", UINT64_MAX};
  const loom::Register<1> bit{"bit", 2};
  EXPECT_EQ(wide.reset, UINT64_MAX);
  EXPECT_EQ(bit.reset, 0U);
}
TEST(Circuit, EdgeBudgetStopsWithoutWrappingOrAdvancing) {
  auto sim =
      loom::Simulation<Pair>::create(*loom::Definition<Pair>::create(), 1);
  EXPECT_EQ(sim->step({})->index, 0U);
  EXPECT_EQ(sim->step({}).error().operation, "edge_limit");
  EXPECT_EQ(sim->step({}).error().operation, "edge_limit");
}
} // namespace
