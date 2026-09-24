#include "loom/simulation/logic.h"
#include <array>
#include <gtest/gtest.h>

namespace {
using loom::components::And;
using loom::components::ConstantBit;
using loom::components::Not;
using loom::components::Or;
using loom::simulation::Definition;
using loom::simulation::Error;
using loom::simulation::Kind;
using loom::structure::connect;
using loom::structure::ExternalInput;
using loom::structure::ExternalOutput;
using loom::value::Bit;

struct Inverter {
  const std::string name = "inverter";
  ExternalInput input{"input"};
  Not gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(input, gate, output); }
  auto connections() const {
    return std::tuple{connect(input.output(), gate.input()),
                      connect(gate.output(), output.input())};
  }
};
TEST(Not, TruthTableAndTransientInputsBeforeAnyEdge) {
  const auto definition = Definition<Inverter>::create();
  ASSERT_TRUE(definition);
  const auto &root = (*definition)->root();
  const std::array low{root.input.bind(Bit{false})},
      high{root.input.bind(Bit{true})};
  const auto first = (*definition)->observe(low);
  ASSERT_TRUE(first);
  EXPECT_EQ(first->signals, (std::vector<loom::simulation::Signal>{
                                {"inverter.gate.in", Bit{false}},
                                {"inverter.gate.out", Bit{true}},
                                {"inverter.input.out", Bit{false}},
                                {"inverter.output.in", Bit{true}}}));
  const auto second = (*definition)->observe(high);
  ASSERT_TRUE(second);
  EXPECT_FALSE(second->signals.back().value.high());
  EXPECT_EQ(*(*definition)->observe(low), *first);
  EXPECT_TRUE(first->signals.back().value.high());
  EXPECT_EQ((*definition)->observe({}).error(),
            (Error{"missing_input", "inverter.input"}));
  const std::array duplicate{low[0], high[0]};
  EXPECT_EQ((*definition)->observe(duplicate).error(),
            (Error{"duplicate_input", "inverter.input"}));
  const auto other = Definition<Inverter>::create();
  ASSERT_TRUE(other);
  const std::array foreign{(*other)->root().input.bind(Bit{false})};
  EXPECT_EQ((*definition)->observe(foreign).error().operation, "foreign_input");
  EXPECT_EQ(*(*definition)->observe(low), *first);
}

struct Pair {
  const std::string name;
  ExternalInput input{"input"};
  Not first{"z_first"}, second{"a_second"};
  ExternalOutput output{"output"}, fan{"fan"};
  const bool reverse, bypass;
  explicit Pair(std::string label = "pair", bool backwards = false,
                bool skip_second = false)
      : name(std::move(label)), reverse(backwards), bypass(skip_second) {}
  auto children() const {
    return reverse ? std::tie(input, second, first, output, fan)
                   : std::tie(input, first, second, output, fan);
  }
  auto connections() const {
    const auto a = connect(input.output(), first.input());
    const auto b = connect(first.output(), second.input());
    const auto c =
        connect(bypass ? first.output() : second.output(), output.input());
    const auto d = connect(first.output(), fan.input());
    return reverse ? std::tuple{d, c, b, a} : std::tuple{a, b, c, d};
  }
};
TEST(Not, TwoGateCompositionInventoryScheduleAndFanout) {
  const auto forward = Definition<Pair>::create();
  const auto reverse = Definition<Pair>::create("pair", true);
  ASSERT_TRUE(forward);
  ASSERT_TRUE(reverse);
  EXPECT_EQ((*forward)->inventory(),
            (std::vector<loom::simulation::ComponentInfo>{
                {"pair", Kind::composite},
                {"pair.a_second", Kind::not_gate},
                {"pair.fan", Kind::external_output},
                {"pair.input", Kind::external_input},
                {"pair.output", Kind::external_output},
                {"pair.z_first", Kind::not_gate}}));
  EXPECT_EQ((*forward)->inventory(), (*reverse)->inventory());
  EXPECT_EQ((*forward)->connections(), (*reverse)->connections());
  EXPECT_EQ((*forward)->connections().size(), 4U);
  EXPECT_EQ((*forward)->schedule(),
            (std::vector<std::string>{"pair.z_first", "pair.a_second",
                                      "pair.fan", "pair.output"}));
  EXPECT_EQ((*forward)->schedule(), (*reverse)->schedule());
  for (bool high : {false, true}) {
    const std::array a{(*forward)->root().input.bind(Bit{high})};
    const std::array b{(*reverse)->root().input.bind(Bit{high})};
    const auto observed = (*forward)->observe(a);
    ASSERT_TRUE(observed);
    EXPECT_EQ(*observed, *(*reverse)->observe(b));
    const auto &v = observed->signals;
    EXPECT_EQ(v[0].value.high(),
              !high); // Second gate consumes the first gate.
    EXPECT_EQ(v[1].value.high(), high);
    EXPECT_EQ(v[2].value.high(),
              !high); // Fan-out exposes intermediate value.
    EXPECT_EQ(v[4].value.high(),
              high); // Assembly output is the second result.
  }
}
// Deliberately inject a parent wiring fault. Both NOTs still satisfy their
// contracts; the first wrong boundary is the parent's output connection.
TEST(Not, DrillDownLocalizesInjectedParentWiringFault) {
  const auto good = *Definition<Pair>::create();
  const auto faulty = *Definition<Pair>::create("pair", false, true);
  for (bool high : {false, true}) {
    const std::array a{good->root().input.bind(Bit{high})};
    const std::array b{faulty->root().input.bind(Bit{high})};
    const auto expected = good->observe(a);
    const auto actual = faulty->observe(b);
    ASSERT_TRUE(expected);
    ASSERT_TRUE(actual);
    for (const auto &signal : expected->signals) {
      const auto found = std::ranges::find(actual->signals, signal.path,
                                           &loom::simulation::Signal::path);
      ASSERT_NE(found, actual->signals.end());
      if (signal.path == "pair.output.in") {
        EXPECT_NE(found->value, signal.value);
        EXPECT_EQ(found->value.high(), !high);
      } else
        EXPECT_EQ(*found, signal);
    }
    EXPECT_EQ(
        faulty->connections()[2],
        (loom::simulation::WireInfo{"pair.z_first.out", "pair.output.in"}));
    EXPECT_EQ(
        good->connections()[2],
        (loom::simulation::WireInfo{"pair.a_second.out", "pair.output.in"}));
  }
}
struct Nested {
  const std::string name = "nested";
  Pair left{"left"}, right{"right"};
  auto children() const { return std::tie(left, right); }
  auto connections() const { return std::tuple{}; }
};
TEST(Not, NestedIndependentInputsAndLifetime) {
  auto definition = *Definition<Nested>::create();
  auto retained = definition;
  const std::array inputs{definition->root().left.input.bind(Bit{false}),
                          definition->root().right.input.bind(Bit{true})};
  definition.reset();
  const auto result = retained->observe(inputs);
  ASSERT_TRUE(result);
  const auto find = [&](const std::string &path) {
    return std::ranges::find(result->signals, path,
                             &loom::simulation::Signal::path)
        ->value.high();
  };
  EXPECT_FALSE(find("nested.left.output.in"));
  EXPECT_TRUE(find("nested.right.output.in"));
  const std::array incomplete{inputs[0]};
  EXPECT_EQ(retained->observe(incomplete).error(),
            (Error{"missing_input", "nested.right.input"}));
}

struct Invalid {
  const std::string name = "bad";
  ExternalInput source{"source"};
  Not a{"a"}, b;
  Not foreign{"foreign"};
  const int mode;
  explicit Invalid(int choice) : b(choice == 0 ? "a" : "b"), mode(choice) {}
  auto children() const {
    return mode == 1 ? std::tie(source, a, a) : std::tie(source, a, b);
  }
  auto connections() const {
    return std::tuple{connect(source.output(), a.input()),
                      mode == 2 ? connect(foreign.output(), b.input())
                                : connect(source.output(), a.input())};
  }
};
TEST(Not, RejectsInvalidStructureBeforeObservation) {
  EXPECT_EQ(Definition<Invalid>::create(0).error().operation, "duplicate_path");
  EXPECT_EQ(Definition<Invalid>::create(1).error().operation,
            "duplicate_component");
  EXPECT_EQ(Definition<Invalid>::create(2).error().operation,
            "foreign_endpoint");
  EXPECT_EQ(Definition<Invalid>::create(3).error(),
            (Error{"multiple_drivers", "bad.a.in"}));
  EXPECT_EQ(Definition<Not>::create("missing").error(),
            (Error{"missing_driver", "missing.in"}));
  for (const auto *name : {"", "bad.name", "bad name"})
    EXPECT_EQ(Definition<Pair>::create(name).error().operation, "invalid_name");
}
struct Cycle {
  const std::string name = "cycle";
  Not a{"a"}, b{"b"};
  ExternalOutput downstream{"aaa_downstream"};
  auto children() const { return std::tie(downstream, b, a); }
  auto connections() const {
    return std::tuple{connect(a.output(), b.input()),
                      connect(b.output(), a.input()),
                      connect(a.output(), downstream.input())};
  }
};
TEST(Not, CycleErrorIdentifiesActualCycleRatherThanBlockedOutput) {
  const auto definition = Definition<Cycle>::create();
  ASSERT_FALSE(definition);
  EXPECT_EQ(definition.error(), (Error{"combinational_cycle", "cycle.a.in"}));
}
struct NullWire {
  const std::string name = "null";
  Not gate{"gate"};
  auto children() const { return std::tie(gate); }
  auto connections() const {
    return std::tuple{loom::structure::Connection<1>{nullptr, &gate.input()}};
  }
};
TEST(Not, RejectsNullEndpoint) {
  EXPECT_EQ(Definition<NullWire>::create().error().operation,
            "foreign_endpoint");
}
struct Empty {
  const std::string name = "empty";
  auto children() const { return std::tuple{}; }
  auto connections() const { return std::tuple{}; }
};
TEST(Not, ClosedEmptyAssemblyHasAnEmptyInputSnapshot) {
  const auto definition = Definition<Empty>::create();
  ASSERT_TRUE(definition);
  const auto result = (*definition)->observe({});
  ASSERT_TRUE(result);
  EXPECT_TRUE(result->signals.empty());
  EXPECT_TRUE((*definition)->schedule().empty());
}
struct ForeignDestination {
  const std::string name = "foreign_destination";
  ExternalInput input{"input"};
  ExternalOutput foreign{"foreign"};
  auto children() const { return std::tie(input); }
  auto connections() const {
    return std::tuple{connect(input.output(), foreign.input())};
  }
};
TEST(Not, ForeignDestinationCannotExtendOwnership) {
  EXPECT_EQ(Definition<ForeignDestination>::create().error().operation,
            "foreign_endpoint");
}
struct SelfLoop {
  const std::string name = "self";
  Not gate{"gate"};
  auto children() const { return std::tie(gate); }
  auto connections() const {
    return std::tuple{connect(gate.output(), gate.input())};
  }
};
TEST(Not, SelfFeedbackIsRejected) {
  EXPECT_EQ(Definition<SelfLoop>::create().error(),
            (Error{"combinational_cycle", "self.gate.in"}));
}
struct AndOnly {
  const std::string name = "and_only";
  ExternalInput left{"left"}, right{"right"};
  And gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(left, right, gate, output); }
  auto connections() const {
    return std::tuple{connect(left.output(), gate.left()),
                      connect(right.output(), gate.right()),
                      connect(gate.output(), output.input())};
  }
};
TEST(And, TruthTableUsesDistinctInputPortsAndDerivedDependencies) {
  const auto definition = Definition<AndOnly>::create();
  ASSERT_TRUE(definition);
  EXPECT_EQ((*definition)->connections(),
            (std::vector<loom::simulation::WireInfo>{
                {"and_only.left.out", "and_only.gate.left"},
                {"and_only.right.out", "and_only.gate.right"},
                {"and_only.gate.out", "and_only.output.in"}}));
  EXPECT_EQ((*definition)->schedule(),
            (std::vector<std::string>{"and_only.gate", "and_only.output"}));
  for (const bool left : {false, true})
    for (const bool right : {false, true}) {
      const std::array inputs{(*definition)->root().left.bind(Bit{left}),
                              (*definition)->root().right.bind(Bit{right})};
      const auto observation = (*definition)->observe(inputs);
      ASSERT_TRUE(observation);
      const auto find = [&](const std::string &path) {
        return std::ranges::find(observation->signals, path,
                                 &loom::simulation::Signal::path)
            ->value.high();
      };
      EXPECT_EQ(find("and_only.gate.left"), left);
      EXPECT_EQ(find("and_only.gate.right"), right);
      EXPECT_EQ(find("and_only.output.in"), left && right);
    }
}

struct NotAnd {
  const std::string name = "not_and";
  ExternalInput input{"input"}, other{"other"};
  Not invert{"invert"};
  And gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(input, other, invert, gate, output); }
  auto connections() const {
    return std::tuple{connect(input.output(), invert.input()),
                      connect(invert.output(), gate.left()),
                      connect(other.output(), gate.right()),
                      connect(gate.output(), output.input())};
  }
};
TEST(And, ComposesWithNotThroughTheSameObservedCircuit) {
  const auto definition = Definition<NotAnd>::create();
  ASSERT_TRUE(definition);
  for (const bool input : {false, true})
    for (const bool other : {false, true}) {
      const std::array bindings{(*definition)->root().input.bind(Bit{input}),
                                (*definition)->root().other.bind(Bit{other})};
      const auto observed = (*definition)->observe(bindings);
      ASSERT_TRUE(observed);
      EXPECT_EQ(observed->signals.back().value.high(), !input && other);
    }
}

struct OrOnly {
  const std::string name = "or_only";
  ExternalInput left{"left"}, right{"right"};
  Or gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(left, right, gate, output); }
  auto connections() const {
    return std::tuple{connect(left.output(), gate.left()),
                      connect(right.output(), gate.right()),
                      connect(gate.output(), output.input())};
  }
};
TEST(Or, TruthTableUsesDistinctInputPortsAndDerivedDependencies) {
  const auto definition = Definition<OrOnly>::create();
  ASSERT_TRUE(definition);
  const auto gate =
      std::ranges::find((*definition)->inventory(), "or_only.gate",
                        &loom::simulation::ComponentInfo::path);
  ASSERT_NE(gate, (*definition)->inventory().end());
  EXPECT_EQ(gate->kind, Kind::or_gate);
  EXPECT_EQ((*definition)->connections(),
            (std::vector<loom::simulation::WireInfo>{
                {"or_only.left.out", "or_only.gate.left"},
                {"or_only.right.out", "or_only.gate.right"},
                {"or_only.gate.out", "or_only.output.in"}}));
  EXPECT_EQ((*definition)->schedule(),
            (std::vector<std::string>{"or_only.gate", "or_only.output"}));
  for (const bool left : {false, true})
    for (const bool right : {false, true}) {
      const std::array inputs{(*definition)->root().left.bind(Bit{left}),
                              (*definition)->root().right.bind(Bit{right})};
      const auto observation = (*definition)->observe(inputs);
      ASSERT_TRUE(observation);
      const auto find = [&](const std::string &path) {
        return std::ranges::find(observation->signals, path,
                                 &loom::simulation::Signal::path)
            ->value.high();
      };
      EXPECT_EQ(find("or_only.gate.left"), left);
      EXPECT_EQ(find("or_only.gate.right"), right);
      EXPECT_EQ(find("or_only.output.in"), left || right);
    }
}

struct NotOr {
  const std::string name = "not_or";
  ExternalInput input{"input"}, other{"other"};
  Not invert{"invert"};
  Or gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(input, other, invert, gate, output); }
  auto connections() const {
    return std::tuple{connect(input.output(), invert.input()),
                      connect(invert.output(), gate.left()),
                      connect(other.output(), gate.right()),
                      connect(gate.output(), output.input())};
  }
};
TEST(Or, ComposesWithNotThroughTheSameObservedCircuit) {
  const auto definition = Definition<NotOr>::create();
  ASSERT_TRUE(definition);
  for (const bool input : {false, true})
    for (const bool other : {false, true}) {
      const std::array bindings{(*definition)->root().input.bind(Bit{input}),
                                (*definition)->root().other.bind(Bit{other})};
      const auto observed = (*definition)->observe(bindings);
      ASSERT_TRUE(observed);
      EXPECT_EQ(observed->signals.back().value.high(), !input || other);
    }
}
struct AndRightCycle {
  const std::string name = "and_right_cycle";
  ExternalInput left{"left"};
  And gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(left, gate, output); }
  auto connections() const {
    return std::tuple{connect(left.output(), gate.left()),
                      connect(gate.output(), gate.right()),
                      connect(gate.output(), output.input())};
  }
};
TEST(And, CycleErrorNamesTheActualRightInputPort) {
  EXPECT_EQ(Definition<AndRightCycle>::create().error(),
            (Error{"combinational_cycle", "and_right_cycle.gate.right"}));
}

struct InputAndConstant {
  const std::string name = "input_and_constant";
  ExternalInput input{"input"};
  ConstantBit one{"one", Bit{true}};
  And gate{"gate"};
  ExternalOutput output{"output"};
  auto children() const { return std::tie(input, one, gate, output); }
  auto connections() const {
    return std::tuple{connect(input.output(), gate.left()),
                      connect(one.output(), gate.right()),
                      connect(gate.output(), output.input())};
  }
};

TEST(ConstantBit, BothValuesAreInputlessSourcesAndNotScheduled) {
  for (const bool high : {false, true}) {
    const auto definition =
        Definition<ConstantBit>::create("constant", Bit{high});
    ASSERT_TRUE(definition);
    EXPECT_EQ((*definition)->inventory(),
              (std::vector<loom::simulation::ComponentInfo>{
                  {"constant", Kind::constant_bit}}));
    EXPECT_TRUE((*definition)->schedule().empty());
    const auto observed = (*definition)->observe({});
    ASSERT_TRUE(observed);
    EXPECT_EQ(observed->signals, (std::vector<loom::simulation::Signal>{
                                     {"constant.out", Bit{high}}}));
  }
}

TEST(ConstantBit, ComposesWithInputAndGateInParent) {
  const auto definition = Definition<InputAndConstant>::create();
  ASSERT_TRUE(definition);
  EXPECT_EQ((*definition)->schedule(),
            (std::vector<std::string>{"input_and_constant.gate",
                                      "input_and_constant.output"}));
  for (const bool input : {false, true}) {
    const std::array binding{(*definition)->root().input.bind(Bit{input})};
    const auto observed = (*definition)->observe(binding);
    ASSERT_TRUE(observed);
    const auto &signals = observed->signals;
    const auto one = std::ranges::find(signals, "input_and_constant.one.out",
                                       &loom::simulation::Signal::path);
    ASSERT_NE(one, signals.end());
    EXPECT_TRUE(one->value.high());
    EXPECT_EQ(signals.back().value.high(), input);
  }
}

struct LiteralName {
  const char *name = "literal";
  auto children() const { return std::tuple{}; }
  auto connections() const { return std::tuple{}; }
};
static_assert(!loom::simulation::CircuitRoot<LiteralName>);
static_assert(!std::is_constructible_v<Bit, unsigned>);
static_assert(!std::is_copy_constructible_v<Not>);
static_assert(!std::is_copy_constructible_v<And>);
static_assert(!std::is_copy_constructible_v<Or>);
static_assert(!std::is_copy_constructible_v<ConstantBit>);
static_assert(!loom::simulation::CircuitRoot<Inverter &>);
static_assert(!loom::simulation::CircuitRoot<Inverter *>);
static_assert(!loom::simulation::CircuitRoot<And &>);
static_assert(!loom::simulation::CircuitRoot<Or &>);
template <class A, class B>
concept Connectable = requires(const A &a, const B &b) { connect(a, b); };
static_assert(
    !Connectable<loom::structure::Input<1>, loom::structure::Output<1>>);
static_assert(
    !Connectable<loom::structure::Output<1>, loom::structure::Input<2>>);
} // namespace
