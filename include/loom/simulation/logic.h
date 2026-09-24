#pragma once

#include "loom/components/and.h"
#include "loom/components/constant_bit.h"
#include "loom/components/d_flip_flop.h"
#include "loom/components/not.h"
#include "loom/components/or.h"
#include <algorithm>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <type_traits>
#include <vector>

namespace loom::simulation {
struct Error {
  std::string operation;
  std::string path;
  bool operator==(const Error &) const = default;
};
enum class Kind {
  composite,
  external_input,
  external_output,
  not_gate,
  and_gate,
  or_gate,
  constant_bit,
  d_flip_flop
};
struct ComponentInfo {
  std::string path;
  Kind kind;
  bool operator==(const ComponentInfo &) const = default;
};
struct WireInfo {
  std::string source, destination;
  bool operator==(const WireInfo &) const = default;
};
struct Signal {
  std::string path;
  value::Bit value;
  bool operator==(const Signal &) const = default;
};
struct Observation {
  std::vector<Signal> signals;
  bool operator==(const Observation &) const = default;
};

namespace detail {
template <class T> struct References : std::false_type {};
template <class... T>
struct References<std::tuple<T...>>
    : std::bool_constant<(std::is_lvalue_reference_v<T> && ...)> {};
template <class T>
concept Composite = requires(const T &node) {
  { node.name } -> std::same_as<const std::string &>;
  node.children();
  node.connections();
  requires References<decltype(node.children())>::value;
};
template <class T>
concept Atom =
    std::same_as<T, components::Not> || std::same_as<T, components::And> ||
    std::same_as<T, components::Or> ||
    std::same_as<T, components::ConstantBit> ||
    std::same_as<T, components::DFlipFlop> ||
    std::same_as<T, structure::ExternalInput> ||
    std::same_as<T, structure::ExternalOutput>;
struct Node {
  const void *identity;
  ComponentInfo info;
  std::vector<std::pair<const structure::Input<1> *, std::string>> inputs;
  const structure::Output<1> *output = nullptr;
  std::optional<bool> constant;
  std::optional<bool> initial;
};
// One derived representation drives validation, execution, and evidence. No
// callable registry exists: only exact allowlisted types supply atom behavior.
struct Plan {
  std::vector<Node> nodes;
  std::vector<structure::Connection<1>> wires;
  std::vector<std::vector<std::size_t>> drivers;
  std::vector<std::size_t> order;
  std::vector<WireInfo> connections;
  std::vector<ComponentInfo> inventory;
  std::vector<std::string> schedule;

  template <class T>
  std::expected<void, Error> discover(const T &node,
                                      const std::string &prefix) {
    static_assert(Atom<T> || Composite<T>, "unsupported circuit component");
    const auto path = prefix.empty() ? node.name : prefix + "." + node.name;
    if (node.name.empty() || node.name.find_first_not_of(
                                 "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRS"
                                 "TUVWXYZ0123456789_-") != std::string::npos)
      return std::unexpected(Error{"invalid_name", path});
    if (std::ranges::find(nodes, &node, &Node::identity) != nodes.end())
      return std::unexpected(Error{"duplicate_component", path});
    if constexpr (std::same_as<T, components::Not>) {
      Node discovered{&node, {path, Kind::not_gate}, {}, &node.output(), {},
                      {}};
      discovered.inputs.emplace_back(&node.input(), "in");
      nodes.push_back(std::move(discovered));
    } else if constexpr (std::same_as<T, components::And>) {
      Node discovered{&node, {path, Kind::and_gate}, {}, &node.output(), {},
                      {}};
      discovered.inputs.emplace_back(&node.left(), "left");
      discovered.inputs.emplace_back(&node.right(), "right");
      nodes.push_back(std::move(discovered));
    } else if constexpr (std::same_as<T, components::Or>) {
      Node discovered{&node, {path, Kind::or_gate}, {}, &node.output(), {}, {}};
      discovered.inputs.emplace_back(&node.left(), "left");
      discovered.inputs.emplace_back(&node.right(), "right");
      nodes.push_back(std::move(discovered));
    } else if constexpr (std::same_as<T, components::ConstantBit>) {
      Node discovered{&node,          {path, Kind::constant_bit}, {},
                      &node.output(), node.fixed_value.high(),    {}};
      nodes.push_back(std::move(discovered));
    } else if constexpr (std::same_as<T, components::DFlipFlop>) {
      Node discovered{&node, {path, Kind::d_flip_flop}, {}, &node.output(),
                      {},    node.initial_value.high()};
      discovered.inputs.emplace_back(&node.data(), "data");
      nodes.push_back(std::move(discovered));
    } else if constexpr (std::same_as<T, structure::ExternalInput>) {
      nodes.push_back(
          {&node, {path, Kind::external_input}, {}, &node.output(), {}, {}});
    } else if constexpr (std::same_as<T, structure::ExternalOutput>) {
      Node discovered{&node, {path, Kind::external_output}, {}, nullptr, {},
                      {}};
      discovered.inputs.emplace_back(&node.input(), "in");
      nodes.push_back(std::move(discovered));
    } else {
      nodes.push_back({&node, {path, Kind::composite}, {}, nullptr, {}, {}});
      std::expected<void, Error> valid;
      if constexpr (std::tuple_size_v<decltype(node.children())> != 0) {
        std::apply(
            [&](const auto &...child) {
              auto visit = [&](const auto &item) {
                if (valid)
                  valid = discover(item, path);
              };
              (visit(child), ...);
            },
            node.children());
      }
      if (!valid)
        return valid;
      std::apply([&](const auto &...wire) { (wires.push_back(wire), ...); },
                 node.connections());
    }
    // The default expected<void> success construction has no runtime operation.
    using Ok = std::expected<void, Error>; // LCOV_EXCL_LINE
    return Ok{};
  }

  std::expected<void, Error> finalize() {
    std::ranges::sort(nodes, {},
                      [](const Node &node) { return node.info.path; });
    for (std::size_t i = 1; i < nodes.size(); ++i)
      if (nodes[i].info.path == nodes[i - 1].info.path)
        return std::unexpected(Error{"duplicate_path", nodes[i].info.path});
    for (const auto &node : nodes)
      inventory.push_back(node.info);
    drivers.resize(nodes.size());
    for (std::size_t i = 0; i < nodes.size(); ++i)
      drivers[i].assign(nodes[i].inputs.size(), nodes.size());
    for (const auto &wire : wires) {
      // Null cannot identify a port: composites have no ports of their own.
      if (!wire.source || !wire.destination)
        return std::unexpected(Error{"foreign_endpoint", ""});
      const auto source = std::ranges::find(nodes, wire.source, &Node::output);
      auto destination = nodes.end();
      std::size_t input_index = 0;
      for (auto it = nodes.begin();
           it != nodes.end() && destination == nodes.end(); ++it) {
        const auto port =
            std::ranges::find(it->inputs, wire.destination,
                              [](const auto &entry) { return entry.first; });
        if (port != it->inputs.end()) {
          destination = it;
          input_index = static_cast<std::size_t>(port - it->inputs.begin());
        }
      }
      if (source == nodes.end() || destination == nodes.end())
        return std::unexpected(Error{"foreign_endpoint", ""});
      const auto index = static_cast<std::size_t>(destination - nodes.begin());
      if (drivers[index][input_index] != nodes.size())
        return std::unexpected(Error{
            "multiple_drivers", destination->info.path + "." +
                                    destination->inputs[input_index].second});
      drivers[index][input_index] =
          static_cast<std::size_t>(source - nodes.begin());
      connections.push_back({source->info.path + ".out",
                             destination->info.path + "." +
                                 destination->inputs[input_index].second});
    }
    std::ranges::sort(connections, {}, &WireInfo::destination);
    std::vector<bool> ready(nodes.size(), false);
    std::size_t remaining = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
      if (!nodes[i].inputs.empty()) {
        for (std::size_t port = 0; port < drivers[i].size(); ++port) {
          if (drivers[i][port] == nodes.size())
            return std::unexpected(
                Error{"missing_driver",
                      nodes[i].info.path + "." + nodes[i].inputs[port].second});
        }
        if (nodes[i].info.kind == Kind::d_flip_flop)
          ready[i] = true;
        else
          ++remaining;
      } else
        ready[i] = true;
    }
    // Pick the first ready node by canonical path after each evaluation. This
    // tie-break never relies on authored child or connection enumeration.
    while (remaining) {
      const auto found = std::ranges::find_if(nodes, [&](const Node &node) {
        const auto i = static_cast<std::size_t>(&node - nodes.data());
        return !ready[i] && std::ranges::all_of(drivers[i], [&](std::size_t d) {
          return ready[d];
        });
      });
      if (found == nodes.end()) {
        auto i = static_cast<std::size_t>(std::ranges::find(ready, false) -
                                          ready.begin());
        // Every input already has a driver, so an unresolved node always has
        // at least one unresolved predecessor when no node can become ready.
        for (std::size_t steps = 0; steps < nodes.size(); ++steps) {
          const auto unresolved = std::ranges::find_if(
              drivers[i], [&](std::size_t d) { return !ready[d]; });
          i = *unresolved;
        }
        const auto cycle_input = std::ranges::find_if(
            drivers[i], [&](std::size_t d) { return !ready[d]; });
        const auto port =
            static_cast<std::size_t>(cycle_input - drivers[i].begin());
        return std::unexpected(
            Error{"combinational_cycle",
                  nodes[i].info.path + "." + nodes[i].inputs[port].second});
      }
      const auto i = static_cast<std::size_t>(found - nodes.begin());
      ready[i] = true;
      order.push_back(i);
      schedule.push_back(nodes[i].info.path);
      --remaining;
    }
    return {};
  }

  struct Evaluation {
    std::vector<bool> values;
    Observation observation;
  };

  std::vector<bool> initial_state() const {
    std::vector<bool> state(nodes.size(), false);
    for (std::size_t i = 0; i < nodes.size(); ++i)
      if (nodes[i].info.kind == Kind::d_flip_flop)
        state[i] = *nodes[i].initial;
    return state;
  }

  std::expected<Evaluation, Error>
  evaluate(std::span<const structure::Binding> inputs,
           const std::vector<bool> &state) const {
    std::vector<bool> values(nodes.size(), false), bound(nodes.size(), false);
    for (std::size_t i = 0; i < nodes.size(); ++i)
      if (nodes[i].constant)
        values[i] = *nodes[i].constant;
      else if (nodes[i].info.kind == Kind::d_flip_flop)
        values[i] = state[i];
    for (const auto &binding : inputs) {
      const auto found =
          std::ranges::find(nodes, binding.port(), &Node::output);
      if (found == nodes.end() || found->info.kind != Kind::external_input)
        return std::unexpected(Error{"foreign_input", ""});
      const auto i = static_cast<std::size_t>(found - nodes.begin());
      if (bound[i])
        return std::unexpected(Error{"duplicate_input", found->info.path});
      bound[i] = true;
      values[i] = binding.value().high();
    }
    for (std::size_t i = 0; i < nodes.size(); ++i)
      if (nodes[i].info.kind == Kind::external_input && !bound[i])
        return std::unexpected(Error{"missing_input", nodes[i].info.path});
    for (const auto i : order)
      if (nodes[i].info.kind == Kind::not_gate)
        values[i] = !values[drivers[i][0]];
      else if (nodes[i].info.kind == Kind::and_gate)
        values[i] = values[drivers[i][0]] && values[drivers[i][1]];
      else if (nodes[i].info.kind == Kind::or_gate)
        values[i] = values[drivers[i][0]] || values[drivers[i][1]];
      else
        values[i] = values[drivers[i][0]];
    Observation result;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
      for (std::size_t port = 0; port < nodes[i].inputs.size(); ++port)
        result.signals.push_back(
            {nodes[i].info.path + "." + nodes[i].inputs[port].second,
             value::Bit{bool(values[drivers[i][port]])}});
      if (nodes[i].output)
        result.signals.push_back(
            {nodes[i].info.path + ".out", value::Bit{bool(values[i])}});
    }
    return Evaluation{std::move(values), std::move(result)};
  }

  std::expected<Observation, Error>
  observe(std::span<const structure::Binding> inputs) const {
    auto result = evaluate(inputs, initial_state());
    if (!result)
      return std::unexpected(result.error());
    return std::move(result->observation);
  }
};
} // namespace detail

template <class T>
concept CircuitRoot = std::is_object_v<T> && !std::is_const_v<T> &&
                      (detail::Atom<T> || detail::Composite<T>);

// Construction happens at its final address. The immutable root owns ports;
// the plan only borrows them. Callers bind through this same const root.
template <CircuitRoot Root> class Definition {
public:
  template <class... Args>
  static std::expected<std::shared_ptr<const Definition>, Error>
  create(Args &&...args) {
    auto result = std::shared_ptr<Definition>(
        new Definition(std::forward<Args>(args)...));
    auto valid = result->plan_.discover(result->root_, "");
    if (valid)
      valid = result->plan_.finalize();
    if (!valid)
      return std::unexpected(valid.error());
    return result;
  }
  Definition(const Definition &) = delete;
  Definition &operator=(const Definition &) = delete;
  const Root &root() const { return root_; }
  const auto &inventory() const { return plan_.inventory; }
  const auto &connections() const { return plan_.connections; }
  const auto &schedule() const { return plan_.schedule; }
  // This definition-level view uses declared initial state. Evolving state
  // belongs to independent Simulation instances created from this definition.
  std::expected<Observation, Error>
  observe(std::span<const structure::Binding> inputs) const {
    return plan_.observe(inputs);
  }

private:
  template <class... Args>
  explicit Definition(Args &&...args) : root_(std::forward<Args>(args)...) {}
  const Root root_;
  detail::Plan plan_;
  template <CircuitRoot> friend class Simulation;
};

struct Commit {
  std::string path;
  value::Bit old_value;
  value::Bit data_value;
  value::Bit new_value;
  bool operator==(const Commit &) const = default;
};
struct Edge {
  std::uint64_t index;
  Observation evaluated;
  std::vector<Commit> commits;
  bool operator==(const Edge &) const = default;
};

template <CircuitRoot Root> class Simulation {
public:
  Simulation(const Simulation &) = delete;
  Simulation &operator=(const Simulation &) = delete;
  Simulation(Simulation &&) = delete;
  Simulation &operator=(Simulation &&) = delete;

  static std::expected<std::shared_ptr<Simulation>, Error>
  create(std::shared_ptr<const Definition<Root>> definition) {
    if (!definition)
      return std::unexpected(Error{"null_definition", ""});
    auto initial = definition->plan_.initial_state();
    return std::shared_ptr<Simulation>(
        new Simulation(std::move(definition), std::move(initial)));
  }

  std::uint64_t edge_index() const { return edge_index_; }
  std::expected<Observation, Error>
  observe(std::span<const structure::Binding> inputs) const {
    auto result = definition_->plan_.evaluate(inputs, state_);
    if (!result)
      return std::unexpected(result.error());
    return std::move(result->observation);
  }

  std::expected<Edge, Error> step(std::span<const structure::Binding> inputs) {
    auto evaluation = definition_->plan_.evaluate(inputs, state_);
    if (!evaluation)
      return std::unexpected(evaluation.error());
    auto next = state_;
    std::vector<Commit> commits;
    for (std::size_t i = 0; i < definition_->plan_.nodes.size(); ++i) {
      const auto &node = definition_->plan_.nodes[i];
      if (node.info.kind != Kind::d_flip_flop)
        continue;
      const bool data = evaluation->values[definition_->plan_.drivers[i][0]];
      next[i] = data;
      const bool old = state_[i];
      const bool committed = next[i];
      commits.push_back({node.info.path, value::Bit{old}, value::Bit{data},
                         value::Bit{committed}});
    }
    Edge result{edge_index_, std::move(evaluation->observation),
                std::move(commits)};
    state_ = std::move(next);
    ++edge_index_;
    return result;
  }

private:
  Simulation(std::shared_ptr<const Definition<Root>> definition,
             std::vector<bool> state)
      : definition_(std::move(definition)), state_(std::move(state)) {}
  std::shared_ptr<const Definition<Root>> definition_;
  std::vector<bool> state_;
  std::uint64_t edge_index_ = 0;
};
} // namespace loom::simulation
