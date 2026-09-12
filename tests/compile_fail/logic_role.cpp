#include "loom/structure/ports.h"
int main() {
  loom::structure::Input<1> input;
  loom::structure::Output<1> output;
  loom::structure::connect(input, output);
}
