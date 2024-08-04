/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include <cmath>
#include <type_traits>

#include "matvecmul.h"

void copy(RawDataT *a_buf, float *a, const int a_raw_elems) {
  for (int elems = 0; elems < a_raw_elems; ++elems) {
    for (int p = 0; p < kPackets; ++p) {
      AccT num;
      GET_NUMBER(num) = a[p + kPackets * elems];
      const int low = p * kDataWidth;
      const int high = low + kDataWidth - 1;
      a_buf[elems](high, low) = GET_RAW(num);
    }
  }
}

void copy_back(float *a, RawDataT *a_buf, const int a_raw_elems) {
  for (int elems = 0; elems < a_raw_elems; ++elems) {
    for (int p = 0; p < kPackets; ++p) {
      AccT num;
      const int low = p * kDataWidth;
      const int high = low + kDataWidth - 1;
      GET_RAW(num) = a_buf[elems](high, low);
      a[p + kPackets * elems] = GET_NUMBER(num);
    }
  }
}

int main(int, char **) {
  static_assert(std::is_same<float, DataT>::value,
                "Testbench is only compatible with Float32");
  const int a_rows = kARows;
  const int b_cols = kBCols;
  const int c_cols = 1;

  float a[a_rows * b_cols];
  float b[b_cols * c_cols];
  float c_hw[a_rows * c_cols];
  float c_sw[a_rows * c_cols];

  // Fill A
  float as = 0.3f;
  std::cout << "A: " << std::endl;
  for (int row = 0; row < a_rows; ++row) {
    for (int col = 0; col < b_cols; ++col) {
      a[col + row * b_cols] = as;
      std::cout << as << ", ";
      as += 0.01;
    }
    std::cout << std::endl;
  }

  // Fill B
  float bs = 1.0f;
  std::cout << "B: " << std::endl;
  for (int col = 0; col < b_cols; ++col) {
    b[col] = bs;
    std::cout << bs << ", ";
    bs += 0.05;
  }
  std::cout << std::endl;

  const int a_raw_elems = a_rows * b_cols / kPackets;
  const int b_raw_elems = c_cols * b_cols / kPackets;
  const int c_raw_elems = a_rows * c_cols / kPackets;

  RawDataT a_raw[a_raw_elems], b_raw[b_raw_elems], c_raw[c_raw_elems];

  // Copy A and B
  copy(a_raw, a, a_raw_elems);
  copy(b_raw, b, b_raw_elems);

  matvecmul(a_raw, b_raw, c_raw, a_rows, b_cols, c_cols);

  // Copy C
  copy_back(c_hw, c_raw, c_raw_elems);

  // Multiply by software
  for (int row = 0; row < a_rows; ++row) {
    c_sw[row] = 0.f;
    for (int k = 0; k < b_cols; ++k) {
      c_sw[row] += a[row * b_cols + k] * b[k];
    }
  }

  // Compare results
  std::cout << "C_SW - C_HW: " << std::endl;
  for (int row = 0; row < a_rows; ++row) {
    std::cout << c_sw[row] << " - " << c_hw[row] << std::endl;
    if (fabs(c_sw[row] - c_hw[row]) > 1e-5) {
      std::cerr << "Error in index: " << row
                << "Val: " << (fabs(c_sw[row] - c_hw[row])) << std::endl;
      return -1;
    }
  }
  return 0;
}
