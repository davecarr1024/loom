#pragma once

#include "loom/components/and.h"
#include "loom/components/not.h"
#include "loom/structure/wire_bundle.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

// A total two-bit to four-line one-hot decoder. Address bit 0 is least
// significant, so output index equals the unsigned address value.
class Decoder2To4 final {
public:
  using Connections =
      std::tuple<structure::Connection<1>, structure::Connection<1>,
                 structure::Connection<1>, structure::Connection<1>,
                 structure::Connection<1>, structure::Connection<1>,
                 structure::Connection<1>, structure::Connection<1>,
                 structure::Connection<1>, structure::Connection<1>>;
  const std::string name;
  explicit Decoder2To4(std::string label) : name(std::move(label)) {}

  const auto &address_ports() const { return address_; }
  auto address() const {
    return structure::wire_bundle(address_[0].output(), address_[1].output());
  }
  auto output() const {
    return structure::wire_bundle(decode_0_.output(), decode_1_.output(),
                                  decode_2_.output(), decode_3_.output());
  }

  auto children() const {
    return std::tie(address_[0], address_[1], not_low_, not_high_, decode_0_,
                    decode_1_, decode_2_, decode_3_);
  }

  Connections connections() const {
    return connections_impl(std::make_index_sequence<4>{});
  }

private:
  const std::array<structure::ExternalInput, 2> address_{
      structure::ExternalInput{"address_0"},
      structure::ExternalInput{"address_1"}};
  const Not not_low_{"not_address_0"};
  const Not not_high_{"not_address_1"};
  const And decode_0_{"decode_0"};
  const And decode_1_{"decode_1"};
  const And decode_2_{"decode_2"};
  const And decode_3_{"decode_3"};

  template <std::size_t Index> const And &decode() const {
    if constexpr (Index == 0)
      return decode_0_;
    else if constexpr (Index == 1)
      return decode_1_;
    else if constexpr (Index == 2)
      return decode_2_;
    else
      return decode_3_;
  }

  template <std::size_t Index> auto connect_low_bit() const {
    if constexpr ((Index & 1U) != 0)
      return structure::connect(address_[0].output(), decode<Index>().left());
    else
      return structure::connect(not_low_.output(), decode<Index>().left());
  }

  template <std::size_t Index> auto connect_high_bit() const {
    if constexpr ((Index & 2U) != 0)
      return structure::connect(address_[1].output(), decode<Index>().right());
    else
      return structure::connect(not_high_.output(), decode<Index>().right());
  }

  template <std::size_t Index> auto row_connections() const {
    return std::tuple{connect_low_bit<Index>(), connect_high_bit<Index>()};
  }

  template <std::size_t... Index>
  Connections connections_impl(std::index_sequence<Index...>) const {
    return std::tuple_cat(
        std::tuple{structure::connect(address_[0].output(), not_low_.input()),
                   structure::connect(address_[1].output(), not_high_.input())},
        std::tuple_cat(row_connections<Index>()...));
  }
};

} // namespace loom::components
