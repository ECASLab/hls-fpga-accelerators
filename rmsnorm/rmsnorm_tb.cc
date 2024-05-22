/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include "rmsnorm.h"

#define BW kDataWidth

int main(int, char **) {
  int rows = 16;
  int cols = 16;

  int size_a = rows * cols / kPackets;

  RawDataT a[size_a];
  RawDataT c[size_a];

  // Fill A
  AccT as;
  GET_NUMBER(as) = -10.f;
  AccT incr;
  GET_NUMBER(incr) = 0.2;
  std::cout << "A: " << std::endl;
  for (int elem = 0; elem < size_a; elem++) {
    for (int p = 0; p < kPackets; ++p) {
      std::cout << GET_NUMBER(as) << " ";
      a[elem]((p + 1) * BW - 1, p * BW) = GET_RAW(as);
      GET_NUMBER(as) += GET_NUMBER(incr);
    }
    std::cout << std::endl;
  }

  rmsnorm(a, c, rows * cols);

  std::cout << "C: " << std::endl;
  for (int elem = 0; elem < size_a; ++elem) {
    for (int p = 0; p < kPackets; ++p) {
      GET_RAW(as) = c[elem]((p + 1) * BW - 1, p * BW);
      std::cout << GET_NUMBER(as) << " ";
    }
    std::cout << std::endl;
  }
  return 0;
}
