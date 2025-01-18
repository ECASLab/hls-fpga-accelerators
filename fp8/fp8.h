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

  /* Copy */
  void operator=(const minifloat &m) {
    this->mantissa_ = m.mantissa_;
    this->exponent_ = m.exponent_;
    this->sign_ = m.sign_;
  }

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
  ap_uint<23> mantissa = raw(22, 0);
  ap_uint<8> exponent = static_cast<uint8_t>(raw(30, 23));

  /* Handle corner cases: inf */
  if (mantissa == 0x00 && exponent == 0xFF) {
    this->exponent_ = 0xF;
  } else if (exponent > 134) {
    exponent = 134;
    mantissa = -1;
  } else if (exponent < 121) {
    exponent = 120;
    mantissa = 0x000;
  }

  exponent -= this->kExpOffset;

  this->sign_ = raw(31, 31);
  this->exponent_ = exponent(3, 0);
  this->mantissa_ = mantissa(22, 20);
}

inline float minifloat::convertfloat() const noexcept {
  union fp {
    uint32_t raw;
    float val;
  } element;

  ap_uint<8> exponent = this->exponent_ == ap_uint<4>{0xF} ?  
      ap_uint<8>{0xFF} : this->exponent_ == ap_uint<4>{0x0} ?
      ap_uint<8>{0x0} : ap_uint<8>{static_cast<uint8_t>(this->exponent_) + this->kExpOffset};

  ap_uint<32> raw = 0;
  raw(31, 31) = this->sign_;
  raw(30, 23) = exponent;
  raw(22, 20) = this->mantissa_;

  element.raw = raw;
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
