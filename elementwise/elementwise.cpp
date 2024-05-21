/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include "elementwise.h"

enum { OP_ADD = 0, OP_MULT = 1 };

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
                    hls::stream<RawDataT> &in2_stream,
                    hls::stream<RawDataT> &out_stream, uint64_t size, int op) {
// The kernel is operating with vector of NUM_WORDS integers. The + operator
// performs an element-wise add, resulting in NUM_WORDS parallel additions.
execute:
  for (uint64_t i = 0; i < size; i += kPackets) {
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
#pragma HLS PIPELINE
    RawDataT raw_in1 = in1_stream.read();
    RawDataT raw_in2 = in2_stream.read();
    RawDataT raw_out = 0;
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;

      // Data
      AccT in1, in2, out;
#ifdef USE_UNION
      in1.i = raw_in1(offhigh, offlow);
      in2.i = raw_in2(offhigh, offlow);
#else
      in1.V = raw_in1(offhigh, offlow);
      in2.V = raw_in2(offhigh, offlow);
#endif
      // Operation
      switch (op) {
      case OP_ADD:
#ifdef USE_UNION
        out.f = in1.f + in2.f;
#else
        out = in1 + in2;
#endif
        break;
      case OP_MULT:
#ifdef USE_UNION
        out.f = in1.f * in2.f;
#else
        out = in1 * in2;
#endif
        break;
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
    Vector Addition Kernel

    Arguments:
        in1  (input)  --> Input vector 1
        in2  (input)  --> Input vector 2
        out  (output) --> Output vector
        size (input)  --> Number of elements in vector
        op (input)    --> Operation: 0: add, 1: add+relu 2: mult
*/

void elementwise(RawDataT *in1, RawDataT *in2, RawDataT *out, uint64_t size,
                 int op) {
#pragma HLS INTERFACE m_axi port = in1 bundle = gmem0
#pragma HLS INTERFACE m_axi port = in2 bundle = gmem1
#pragma HLS INTERFACE m_axi port = out bundle = gmem2

  static StreamT in1_stream("in1_stream");
  static StreamT in2_stream("in2_stream");
  static StreamT out_stream("out_stream");
#pragma HLS stream variable = in1_stream depth = 32
#pragma HLS stream variable = in2_stream depth = 32
#pragma HLS stream variable = out_stream depth = 32

#pragma HLS dataflow
  // dataflow pragma instruct compiler to run following three APIs in parallel
  load_input(in1, in1_stream, size);
  load_input(in2, in2_stream, size);
  compute(in1_stream, in2_stream, out_stream, size, op);
  store_result(out, out_stream, size);
}
}
