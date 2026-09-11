#pragma once

// The first circuit vocabulary: explicit registers and typed connections.
// Inventories are derived from owned objects; there is no CPU or global
// registry.
#include <algorithm>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace loom {

template <unsigned Width> struct Register {
  static_assert(Width > 0 && Width <= 64, "register width must be 1..64");
  static constexpr unsigned width = Width;
  static constexpr std::uint64_t mask = UINT64_MAX >> (64 - Width);
  const std::string name;
  const std::uint64_t reset;
  Register(std::string label, std::uint64_t initial)
      : name(std::move(label)), reset(initial & mask) {}
};

template <unsigned Width> struct Connection {
  const Register<Width> *source;
  const Register<Width> *destination;
};

template <unsigned Width>
auto connect(const Register<Width> &source,
             const Register<Width> &destination) {
  return Connection<Width>{&source, &destination};
}

struct Error {
  std::string operation;
  std::string path;
  bool operator==(const Error &) const = default;
};

namespace detail {
struct Storage {
  const void *identity;
  std::string path;
  std::uint64_t reset;
};
struct Wire {
  const void *source;
  const void *destination;
};

// A composite exposes actual children and local connections as tuples.
// Recurse through ownership only; connections never extend object lifetime.
template <class Node>
void discover(const Node &node, const std::string &prefix,
              std::vector<Storage> &storage, std::vector<Wire> &wires) {
  const auto path = prefix.empty() ? node.name : prefix + "." + node.name;
  if constexpr (requires { Node::width; }) {
    storage.push_back({&node, path, node.reset});
  } else {
    std::apply(
        [&](const auto &...child) {
          (discover(child, path, storage, wires), ...);
        },
        node.children());
    std::apply(
        [&](const auto &...wire) {
          (wires.push_back({wire.source, wire.destination}), ...);
        },
        node.connections());
  }
}
} // namespace detail

// A finalized definition retains its root at a stable address. Simulations
// share the const definition and own only evolving state.
template <class Root> class Definition {
public:
  template <class... Args>
  static std::expected<std::shared_ptr<const Definition>, Error>
  create(Args &&...args) {
    auto result = std::shared_ptr<Definition>(
        new Definition(std::forward<Args>(args)...));
    std::vector<detail::Wire> wires;
    detail::discover(result->root_, "", result->storage_, wires);
    auto &storage = result->storage_;
    std::ranges::sort(storage, {}, &detail::Storage::path);
    for (std::size_t i = 0; i < storage.size(); ++i) {
      if (storage[i].path.empty() ||
          (i && storage[i].path == storage[i - 1].path))
        return std::unexpected(
            Error{"duplicate_or_empty_path", storage[i].path});
    }
    result->sources_.resize(storage.size(), storage.size());
    for (const auto &wire : wires) {
      const auto source =
          std::ranges::find(storage, wire.source, &detail::Storage::identity);
      const auto destination = std::ranges::find(storage, wire.destination,
                                                 &detail::Storage::identity);
      if (source == storage.end() || destination == storage.end())
        return std::unexpected(Error{"foreign_endpoint", ""});
      const auto index =
          static_cast<std::size_t>(destination - storage.begin());
      if (result->sources_[index] != storage.size())
        return std::unexpected(Error{"multiple_drivers", destination->path});
      result->sources_[index] =
          static_cast<std::size_t>(source - storage.begin());
    }
    for (std::size_t i = 0; i < storage.size(); ++i)
      if (result->sources_[i] == storage.size())
        return std::unexpected(Error{"missing_driver", storage[i].path});
    return result;
  }
  Definition(const Definition &) = delete;
  Definition &operator=(const Definition &) = delete;
  const auto &storage() const { return storage_; }
  const auto &sources() const { return sources_; }

private:
  template <class... Args>
  explicit Definition(Args &&...args) : root_(std::forward<Args>(args)...) {}
  const Root root_;
  std::vector<detail::Storage> storage_;
  std::vector<std::size_t> sources_;
};

struct Sample {
  std::string path;
  std::uint64_t before;
  std::uint64_t after;
  bool operator==(const Sample &) const = default;
};
struct Edge {
  std::uint64_t index;
  std::vector<Sample> registers;
  bool operator==(const Edge &) const = default;
};

template <class Root> class Simulation {
public:
  static std::expected<Simulation, Error>
  create(std::shared_ptr<const Definition<Root>> definition,
         std::uint64_t edge_limit = UINT64_MAX) {
    if (!definition)
      return std::unexpected(Error{"null_definition", ""});
    return Simulation(std::move(definition), edge_limit);
  }

  // Enables are explicit external inputs for this first slice. Validate all
  // names before computing proposals; errors cannot partially advance state.
  std::expected<Edge, Error> step(std::span<const std::string> enabled) {
    if (cycle_ == edge_limit_)
      return std::unexpected(Error{"edge_limit", ""});
    std::vector<bool> selected(values_.size(), false);
    const auto &storage = definition_->storage();
    for (const auto &path : enabled) {
      const auto found =
          std::ranges::find(storage, path, &detail::Storage::path);
      if (found == storage.end())
        return std::unexpected(Error{"unknown_enable", path});
      const auto index = static_cast<std::size_t>(found - storage.begin());
      if (selected[index])
        return std::unexpected(Error{"duplicate_enable", path});
      selected[index] = true;
    }
    auto next = values_;
    Edge edge{cycle_, {}};
    for (std::size_t i = 0; i < values_.size(); ++i) {
      if (selected[i])
        next[i] = values_[definition_->sources()[i]];
      edge.registers.push_back({storage[i].path, values_[i], next[i]});
    }
    values_ = std::move(next);
    ++cycle_;
    return edge;
  }

private:
  explicit Simulation(std::shared_ptr<const Definition<Root>> definition,
                      std::uint64_t edge_limit)
      : definition_(std::move(definition)), edge_limit_(edge_limit) {
    for (const auto &item : definition_->storage())
      values_.push_back(item.reset);
  }
  std::shared_ptr<const Definition<Root>> definition_;
  std::vector<std::uint64_t> values_;
  std::uint64_t cycle_ = 0;
  std::uint64_t edge_limit_;
};
} // namespace loom
