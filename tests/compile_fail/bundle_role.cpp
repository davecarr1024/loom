#include "loom/structure/wire_bundle.h"

int main() {
  loom::structure::Input<1> first;
  loom::structure::Output<1> second;
  const auto input = loom::structure::wire_bundle(first);
  const auto output = loom::structure::wire_bundle(second);
  loom::structure::connect(input, output);
}
