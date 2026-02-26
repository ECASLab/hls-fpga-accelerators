/*
 * Copyright 2022-2023
 * Author: Fabricio Elizondo Fernandez <faelizondo@estudiantec.cr>
 * Supervisor: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#pragma once

#include <ap_fixed.h>
#include <ap_float.h>

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

 template <typename T, class LUT, bool is_fp>
 class LinearInterpolation;

 template <typename T, class LUT>
 class LinearInterpolation<T, LUT, true> {
  public:
   T operator()(const T x) {
     const half kStep = LUT::Step;
     const half kInvStep = half(1.0f) / kStep;
     const half kBegin = LUT::Minimum;
     constexpr int kPoints = LUT::Points;

 
     T lut[LUT::Points];
     LUT generator{lut};
 
     int16_t i_lower = static_cast<int16_t>((x.getHalf() - kBegin) * kInvStep);
 
     /* Check bounds */
     i_lower = i_lower <= 0 ? 0 : i_lower;
     i_lower = i_lower >= (kPoints - 1) ? kPoints - 2 : i_lower;
 
     int16_t i_upper = i_lower + 1;
 
     // La fórmula de interpolación lineal estándar usando tipos flotantes
     const half x_lower = kBegin + (i_lower * kStep);
     const half y_lower = lut[i_lower].getHalf();
     const half y_upper = lut[i_upper].getHalf();
 
     
     half res = y_lower + (x.getHalf() - x_lower) * (y_upper - y_lower) * kInvStep;
 
     return T(res);
   }
 };



template <typename T, class LUT>
class LinearInterpolation<T, LUT, false> {
 public:
  T operator()(const T x) {
    using InternalT = ap_fixed<2 * T::width, T::width>;

    constexpr float kInvStepF = 1.0f / LUT::Step;
    constexpr int kPoints = LUT::Points;
    static const InternalT step = InternalT(LUT::Step);
    static const InternalT inv_step = InternalT(kInvStepF);
    static const InternalT begin = InternalT(LUT::Minimum);

    T lut[LUT::Points];
    LUT generator{lut};

    int16_t i_lower = (InternalT(x) - begin) * inv_step;

    /* Check bounds */
    i_lower = i_lower <= 0 ? 0 : i_lower;
    i_lower = i_lower >= (kPoints - 1) ? kPoints - 2 : i_lower;

    int16_t i_upper = i_lower + 1;

    
    InternalT x_lower = (i_lower * step) + begin;
    const InternalT y_lower = InternalT(lut[i_lower]);
    const InternalT y_upper = InternalT(lut[i_upper]);
    const InternalT xa = x;
    const InternalT delta_y = y_upper - y_lower;
    
    InternalT res_internal = (xa - x_lower) * delta_y * inv_step + y_lower;
    
    return res_internal; 
};
};






}  // namespace helpers
}  // namespace approximate
}  // namespace nonlinear
}  // namespace axc
