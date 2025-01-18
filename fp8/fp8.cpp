/*
 * Copyright 2022-2025
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include <iostream>

#include "fp8.h"

void fp8_dut(const float inval, float &outval) {
  minifloat val{inval};
  val = val * minifloat{2.f} + minifloat{16.f};
  outval = static_cast<float>(val);
}
