#pragma once

#include "loom/components/and.h"
#include "loom/components/enabled_word_register.h"
#include "loom/components/not.h"
#include "loom/components/selected_bus.h"
#include "loom/value/bit.h"
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace loom::components {

enum class RegisterAddress : bool { zero = false, one = true };
enum class RegisterWriteSource : bool {
  external_input = false,
  selected_register = true
};

// A two-word, one-read-port, one-write-port bank. A write copies either the
// external data word or the currently selected read-bus word into one selected
// register. All storage and selection are owned child circuits.
template <std::size_t Width> class RegisterBank2 final {
  static_assert(Width > 0, "register bank width must be positive");

public:
  static constexpr std::size_t width = Width;
  const std::string name;

  RegisterBank2(std::string label,
                std::array<std::array<value::Bit, Width>, 2> initial)
      : name(std::move(label)), registers_(make_registers(initial)),
        read_bus_{"read_bus"}, write_bus_{"write_bus"},
        write_enable_{"write_enable"}, write_address_{"write_address"},
        write_address_not_{"write_address_not"},
        write_zero_enable_{"write_zero_enable"},
        write_one_enable_{"write_one_enable"},
        external_write_data_(
            make_write_data(std::make_index_sequence<Width>{})) {}

  const auto &write_data_ports() const { return external_write_data_; }
  const structure::ExternalInput &write_enable_port() const {
    return write_enable_;
  }
  auto read_address(RegisterAddress address) const {
    return read_bus_.select_port().bind(
        value::Bit{address == RegisterAddress::one});
  }
  auto write_address(RegisterAddress address) const {
    return write_address_.bind(value::Bit{address == RegisterAddress::one});
  }
  auto write_source(RegisterWriteSource source) const {
    return write_bus_.select_port().bind(
        value::Bit{source == RegisterWriteSource::selected_register});
  }
  auto read_output() const { return read_bus_.output(); }

  auto children() const {
    return std::tuple_cat(
        std::tie(registers_[0], registers_[1], read_bus_, write_bus_,
                 write_enable_, write_address_, write_address_not_,
                 write_zero_enable_, write_one_enable_),
        write_data_children(std::make_index_sequence<Width>{}));
  }

  auto connections() const {
    return std::tuple_cat(std::tuple{
        structure::connect(registers_[0].output(),
                           bus_source_zero_input(
                               read_bus_, std::make_index_sequence<Width>{})),
        structure::connect(
            registers_[1].output(),
            bus_source_one_input(read_bus_, std::make_index_sequence<Width>{})),
        structure::connect(read_bus_.output(),
                           bus_source_one_input(
                               write_bus_, std::make_index_sequence<Width>{})),
        structure::connect(write_data_output(std::make_index_sequence<Width>{}),
                           bus_source_zero_input(
                               write_bus_, std::make_index_sequence<Width>{})),
        structure::connect(write_bus_.output(), registers_[0].data_input()),
        structure::connect(write_bus_.output(), registers_[1].data_input()),
        structure::connect(write_enable_.output(), write_zero_enable_.left()),
        structure::connect(write_enable_.output(), write_one_enable_.left()),
        structure::connect(write_address_.output(), write_address_not_.input()),
        structure::connect(write_address_not_.output(),
                           write_zero_enable_.right()),
        structure::connect(write_address_.output(), write_one_enable_.right()),
        structure::connect(write_zero_enable_.output(),
                           registers_[0].enable_input()),
        structure::connect(write_one_enable_.output(),
                           registers_[1].enable_input())});
  }

private:
  const std::array<EnabledWordRegister<Width>, 2> registers_;
  const SelectedBus<Width> read_bus_;
  const SelectedBus<Width> write_bus_;
  const structure::ExternalInput write_enable_;
  const structure::ExternalInput write_address_;
  const Not write_address_not_;
  const And write_zero_enable_;
  const And write_one_enable_;
  const std::array<structure::ExternalInput, Width> external_write_data_;

  static auto
  make_registers(const std::array<std::array<value::Bit, Width>, 2> &initial) {
    return std::array{EnabledWordRegister<Width>{"register_zero", initial[0]},
                      EnabledWordRegister<Width>{"register_one", initial[1]}};
  }

  template <std::size_t... Index>
  static auto make_write_data(std::index_sequence<Index...>) {
    return std::array<structure::ExternalInput, Width>{
        structure::ExternalInput{"write_data_" + std::to_string(Index)}...};
  }

  template <std::size_t... Index>
  auto write_data_children(std::index_sequence<Index...>) const {
    return std::tie(external_write_data_[Index]...);
  }

  template <std::size_t... Index>
  auto write_data_output(std::index_sequence<Index...>) const {
    return structure::wire_bundle(external_write_data_[Index].output()...);
  }

  template <std::size_t... Index>
  auto bus_source_zero_input(const SelectedBus<Width> &bus,
                             std::index_sequence<Index...>) const {
    return structure::wire_bundle(bus.source_zero_ports()[Index].input()...);
  }

  template <std::size_t... Index>
  auto bus_source_one_input(const SelectedBus<Width> &bus,
                            std::index_sequence<Index...>) const {
    return structure::wire_bundle(bus.source_one_ports()[Index].input()...);
  }
};

} // namespace loom::components
