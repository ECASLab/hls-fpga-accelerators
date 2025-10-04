#include "softmax_taylor.h"


static void load_input(RawDataT *in, hls::stream<RawDataT> &inStream,
                       uint64_t size) {
  const uint64_t size_raw = size / kPackets;
mem_reps:
  for (int i = 0; i < 2; ++i) {
  mem_rd:
    for (uint64_t i = 0; i < size_raw; ++i) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
      inStream << in[i];
    }
  }
}

static void compute_aprox(hls::stream<RawDataT> &in_stream,
                          hls::stream<RawDataT> &out_stream, uint64_t size) {
#pragma HLS INLINE off
  
  using ExpOpTaylor =
      axc::nonlinear::approximate::taylor::Exponential<DataT,
                                                       korder,is_fp>;
  ExpOpTaylor exptaylor{};

  DataT sum = {0};
  AccT scale = {0.0f};

// Cumsum
cumsum_out:
  for (int i = 0; i < size; i += kPackets) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    DataT local_cum = {0};
    DataT local_exps[kPackets] = {0};
    RawDataT raw_in1 = in_stream.read();

  compute_exps:
    for (int p = 0; p < kPackets; ++p) {
      // offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;

      ap_uint<kDataWidth> raw_bits = raw_in1.range(offhigh, offlow);
      // Extract the input
      DataT num = GET_NUMBER<DataT>(raw_bits);

      local_exps[p] = exptaylor(num);
    }  // compute_Exps

  // compute cumsum
  cumsum_in:
    for (int p = 0; p < kPackets; ++p) {
      // Accumulate exponentials
      local_cum = local_cum + local_exps[p];

    }  // cumsum_in
    sum = sum + local_cum;
  }  // cumsum_out

  // compute scale
  scale = 1.0 / toFloat(sum);
  


prod_out:
  for (uint64_t elem = 0; elem < size; elem += kPackets) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    RawDataT raw_in2 = in_stream.read();
    RawDataT raw_out = 0;
  norm_in:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;
      ap_uint<kDataWidth> raw_bits = raw_in2.range(offhigh, offlow);

      DataT num = GET_NUMBER<DataT>(raw_bits);


      // Scale
      num =
          exptaylor(num) * DataT(scale);

    
      // Store
      raw_out.range(offhigh, offlow) = GET_RAW(num);

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

void softmax_taylor(RawDataT *in1, RawDataT *out, uint64_t size) {
#pragma HLS INTERFACE m_axi offset = slave port = in1 bundle = gmem0 depth = kTotalMaxSize
#pragma HLS INTERFACE m_axi offset = slave port = out bundle = gmem1 depth = kTotalMaxSize
#pragma HLS INTERFACE s_axilite register port = size
#pragma HLS INTERFACE s_axilite register port = return

  static StreamT stream_a;
  static StreamT stream_c;
#pragma HLS stream variable = stream_a depth = 32
#pragma HLS stream variable = stream_c depth = 32

#pragma HLS dataflow
  load_input(in1, stream_a, size);
  compute_aprox(stream_a, stream_c, size);
  store_result(out, stream_c, size);
}
}