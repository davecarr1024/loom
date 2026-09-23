#pragma once

// Transitional register-only structure. The construction plan replaces wide
// atomic registers with composed D flip-flops; these are baseline contracts.
#include <concepts>
#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace loom {
template <unsigned Width> struct Register;
template <unsigned Width> struct Input {
  const Register<Width> *owner;
};
template <unsigned Width> struct Output {
  const Register<Width> *owner;
};

template <unsigned Width> struct Register {
  static_assert(Width > 0 && Width <= 64, "register width must be 1..64");
  static constexpr unsigned width = Width;
  static constexpr std::uint64_t mask = UINT64_MAX >> (64 - Width);
  const std::string name;
  const std::uint64_t reset;
  Register(std::string label, std::uint64_t initial)
      : name(std::move(label)), reset(initial & mask) {}
  Input<Width> input() const { return {this}; }
  Output<Width> output() const { return {this}; }
};

template <unsigned Width> struct Connection {
  const Register<Width> *source;
  const Register<Width> *destination;
};
template <unsigned Width>
auto connect(Output<Width> source, Input<Width> destination) {
  return Connection<Width>{source.owner, destination.owner};
}
// Compatibility for the initial transfer example, through the same endpoints.
template <unsigned Width>
auto connect(const Register<Width> &source,
             const Register<Width> &destination) {
  return connect(source.output(), destination.input());
}
template <class T> inline constexpr bool is_register = false;
template <unsigned W> inline constexpr bool is_register<Register<W>> = true;
template <class T>
concept RegisterComponent = is_register<std::remove_cvref_t<T>>;
namespace structure_detail {
template <class Tuple> struct ReferenceTuple : std::false_type {};
template <class... T>
struct ReferenceTuple<std::tuple<T...>>
    : std::bool_constant<(std::is_lvalue_reference_v<T> && ...)> {};
} // namespace structure_detail

template <class T>
concept CompositeComponent = requires(const T &node) {
  { node.name } -> std::convertible_to<std::string>;
  node.children();
  node.connections();
  requires structure_detail::ReferenceTuple<decltype(node.children())>::value;
};
template <class T>
concept OwnedRoot = std::is_object_v<T> && !std::is_const_v<T> &&
                    (RegisterComponent<T> || CompositeComponent<T>);

// Counts add across actual owned child types, independently of connections.
template <class T> struct CircuitFacts {
  static_assert(RegisterComponent<T> || CompositeComponent<T>);
  static constexpr std::size_t registers = [] {
    if constexpr (RegisterComponent<T>) {
      return std::size_t{1};
    } else {
      using Children = decltype(std::declval<const T &>().children());
      return []<std::size_t... I>(std::index_sequence<I...>) {
        return (std::size_t{0} + ... +
                CircuitFacts<std::remove_cvref_t<
                    std::tuple_element_t<I, Children>>>::registers);
      }(std::make_index_sequence<std::tuple_size_v<Children>>{});
    }
  }();
  static constexpr std::size_t data_inputs = registers;
  static constexpr std::size_t data_outputs = registers;
};
} // namespace loom
