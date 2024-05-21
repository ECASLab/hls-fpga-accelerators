/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include "matmul.h"

int main(int, char **) {
  int a_rows = 4;
  int b_cols = 8;
  int c_cols = 8;

  int size_a = a_rows * (b_cols >> kShiftData);
  int size_b = c_cols * (b_cols >> kShiftData);
  int size_c = a_rows * (c_cols >> kShiftData);

  RawDataT a[a_rows * b_cols];
  RawDataT b[c_cols * b_cols];
  RawDataT c[a_rows * c_cols];

  // Fill A
  DataT as = 0.2, bs = 0.03;
  std::cout << "A: " << std::endl;
  for (int elem = 0; elem < size_a; ++elem) {
    std::cout << as << " ";
    a[elem](15, 0) = as.V; as += DataT{0.01};
    std::cout << as << " ";
    a[elem](31, 16) = as.V; as += DataT{0.03};
    std::cout << as << " ";
    a[elem](47, 32) = as.V; as += DataT{0.07};
    std::cout << as << " ";
    a[elem](63, 48) = as.V;
    std::cout << std::endl;
  }

  std::cout << "B: " << std::endl;
  for (int elem = 0; elem < size_b; ++elem) {
    std::cout << bs << " ";
    b[elem](15, 0) = bs.V; bs += DataT{0.01};
    std::cout << bs << " ";
    b[elem](31, 16) = bs.V; bs += DataT{0.031};
    std::cout << bs << " ";
    b[elem](47, 32) = bs.V; bs += DataT{0.07};
    std::cout << bs << " ";
    b[elem](63, 48) = bs.V;
    std::cout << std::endl;
  }

  matmul(a, b, c, a_rows, b_cols, c_cols);

  std::cout << "C: " << std::endl;
  for (int elem = 0; elem < size_c; ++elem) {
    DataT cs;
    cs.V = c[elem](15, 0);
    std::cout << cs << " ";
    cs.V = c[elem](31, 16);
    std::cout << cs << " ";
    cs.V = c[elem](47, 32);
    std::cout << cs << " ";
    cs.V = c[elem](63, 48);
    std::cout << cs << std::endl;
  }
  return 0;
}
