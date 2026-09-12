#pragma once

#include <concepts>

namespace loom::value {
// A bit accepts bool explicitly; numeric inputs cannot silently truncate.
class Bit {
public:
  template <std::same_as<bool> T>
  constexpr explicit Bit(T high) : high_(high) {}
  constexpr bool high() const { return high_; }
  bool operator==(const Bit &) const = default;

private:
  bool high_;
};
} // namespace loom::value
