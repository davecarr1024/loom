#pragma once

#include "loom/value/bit.h"
#include <string>
#include <utility>

namespace loom::structure {
// Ports are owned objects, not payload-type identities. They cannot relocate
// independently of a definition. Direction and width constrain connections.
template <unsigned Width> struct Input {
  static_assert(Width > 0);
  Input() = default;
  Input(const Input &) = delete;
  Input &operator=(const Input &) = delete;
};
template <unsigned Width> struct Output {
  static_assert(Width > 0);
  Output() = default;
  Output(const Output &) = delete;
  Output &operator=(const Output &) = delete;
};
template <unsigned Width> struct Connection {
  const Output<Width> *source;
  const Input<Width> *destination;
};
template <unsigned Width>
auto connect(const Output<Width> &source, const Input<Width> &destination) {
  return Connection<Width>{&source, &destination};
}

class ExternalInput;
// Only an external input can produce a binding. Finalization and observation
// additionally check that this port belongs to the requested definition.
class Binding {
public:
  const Output<1> *port() const { return port_; }
  value::Bit value() const { return value_; }

private:
  friend class ExternalInput;
  Binding(const Output<1> *port, value::Bit value)
      : port_(port), value_(value) {}
  const Output<1> *port_;
  value::Bit value_;
};
class ExternalInput final {
public:
  const std::string name;
  explicit ExternalInput(std::string label) : name(std::move(label)) {}
  const Input<1> &input() const { return input_; }
  const Output<1> &output() const { return output_; }
  Binding bind(value::Bit value) const { return Binding(&output_, value); }

private:
  const Input<1> input_;
  const Output<1> output_;
};
class ExternalOutput final {
public:
  const std::string name;
  explicit ExternalOutput(std::string label) : name(std::move(label)) {}
  const Input<1> &input() const { return input_; }

private:
  const Input<1> input_;
};
} // namespace loom::structure
