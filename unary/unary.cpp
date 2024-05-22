/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include "axc-math/exponential-lut.hpp"
#include "hls_math.h"
#include "unary.h"

enum {
  /** None - pass-thru*/
  OP_NONE = 0,
  /** RELU */
  OP_RELU = 1,
  /** RELU */
  OP_SILU = 2
};

static void load_input(RawDataT *in, hls::stream<RawDataT> &inStream,
                       uint64_t size) {
  const uint64_t size_raw = size / kPackets;
mem_rd:
  for (uint64_t i = 0; i < size_raw; ++i) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
    inStream << in[i];
  }
}

static void compute(hls::stream<RawDataT> &in1_stream,
                    hls::stream<RawDataT> &out_stream, uint64_t size, int op) {
  // This kernel operates in vector mode. Even though, contiguous matrices can
  // be processed. This uses the linear interpolation on the exponential
  constexpr int kNumPoints = 32;
  using Start = std::ratio<-6>;
  using End = std::ratio<6>;
  using ExpDataType = ap_fixed<24, 12>;

#ifdef USE_LUT
  using namespace axc::nonlinear::approximate::lut;
  using ExpOpLUT = Exponential<ExpDataType, Start, End, kNumPoints>;
  static ExpOpLUT explut{};
#endif

execute:
  for (uint64_t i = 0; i < size; i += kPackets) {
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
#pragma HLS PIPELINE
    RawDataT raw_in1 = in1_stream.read();
    RawDataT raw_out = 0;
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;

      // Data
      AccT in1, out;
#ifdef USE_UNION
      in1.i = raw_in1(offhigh, offlow);
#else
      in1.V = raw_in1(offhigh, offlow);
#endif
      // Operation
      switch (op) {
      case OP_RELU:
#ifdef USE_UNION
        out.f = in1.f >= 0.f ? in1.f : 0.f;
#else
        out = in1 >= AccT{0} ? in1 : AccT{0};
#endif
        break;
      case OP_SILU: {
#ifdef USE_UNION
#ifdef USE_LUT
        ExpDataType infx{in1.f};
        ExpDataType expx = explut(infx);
        ExpDataType denx = ExpDataType{1} + expx;
        ExpDataType invdenx = ExpDataType{1} / denx;
        out.f = in1.f >= End::num ? decltype(in1.f)(infx)
                                  : decltype(in1.f)(infx * expx * invdenx);
#else
        decltype(in1.f) expx = hls::exp(in1.f);
        decltype(in1.f) denx = 1.f + expx;
        decltype(in1.f) invdenx = 1.f / denx;
        out.f = in1.f * expx * invdenx;
#endif
#else
#ifdef USE_LUT
        ExpDataType infx{in1};
        ExpDataType expx = explut(infx);
        ExpDataType denx = ExpDataType{1} + expx;
        ExpDataType invdenx = ExpDataType{1} / denx;
        out = in1 >= End::num ? in1 : AccT{infx * expx * invdenx};
#else
        AccT expx = hls::exp(in1);
        AccT denx = AccT{1} + expx;
        AccT invdenx = AccT{1} / denx;
        out = in1 * expx * invdenx;
#endif
#endif
      } break;
      default:
        out = in1;
        break;
      }

      // Write back
#ifdef USE_UNION
      raw_out(offhigh, offlow) = out.i;
#else
      raw_out(offhigh, offlow) = out.V;
#endif
    }
    out_stream << raw_out;
  }
}

static void store_result(RawDataT *out, hls::stream<RawDataT> &out_stream,
                         uint64_t size) {
  const uint64_t size_raw = size / kPackets;
mem_wr:
  for (uint64_t i = 0; i < size_raw; ++i) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
    out[i] = out_stream.read();
  }
}

extern "C" {

/*
    Map Kernel

    Arguments:
        in1  (input)  --> Input vector 1
        out  (output) --> Output vector
        size (input)  --> Number of elements in vector
        op (input)    --> Operation: 0: none, 1: relu, 2: silu
*/

void unary(RawDataT *in, RawDataT *out, uint64_t size, int op) {
#pragma HLS INTERFACE m_axi port = in bundle = gmem0
#pragma HLS INTERFACE m_axi port = out bundle = gmem1

  static StreamT in_stream("in_stream");
  static StreamT out_stream("out_stream");
#pragma HLS stream variable = in_stream depth = 32
#pragma HLS stream variable = out_stream depth = 32

#pragma HLS dataflow
  // dataflow pragma instruct compiler to run following three APIs in parallel
  load_input(in, in_stream, size);
  compute(in_stream, out_stream, size, op);
  store_result(out, out_stream, size);
}
}
