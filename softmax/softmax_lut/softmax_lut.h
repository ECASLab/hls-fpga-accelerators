#ifndef __SOFTMAX_H__
#define __SOFTMAX_H__


#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_stream.h>
#include "axc-math/exponential-lut.hpp"


#ifndef BUS
 static constexpr int kBusWidth = 256;
 #else
 static constexpr int kBusWidth = BUS;
 #endif

// Adjustable for Element Wise
#ifndef M_COLS
static constexpr int kCols = 16;
#else
static constexpr int kCols = M_COLS;
#endif
#ifndef M_ROWS
static constexpr int kRows = 16;
#else
static constexpr int kRows = M_ROWS;
#endif

using RawDataT = ap_uint<kBusWidth>;
using StreamT = hls::stream<RawDataT>;

static constexpr int kFxPDataWidth = 32;
static constexpr int kFxPDataInt = 16;
static constexpr int kDataWidth = kFxPDataWidth;

static constexpr int START_APROX = -6;
static constexpr int END_APROX = 6;

using DataT = ap_fixed<kFxPDataWidth, kFxPDataInt>;

static constexpr int kPackets = kBusWidth / kDataWidth;

static constexpr uint64_t kTotalMaxSize = kCols * kRows / kPackets;


using AccT = DataT;




#define GET_NUMBER(n) (n)
#define GET_RAW(n) (n).V

extern "C" {
void softmax_lut(RawDataT *in1, RawDataT *out, uint64_t size);
}

#endif // __SOFTMAX_H__
