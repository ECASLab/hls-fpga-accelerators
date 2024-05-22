/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include "hls_math.h"
#include "rmsnorm.h"

static void compute(hls::stream<RawDataT> &in1_stream,
                    hls::stream<RawDataT> &out_stream, uint64_t size) {
#pragma HLS INLINE off
  AccT sum, mean, nelems, scale;
  AccT eps;
  GET_NUMBER(sum) = 0;
  GET_NUMBER(eps) = 0.01;
  GET_NUMBER(nelems) = size;

cumsum_out:
  for (uint64_t elem = 0; elem < size; elem += kPackets) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
    AccT local_cum;
    AccT local_prods[kPackets];
    GET_NUMBER(local_cum) = 0;
    RawDataT raw_in1 = in1_stream.read();

  cumprod_in:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;

      // Get the number
      AccT num;
      GET_RAW(num) = raw_in1(offhigh, offlow);

      // Accumulate
      GET_NUMBER(local_prods[p]) = GET_NUMBER(num) * GET_NUMBER(num);
    }

  cumsum_in:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Accumulate
      GET_NUMBER(local_cum) += GET_NUMBER(local_prods[p]);
    }
    GET_NUMBER(sum) += GET_NUMBER(local_cum);
  }

  // Find the squared mean
  GET_NUMBER(mean) = GET_NUMBER(sum) / GET_NUMBER(nelems);
  // Find the scale
  GET_NUMBER(scale) = 1.f / hls::sqrtf(GET_NUMBER(mean) + GET_NUMBER(eps));

scale_out:
  for (uint64_t elem = 0; elem < size; elem += kPackets) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
    RawDataT raw_in2 = in1_stream.read();
    RawDataT raw_out = 0;
  scale_in:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;
      AccT num;

      // Get the number
      GET_RAW(num) = raw_in2(offhigh, offlow);

      // Scale
      GET_NUMBER(num) *= GET_NUMBER(scale);

      // Store
      raw_out(offhigh, offlow) = GET_RAW(num);
    }
    out_stream << raw_out;
  }
}

static void load_input(RawDataT *in, hls::stream<RawDataT> &inStream,
                       uint64_t size) {
  const uint64_t size_raw = size / kPackets;
mem_reps:
  for (int i = 0; i < 2; ++i) {
  mem_rd:
    for (uint64_t i = 0; i < size_raw; ++i) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg =       \
    kTotalMaxSize
      inStream << in[i];
    }
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

/**
 * matrix: (rows, cols)
 * a: input (samples, inputs)
 * b: weights (outputs, inputs) assumed transposed
 * c: output (samples, outputs)
 */
void rmsnorm(RawDataT *in1, RawDataT *out, uint64_t size) {
#pragma HLS INTERFACE m_axi offset = slave port = in1 bundle = gmem0
#pragma HLS INTERFACE m_axi offset = slave port = out bundle = gmem1
#pragma HLS INTERFACE s_axilite register port = size
#pragma HLS INTERFACE s_axilite register port = return

  static StreamT stream_a;
  static StreamT stream_c;
#pragma HLS stream variable = stream_a depth = 32
#pragma HLS stream variable = stream_c depth = 32

#pragma HLS dataflow
  load_input(in1, stream_a, size);
  compute(stream_a, stream_c, size);
  store_result(out, stream_c, size);
}
}
