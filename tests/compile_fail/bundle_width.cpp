#include "loom/structure/wire_bundle.h"

int main() {
  loom::structure::Output<1> first, second;
  loom::structure::Input<1> destination;
  const auto wide = loom::structure::wire_bundle(first, second);
  const auto narrow = loom::structure::wire_bundle(destination);
  loom::structure::connect(wide, narrow);
}
