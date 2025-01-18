#include <iostream>

#include "fp8.h"

int main() {
  /*
  for (float i = -512.f; i < 512.f; i += 0.01f) {
    minifloat val{i};
    float recast = static_cast<float>(val);
    std::cout << "Original: " << i << std::endl
              << "Minifloat: " << val << std::endl
              << "Reconverted: " << recast << std::endl << std::endl;
  }
  */

  minifloat val{0.f};
  float recast = static_cast<float>(val);
  std::cout << "Original: " << 0.f << std::endl
            << "Minifloat: " << val << std::endl
            << "Reconverted: " << recast << std::endl << std::endl;

  val = minifloat{6555.f};
  recast = static_cast<float>(val);
  std::cout << "Original: " << 6555.f << std::endl
            << "Minifloat: " << val << std::endl
            << "Reconverted: " << recast << std::endl << std::endl;

  val = minifloat{-6555.f};
  recast = static_cast<float>(val);
  std::cout << "Original: " << -6555.f << std::endl
            << "Minifloat: " << val << std::endl
            << "Reconverted: " << recast << std::endl << std::endl;

  val = minifloat{0.5f} + minifloat{0.1875f};
  std::cout << "Original: " << minifloat{0.6875} << std::endl
            << "Minifloat: " << val << std::endl << std::endl;

  val = minifloat{0.1875f} + minifloat{0.5f};
  std::cout << "Original: " << minifloat{0.6875} << std::endl
            << "Minifloat: " << val << std::endl << std::endl;

  val = minifloat{0.1875f} + minifloat{0.f};
  std::cout << "Original: " << minifloat{0.1875f} << std::endl
            << "Minifloat: " << val << std::endl << std::endl;

  val = minifloat{0.5f} + minifloat{-0.1875f};
  std::cout << "Original: " << minifloat{0.3125} << std::endl
            << "Minifloat: " << val << std::endl << std::endl;

  val = minifloat{0.1875f} + minifloat{-0.5f};
  std::cout << "Original: " << minifloat{-0.3125} << std::endl
            << "Minifloat: " << val << std::endl << std::endl;

  val = minifloat{-0.1875f} + minifloat{0.f};
  std::cout << "Original: " << minifloat{-0.1875f} << std::endl
            << "Minifloat: " << val << std::endl << std::endl;
  return 0;
}