#include "loom/simulation/logic.h"
struct Custom {
  const std::string name = "custom";
  bool evaluate(bool value) const { return value; }
};
int main() { loom::simulation::Definition<Custom>::create(); }
