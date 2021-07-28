#include "mesh.hpp"

#include <fstream>

int main() {
  std::ifstream stream{"../dragon.obj"};
  auto m = parse_obj(stream);
  navigatable_mesh m2{m};
}