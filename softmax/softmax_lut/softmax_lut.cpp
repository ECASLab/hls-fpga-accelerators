#include "softmax_lut.h"

static void load_input(RawDataT *in, hls::stream<RawDataT> &inStream,
                       uint64_t size) {
  const uint64_t size_raw = size / kPackets;
mem_reps:
  for (int i = 0; i < 2; ++i) {
  mem_rd:
    for (uint64_t i = 0; i < size_raw; ++i) {
#pragma HLS PIPELINE
//#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
      inStream << in[i];
    }
  }
}

static void compute(hls::stream<RawDataT> &in_stream,
                          hls::stream<RawDataT> &out_stream, uint64_t size) {
#pragma HLS INLINE off
  constexpr int kNumPoints = 64;

  using Start = std::ratio<START_APROX>;
  using End = std::ratio<END_APROX>;
  using ExpOpLut =
      axc::nonlinear::approximate::lut::Exponential<DataT, Start, End, kNumPoints>;
      ExpOpLut explut{};

  AccT sum = {0};
  AccT scale = {0.0f};

// Cumsum
cumsum_out:
  for (int i = 0; i < size; i += kPackets) {
#pragma HLS PIPELINE
//#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    AccT local_cum = {0};
    AccT local_exps[kPackets] = {0};
    RawDataT raw_in1 = in_stream.read();

  compute_exps:
    for (int p = 0; p < kPackets; ++p) {
      // offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;

      // Extract the input
      AccT num = {0};
      GET_RAW(num) = raw_in1(offhigh, offlow);
      GET_NUMBER(local_exps[p]) = explut(GET_NUMBER(num));
    }  // compute_Exps

  // compute cumsum
  cumsum_in:
    for (int p = 0; p < kPackets; ++p) {
      // Accumulate the exponentials
      GET_NUMBER(local_cum) += GET_NUMBER(local_exps[p]);

    }  // cumsum_in
    GET_NUMBER(sum) += GET_NUMBER(local_cum);
  }  // cumsum_out

  // compute scale
  GET_NUMBER(scale) = 1.0f / static_cast<float>(GET_NUMBER(sum));

prod_out:
  for (uint64_t elem = 0; elem < size; elem += kPackets) {
#pragma HLS PIPELINE
//#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    RawDataT raw_in2 = in_stream.read();
    RawDataT raw_out = 0;
  norm_in:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;
      AccT num = {0};

      // Get the number
      GET_RAW(num) = raw_in2(offhigh, offlow);

      // Scale
      GET_NUMBER(num) = explut(GET_NUMBER(num)) * DataT(GET_NUMBER(scale));

      // Store
      raw_out(offhigh, offlow) = GET_RAW(num);
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
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    out[i] = out_stream.read();
  }
}

extern "C" {

void softmax_lut(RawDataT *in1, RawDataT *out, uint64_t size) {
#pragma HLS INTERFACE m_axi offset = slave port = in1 bundle = gmem0 depth = 32
#pragma HLS INTERFACE m_axi offset = slave port = out bundle = gmem1 depth = 32
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