/*
 * Copyright 2022-2023
 * Author: Fabricio Elizondo Fernandez <faelizondo@estudiantec.cr>
 * Supervisor: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#pragma once

#include <ap_fixed.h>

namespace axc {
namespace nonlinear {
namespace approximate {
namespace taylor {

/**
 * Class in charge of performing Taylor-based exponential function
 *
 * By definition, it is centred at zero. So, this approximation will outperform
 * on domains from -1 to 1
 * @tparam T data type
 * @tparam O order of the Taylor expansion
 */
template <typename T, int O = 1>
class Exponential {
 public:
  /**
   * @brief Order of the Taylor expansion
   */
  static constexpr int Order = O;

  /**
   * @brief Computes the exp(x) of x
   *
   * @param x input
   * @return T output (exp(x))
   */
  T operator()(const T x) {
    /* Not allow O < 0*/
    static_assert(O > 0, "Taylor for exp(x) requires order > 0");
    using InternalT = ap_fixed<2 * T::width, T::width>;

    static const InternalT unit = InternalT{1.f};
    InternalT sum = unit;
    InternalT num = unit;
    InternalT den = unit;

#pragma HLS pipeline
    for (int i = 1; i <= O; ++i) {
      num *= x;
      den /= i;
      sum += (num * den);
    }

    return sum;
  }
};

}  // namespace taylor
}  // namespace approximate
}  // namespace nonlinear
}  // namespace axc
