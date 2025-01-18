/*
 * Copyright 2022-2025
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef FP8_FP8_H_
#define FP8_FP8_H_

#include <ap_int.h>
#include <cstdint>

#ifndef __SYNTHESIS__
#include <iostream>
#endif

class minifloat {
 public:
  minifloat() noexcept : mantissa_{0}, exponent_{0}, sign_{0} {}
  minifloat(const ap_uint<3> m, const ap_uint<4> e, const ap_uint<1> s) noexcept : mantissa_{m}, exponent_{e}, sign_{s} {}

  /* Cast from float */
  explicit minifloat(const float val) noexcept;

  /* Cast to float */
  operator const float() const { return convertfloat(); }

#ifndef __SYNTHESIS__
  friend std::ostream &operator<<(std::ostream&, const minifloat&);
#endif

 private:
  ap_uint<3> mantissa_;
  ap_uint<4> exponent_;
  ap_uint<1> sign_;

  float convertfloat() const noexcept;

  const uint8_t kExpOffset = 120; // 127 (32 bits) - 7 (8 bits)
};

inline minifloat::minifloat(const float val) noexcept {
  union fp {
    uint32_t raw;
    float val;
  } element;

  element.val = val;
  ap_uint<32> raw = element.raw;
  ap_uint<8> exponent = static_cast<uint8_t>(raw(30, 23)) - this->kExpOffset;

  this->sign_ = raw(31, 31);
  this->exponent_ = exponent(3, 0);
  this->mantissa_ = raw(22, 20);
  

  std::cout << "e: " << (uint32_t)this->exponent_
            << " m: " << (uint32_t)this->mantissa_
            << " s: " << (uint32_t)this->sign_ << std::endl;
}

inline float minifloat::convertfloat() const noexcept {
  union fp {
    uint32_t raw;
    float val;
  } element;

  ap_uint<8> exponent = static_cast<uint8_t>(this->exponent_) + this->kExpOffset;

  ap_uint<32> raw = 0;
  raw(31, 31) = this->sign_;
  raw(30, 23) = exponent;
  raw(22, 20) = this->mantissa_;

  element.raw = raw;

  std::cout << "e: " << (uint32_t)this->exponent_
            << " m: " << (uint32_t)this->mantissa_
            << " s: " << (uint32_t)this->sign_ << std::endl;
  return element.val;
}


#ifndef __SYNTHESIS__
inline std::ostream &operator<<(std::ostream& strm, const minifloat& obj){
  strm << static_cast<float>(obj);
  return strm;
}
#endif

void fp8_dut(const float num);

#endif // FP8_FP8_H_
