#pragma once

#include "loom/components/not.h"
#include <algorithm>
#include <expected>
#include <memory>
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
enum class Kind { composite, external_input, external_output, not_gate };
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
concept Atom = std::same_as<T, components::Not> ||
    std::same_as<T, structure::ExternalInput> ||
    std::same_as<T, structure::ExternalOutput>;
struct Node {
  const void *identity;
  ComponentInfo info;
  const structure::Input<1> *input = nullptr;
  const structure::Output<1> *output = nullptr;
};
// One derived representation drives validation, execution, and evidence. No
// callable registry exists: only exact allowlisted types supply atom behavior.
struct Plan {
  std::vector<Node> nodes;
  std::vector<structure::Connection<1>> wires;
  std::vector<std::size_t> drivers, order;
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
      nodes.push_back(
          {&node, {path, Kind::not_gate}, &node.input(), &node.output()});
    } else if constexpr (std::same_as<T, structure::ExternalInput>) {
      nodes.push_back(
          {&node, {path, Kind::external_input}, nullptr, &node.output()});
    } else if constexpr (std::same_as<T, structure::ExternalOutput>) {
      nodes.push_back(
          {&node, {path, Kind::external_output}, &node.input(), nullptr});
    } else {
      nodes.push_back({&node, {path, Kind::composite}});
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
    return {};
  }

  std::expected<void, Error> finalize() {
    std::ranges::sort(nodes, {},
                      [](const Node &node) { return node.info.path; });
    for (std::size_t i = 1; i < nodes.size(); ++i)
      if (nodes[i].info.path == nodes[i - 1].info.path)
        return std::unexpected(Error{"duplicate_path", nodes[i].info.path});
    for (const auto &node : nodes)
      inventory.push_back(node.info);
    drivers.assign(nodes.size(), nodes.size());
    for (const auto &wire : wires) {
      // Null cannot identify a port: composites have no ports of their own.
      if (!wire.source || !wire.destination)
        return std::unexpected(Error{"foreign_endpoint", ""});
      const auto source = std::ranges::find(nodes, wire.source, &Node::output);
      const auto destination =
          std::ranges::find(nodes, wire.destination, &Node::input);
      if (source == nodes.end() || destination == nodes.end())
        return std::unexpected(Error{"foreign_endpoint", ""});
      const auto index = static_cast<std::size_t>(destination - nodes.begin());
      if (drivers[index] != nodes.size())
        return std::unexpected(
            Error{"multiple_drivers", destination->info.path + ".in"});
      drivers[index] = static_cast<std::size_t>(source - nodes.begin());
      connections.push_back(
          {source->info.path + ".out", destination->info.path + ".in"});
    }
    std::ranges::sort(connections, {}, &WireInfo::destination);
    std::vector<bool> ready(nodes.size(), false);
    std::size_t remaining = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i].input) {
        if (drivers[i] == nodes.size())
          return std::unexpected(
              Error{"missing_driver", nodes[i].info.path + ".in"});
        ++remaining;
      } else
        ready[i] = true;
    }
    // Pick the first ready node by canonical path after each evaluation. This
    // tie-break never relies on authored child or connection enumeration.
    while (remaining) {
      const auto found = std::ranges::find_if(nodes, [&](const Node &node) {
        const auto i = static_cast<std::size_t>(&node - nodes.data());
        return !ready[i] && ready[drivers[i]];
      });
      if (found == nodes.end()) {
        auto i = static_cast<std::size_t>(std::ranges::find(ready, false) -
                                          ready.begin());
        // Each unresolved node has one predecessor. After at least as many
        // predecessor links as owned nodes, the walk must be inside a cycle.
        for (std::size_t steps = 0; steps < nodes.size(); ++steps)
          i = drivers[i];
        return std::unexpected(
            Error{"combinational_cycle", nodes[i].info.path + ".in"});
      }
      const auto i = static_cast<std::size_t>(found - nodes.begin());
      ready[i] = true;
      order.push_back(i);
      schedule.push_back(nodes[i].info.path);
      --remaining;
    }
    return {};
  }

  std::expected<Observation, Error>
  observe(std::span<const structure::Binding> inputs) const {
    std::vector<bool> values(nodes.size(), false), bound(nodes.size(), false);
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
      values[i] = nodes[i].info.kind == Kind::not_gate ? !values[drivers[i]]
                                                       : values[drivers[i]];
    Observation result;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i].input)
        result.signals.push_back(
            {nodes[i].info.path + ".in", value::Bit{bool(values[drivers[i]])}});
      if (nodes[i].output)
        result.signals.push_back(
            {nodes[i].info.path + ".out", value::Bit{bool(values[i])}});
    }
    return result;
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
  // With no state atoms admitted yet, all runs are pure observations at edge
  // zero. Each call owns its input/evaluation buffers and returned evidence.
  std::expected<Observation, Error>
  observe(std::span<const structure::Binding> inputs) const {
    return plan_.observe(inputs);
  }

private:
  template <class... Args>
  explicit Definition(Args &&...args) : root_(std::forward<Args>(args)...) {}
  const Root root_;
  detail::Plan plan_;
};
} // namespace loom::simulation
