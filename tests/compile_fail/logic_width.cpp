#include "loom/structure/ports.h"
int main() {
  loom::structure::Output<1> source;
  loom::structure::Input<2> destination;
  loom::structure::connect(source, destination);
}
