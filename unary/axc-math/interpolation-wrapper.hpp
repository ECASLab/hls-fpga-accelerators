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
namespace helpers {

/**
 * Linear Interpolation class
 * It uses a Look-Up Table and performs a newton linear interpolation
 * @tparam T datatype to work with
 * @tparam S size of the LUT array
 * @tparam BEGIN begin of the function domain
 * @tparam END end of the function domain
 * @tparam LUT It represents the Look-up table that will be used for the
 * interpolation process.
 */
template <typename T, class LUT>
class LinearInterpolation {
 public:
  typedef LUT Generator;

  /**
   * @brief Construct a new Linear Interpolation object
   *
   * It constructs the LUT corresponding to the LUT parameter. It is agnostic
   * to the operator and depends on the values of the LUT. For better
   * efficiency, the function should be smooth.
   */
  LinearInterpolation() {}

  /**
   * @brief Operator to compute the image of the given x (pre-image)
   *
   * @param x domain point
   * @return correspoding function value
   */
  T operator()(const T x);
};

template <typename T, class LUT>
inline T LinearInterpolation<T, LUT>::operator()(const T x) {
  /* Compute the step in compile time */
  using InternalT = ap_fixed<2 * T::width, T::width>;
  constexpr float kInvStepF = 1.f / LUT::Step;
  constexpr int kPoints = LUT::Points;
  static const InternalT step = InternalT{LUT::Step};
  static const InternalT inv_step = InternalT{kInvStepF};
  static const InternalT begin = InternalT{LUT::Minimum};
  static const InternalT end = InternalT{LUT::Maximum};
  static const InternalT unit = InternalT{1.f};

  T lut[LUT::Points];
  LUT generator{lut};

  int16_t i_upper{0}, i_lower{0};
  T res{0};

  /* Get the index values from x -> the result leads to a proper ap_fixed and it
   * is casted to integer */
  i_lower = (InternalT{x} - begin) * inv_step;

  /* Check bounds */
  i_lower = i_lower <= 0 ? 0 : i_lower;
  i_lower = i_lower >= (kPoints - 1) ? kPoints - 2 : i_lower;

  /* Get the next point */
  i_upper = i_lower + 1;

  InternalT x_lower = (i_lower * step);
  x_lower += begin;
  const InternalT y_lower = InternalT{lut[i_lower]};
  const InternalT y_upper = InternalT{lut[i_upper]};
  const InternalT xa = x;

  /* Determine the deltas */
  const InternalT delta_y = y_upper - y_lower;

  res = (xa - x_lower);
  res *= delta_y;
  res *= inv_step;
  res += y_lower;
  return res;
}

}  // namespace helpers
}  // namespace approximate
}  // namespace nonlinear
}  // namespace axc
