/*
 * Copyright 2022-2023
 * Author: Fabricio Elizondo Fernandez <faelizondo@estudiantec.cr>
 * Supervisor: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#pragma once

#include <ap_fixed.h>
#include <ap_float.h>
#include "../CuFP/custom_float.h"

#include <type_traits>


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
template <typename T, int O = 1, bool is_fp = false>
class Exponential;

// Fixed point specialization

template <typename T, int O>
class Exponential<T, O, false> {
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

// Floating point specialization
template <typename T, int O>
class Exponential<T, O, true> {
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
  #pragma HLS INLINE off
    static_assert(O > 0, "Taylor for exp(x) requires order > 0");
    static const T unit = T{1.f};
    //T sum = unit;
    //T num = unit;
    //T den = unit;
    //T term = unit;

    half sum = 1.f;
    half num = 1.f;
    half den = 1.f;




    for (int i = 1; i <= O; ++i) {
#pragma HLS INLINE off
      num *= x.getHalf();
      //num *= x;
      //num *= float(x);
      //den *= T(i).reciprocal();
      den = den/i;
      sum += (num * den);


    }

    return T(sum);
  }

};  // class Exponential false



};  // namespace taylor
}  // namespace approximate
}  // namespace nonlinear
}  // namespace axc
