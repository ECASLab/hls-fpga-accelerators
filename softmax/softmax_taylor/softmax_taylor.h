#ifndef __SOFTMAX_H__
#define __SOFTMAX_H__

#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_stream.h>
#include <type_traits>
#include "../axc-math/exponential-taylor.hpp"
#include "../CuFP/custom_float.h"




#define USE_FIXED32

#ifdef USE_FIXED32
static constexpr int kDataWidth = 8;
static constexpr int kFxPDataInt = 16;
#elif defined(USE_FIXED64)
static constexpr int kDataWidth = 64;
static constexpr int kFxPDataInt = 32;
#elif defined(USE_FIXED24)
static constexpr int kDataWidth = 24;
static constexpr int kFxPDataInt = 8;
#elif defined(USE_FIXED20)
static constexpr int kDataWidth = 20;
static constexpr int kFxPDataInt = 10;
#elif defined(USE_FIXED16)
static constexpr int kDataWidth = 16;
static constexpr int kFxPDataInt = 5;
#elif defined(USE_FIXED12)
static constexpr int kDataWidth = 12;
static constexpr int kFxPDataInt = 6;
#elif defined(USE_FIXED8)
static constexpr int kDataWidth = 8;
static constexpr int kFxPDataInt = 3;
#else
static constexpr int kDataWidth = 32;
static constexpr int kFxPDataInt = 16;
#endif


constexpr bool is_fp = true; //is floating point?
constexpr int WS = 8;
constexpr int MS = 3;


using floating_point = CuFl::CustomFloat<WS,MS>;
using fixed = ap_fixed<kDataWidth, kFxPDataInt>;
using DataT = typename std::conditional<is_fp,floating_point,fixed>::type;

static constexpr int kBusWidth = 512;
constexpr int korder = 3;


static constexpr int kCols = 32;
static constexpr int kRows = 32;


using RawDataT = ap_uint<kBusWidth>;
using StreamT = hls::stream<RawDataT>;



static constexpr int kPackets = kBusWidth / kDataWidth;
static constexpr uint64_t kTotalMaxSize = kCols * kRows / kPackets;


using AccT = typename std::conditional<is_fp,double,ap_fixed<32, 16>>::type;



// #define GET_NUMBER(n) (n)
// #define GET_RAW(n) (n).V


inline float toFloat(const fixed& val) {
    return val.to_float();
}
inline float toFloat(const floating_point& val) {
    return val.getDouble();
}

inline ap_uint<kDataWidth> GET_RAW(const fixed& val) {
    return val.range(kDataWidth - 1, 0);
}


inline ap_uint<kDataWidth> GET_RAW(const floating_point& val) {
    return (ap_uint<1>(val.sign), ap_uint<WS-MS-1>(val.exp), ap_uint<MS>(val.mnts));
}



template <typename T>
T GET_NUMBER(const ap_uint<kDataWidth>& raw); 

template <> 
inline fixed GET_NUMBER<fixed>(const ap_uint<kDataWidth>& raw) {
    fixed result;
    result.V = raw.to_uint64();
    return result;
}

template <> 
inline floating_point GET_NUMBER<floating_point>(const ap_uint<kDataWidth>& raw) {
    floating_point result;
    result.sign  = raw[kDataWidth - 1];
    result.exp   = raw.range(kDataWidth - 2, MS);
    result.mnts  = raw.range(MS - 1, 0);
    return result;
}


extern "C" {
void softmax_taylor(RawDataT *in, RawDataT *out, uint64_t size);
}

#endif // __SOFTMAX_H__
