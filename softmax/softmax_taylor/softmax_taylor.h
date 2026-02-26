#ifndef __SOFTMAX_H__
#define __SOFTMAX_H__

#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_stream.h>
#include <type_traits>
#include "../axc-math/exponential-taylor.hpp"
#include "../CuFP/custom_float.h"


#ifndef USE_RECIPROCAL
#define USE_RECIPROCAL 0  //is floating point?
#endif


#ifndef IS_FP
#define IS_FP 0  //is floating point?
#endif


#ifndef WS
#define WS 16  
#endif
#ifndef MS
#define MS 10
#endif

#ifndef KDATAWIDTH_FIXED
#define KDATAWIDTH_FIXED 18
#endif
#ifndef KFXPDATAINT
#define KFXPDATAINT 7
#endif

#ifndef KORDER
#define KORDER 2 // Orden de la serie de Taylor
#endif


#ifndef KBUSWIDTH
#define KBUSWIDTH 320 // Ancho del bus
#endif
#ifndef KCOLS
#define KCOLS 50
#endif
#ifndef KROWS
#define KROWS 20
#endif



#if IS_FP == 1
    static constexpr int kDataWidth = WS;
    static constexpr int kFxPDataInt = 0; 
#else
    static constexpr int kDataWidth = KDATAWIDTH_FIXED;
    static constexpr int kFxPDataInt = KFXPDATAINT;
#endif


using floating_point = CuFl::CustomFloat<WS, MS>;
using fixed = ap_fixed<kDataWidth, kFxPDataInt>;
using DataT = typename std::conditional<IS_FP, floating_point, fixed>::type;

static constexpr int kBusWidth = 10 * KDATAWIDTH_FIXED;
constexpr int korder = KORDER;

static constexpr int kCols = KCOLS;
static constexpr int kRows = KROWS;

using RawDataT = ap_uint<kBusWidth>;
using StreamT = hls::stream<RawDataT>;

static constexpr int kPackets = kBusWidth / kDataWidth;
static constexpr uint64_t kTotalMaxSize = kCols * kRows / kPackets;

//using AccT = typename std::conditional<IS_FP, CuFl::CustomFloat<16,10>, ap_fixed<32, 16>>::type;
//using AccT = typename std::conditional<IS_FP, half, ap_fixed<32, 16>>::type;


#if USE_RECIPROCAL == 1
    using AccT = typename std::conditional<IS_FP, CuFl::CustomFloat<16,10>, ap_fixed<32, 16>>::type;
#else
    using AccT = typename std::conditional<IS_FP, half, ap_fixed<32, 16>>::type;
#endif



inline half toFloat(const fixed& val) {
    
    return half(val);
}
inline half toFloat(const floating_point& val) {
    return val.getHalf();
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
