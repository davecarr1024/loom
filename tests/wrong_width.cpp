#include "loom/circuit.h"
int main() {
  loom::Register<4> a{"a", 0};
  loom::Register<8> b{"b", 0};
  auto invalid = loom::connect(a, b);
}
