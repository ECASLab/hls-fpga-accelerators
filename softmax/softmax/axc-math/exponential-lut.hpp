/*
 * Copyright 2022-2023
 * Author: Fabricio Elizondo Fernandez <faelizondo@estudiantec.cr>
 * Supervisor: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#pragma once

#include <cmath>
#include <ratio>

#include "interpolation-wrapper.hpp"

namespace axc {
namespace nonlinear {
namespace approximate {
namespace lut {
namespace generator {

/**
 * Class in charge of initialize the LUT with the exp values
 * @tparam T type of the LUT array
 * @tparam BEGIN begin of the function domain. Based on std::ratio
 * @tparam END end of the function domain. Based on std::ratio
 * @tparam S size of the LUT array. It defaults to 64 elements from BEGIN to
 * END
 */
template <typename T, class BEGIN, class END, int S = 64>
class Exponential {
 public:
  /**
   * @brief Number of points of the look-up-table to initialise
   */
  static constexpr int Points = S;

  /**
   * @brief Beginning of the look-up-table to initialise
   */
  static constexpr float Minimum =
      static_cast<float>(BEGIN::num) / static_cast<float>(BEGIN::den);

  /**
   * @brief Ending of the look-up-table to initialise
   */
  static constexpr float Maximum =
      static_cast<float>(END::num) / static_cast<float>(END::den);

  /**
   * @brief Define the step
   */
  static constexpr float Step =
      (Maximum - Minimum) / static_cast<float>(Points);

  /**
   * @brief Construct a new Exponential LUT Generator object
   *
   * It also fills the LUT in the lut parameter with a domain-specific
   * exp(x) function
   *
   * @param lut LUT array to fill
   */
  explicit Exponential(T lut[S]);
};

template <typename T, class BEGIN, class END, int S>
Exponential<T, BEGIN, END, S>::Exponential(T lut[S]) {
  for (int i = 0; i < Points; ++i) {
    float x = Minimum + i * Step;
    lut[i] = T{std::exp(x)};
  }
}

}  // namespace generator

/**
 * Class in charge of performing LUT-based exponential function with
 * domain-specific characteristics
 * @tparam T type of the LUT array
 * @tparam BEGIN begin of the function domain. Defaults to -1
 * @tparam END end of the function domain. Defaults to 1
 * @tparam S size of the LUT array. It defaults to 64 elements from BEGIN to
 * END
 */
template <typename T, class BEGIN, class END, int S = 64>
class Exponential {
 public:
  /**
   * @brief Number of points of the look-up-table to initialise
   */
  static constexpr int Points = S;

  /**
   * @brief Beginning of the look-up-table to initialise
   */
  static constexpr float Minimum =
      static_cast<float>(BEGIN::num) / static_cast<float>(BEGIN::den);

  /**
   * @brief Ending of the look-up-table to initialise
   */
  static constexpr float Maximum =
      static_cast<float>(END::num) / static_cast<float>(END::den);

  /**
   * @brief Define the step
   */
  static constexpr float Step =
      (Maximum - Minimum) / static_cast<float>(Points);

  /**
   * @brief Computes the exp(x) of x
   *
   * @param x input
   * @return T output (exp(x))
   */
  T operator()(const T x) { return interpolator_(x); }

  /**
   * @brief LUT factory
   */
  typedef generator::Exponential<T, BEGIN, END, S> Generator;

 private:
  ::axc::nonlinear::approximate::helpers::LinearInterpolation<T, Generator>
      interpolator_;
};

}  // namespace lut
}  // namespace approximate
}  // namespace nonlinear
}  // namespace axc
