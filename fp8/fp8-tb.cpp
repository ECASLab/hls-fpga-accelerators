#include <iostream>

#include "fp8.h"

int main() {
  
  for (float i = -16.f; i < 16.f; i += 0.01f) {
    minifloat val{i};
    float recast = static_cast<float>(val);
    std::cout << "Original: " << i << std::endl
              << "Minifloat: " << val << std::endl
              << "Reconverted: " << recast << std::endl << std::endl;
  }

  minifloat val{0.f};
  float recast = static_cast<float>(0.f);
  std::cout << "Original: " << 0.f << std::endl
            << "Minifloat: " << val << std::endl
            << "Reconverted: " << recast << std::endl << std::endl;

  return 0;
}