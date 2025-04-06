#ifndef __SOFTMAX_H__
#define __SOFTMAX_H__


#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <ap_float.h>

#include <hls_stream.h>
#include "axc-math/exponential-lut.hpp"

static constexpr int kBusWidth = 640;

static constexpr int kCols = 31;
static constexpr int kRows = 32;

using RawDataT = ap_uint<kBusWidth>;
using StreamT = hls::stream<RawDataT>;
static constexpr int START_APROX = -8;
static constexpr int END_APROX = 8;

#define USE_FIXED20

#ifdef USE_FIXED32
static constexpr int kDataWidth = 32;
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
static constexpr int kFxPDataInt = 6;
#elif defined(USE_FIXED12)
static constexpr int kDataWidth = 12;
static constexpr int kFxPDataInt = 5;
#elif defined(USE_FIXED8)
static constexpr int kDataWidth = 8;
static constexpr int kFxPDataInt = 3;
#else
static constexpr int kDataWidth = 32;
static constexpr int kFxPDataInt = 16;
#endif

using DataT = ap_fixed<kDataWidth, kFxPDataInt>;

static constexpr int kPackets = kBusWidth / kDataWidth;
static constexpr uint64_t kTotalMaxSize = kCols * kRows / kPackets;

using RawDataT = ap_uint<kBusWidth>;
using StreamT = hls::stream<RawDataT>;




//  using AccT = DataT;
 using AccT = ap_fixed<32, 16>;

 #define GET_NUMBER(n) (n)
 #define GET_RAW(n) (n).V


extern "C" {
void softmax_lut(RawDataT *in1, RawDataT *out, uint64_t size);
}

#endif // __SOFTMAX_H__
