/*
 * Copyright 2022-2025
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef FP8_FP8_H_
#define FP8_FP8_H_

#include <ap_int.h>
#include <hls_math.h>
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

  /* Addition */
  minifloat operator+(const minifloat& other) const {
    return addition(*this, other);
  }

  /* Substraction */
  minifloat operator-(const minifloat& other) const {
    minifloat negative = -other;
    return minifloat{*this + negative};
  }

  /* Multiplication */
  minifloat operator*(const minifloat& other) const {
    return multiplication(*this, other);
  }

  /* Unary minus */
  minifloat operator-() const {
    minifloat res = *this;
    res.sign_ = !this->sign_;
    return res;
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
  minifloat addition(const minifloat &a, const minifloat &b) const noexcept;
  minifloat multiplication(const minifloat &a, const minifloat &b) const noexcept;
  bool iszero() const noexcept;

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

inline bool minifloat::iszero() const noexcept {
  return mantissa_ == 0 && exponent_ == 0;
}

inline minifloat minifloat::addition(const minifloat &a, const minifloat &b) const noexcept {
#pragma HLS inline off
  minifloat res, a_, b_;

  /* Return in case of zero */
  if (a.iszero()) return b;
  if (b.iszero()) return a;
  
  /* Swap flag: in case the second operand is bigger */
  ap_uint<1> swap = a.exponent_ < b.exponent_ ?  ap_uint<1>{1} : ap_uint<1>{0};
  a_ = swap ? b : a; b_ = swap ? a : b;

  /* Define the variables: swap is needed */
  ap_uint<3> m1 = a_.mantissa_;
  ap_uint<3> m2 = b_.mantissa_;
  ap_uint<4> e1 = a_.exponent_;
  ap_uint<4> e2 = b_.exponent_;
  ap_uint<1> s1 = a_.sign_;
  ap_uint<1> s2 = b_.sign_;
  ap_uint<1> sign = s1 ^ s2; // check sign

  /* Difference between exponents */
  ap_uint<4> diff_e = e1 - e2;

  /* Set the (1 + mf): 3 frac for mantissa, 16 bits for integer part */
  ap_fixed<16 + 3, 16> operand_a{1}, operand_b{1}, operand_c{0};
  operand_a(2, 0) = m1;
  operand_b(2, 0) = m2;
  operand_a <<= diff_e;

  /* Compute the number of integer bits */
  operand_c = operand_a + (sign ?  decltype(operand_b){-operand_b} : decltype(operand_b){operand_b});
  //ap_uint<4> ilog2_c = hls::ilogb(operand_c);
  ap_uint<4> ilog2_c = 15 - operand_c.countLeadingZeros(); // 15 = I - 1

  /* Get normalised mantissa */
  ap_fixed<16 + 3, 16> operand_c_norm = operand_c >> ilog2_c;

  /* Get resulting components */
  res.mantissa_ = operand_c_norm(2, 0);
  res.exponent_ = ilog2_c + b_.exponent_;

  res.sign_ = sign ? a_.sign_ : sign;

  return res;
}

inline minifloat minifloat::multiplication(const minifloat &a, const minifloat &b) const noexcept {
#pragma HLS inline off
  minifloat res;
  /* The sign is the XOR */
  res.sign_ = a.sign_ ^ b.sign_;

  /* Check any possible zero */
  if (a.iszero() || b.iszero()) return minifloat{0.f};

  /* The exponent is the sum of both: 7 due to double replication */
  res.exponent_ = a.exponent_ + b.exponent_ - 7;

  /* The mantissa depends: 1 + (m1 + m2)/8 + m1m2/64. 
     Requires two bits int and twice the resolution */
  ap_ufixed<1 + 3, 1> ma_{0}, mb_{0}, mres_{0};
  ma_(2, 0) = a.mantissa_;
  mb_(2, 0) = b.mantissa_;

  ap_ufixed<6 + 2, 2> ma_ext_{0}, mb_ext_{0}, resm_ext_{1};
  ma_ext_ = ma_;
  mb_ext_ = mb_;

  /* Can it be approximate? */
  resm_ext_ += (ma_ext_ + mb_ext_) + (ma_ext_ * mb_ext_); 
  if (resm_ext_ > ap_ufixed<6 + 2, 2>{2}) {
    resm_ext_ >>= 1;
    res.exponent_++;
  }
  mres_ = resm_ext_;
  res.mantissa_ = mres_(2, 0);

  return res;
}


#ifndef __SYNTHESIS__
inline std::ostream &operator<<(std::ostream& strm, const minifloat& obj){
  strm << static_cast<float>(obj);
  return strm;
}
#endif

void fp8_dut(const float num);

#endif // FP8_FP8_H_
