#include "softmax_lut.h"

static void load_input(RawDataT *in, hls::stream<RawDataT> &inStream,
                       uint64_t size) {
#pragma HLS INLINE off
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

static void compute(hls::stream<RawDataT> &in_stream,
                          hls::stream<RawDataT> &out_stream, uint64_t size) {
#pragma HLS INLINE off

  using Start = std::ratio<START_APROX>;
  using End = std::ratio<END_APROX>;
  using ExpOpLut =
      axc::nonlinear::approximate::lut::Exponential<DataT, Start, End, KNUMPOINTS, IS_FP>;
      ExpOpLut explut{};

  AccT sum = {0};
  AccT scale = {0};

// Cumsum
cumsum_out:
  for (int i = 0; i < size; i += kPackets) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    AccT local_cum = {0.0f};
    DataT local_exps[kPackets] = {0};
    RawDataT raw_in1 = in_stream.read();

  compute_exps:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;

      ap_uint<kDataWidth> raw_bits = raw_in1.range(offhigh, offlow);
      // Extract the input
      DataT num = GET_NUMBER<DataT>(raw_bits);

      local_exps[p] = explut(num);
      //local_exps[p] = hls::exp(num);
      

    }  // compute_Exps

  // compute cumsum
  cumsum_in:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS UNROLL
      // Accumulate exponentials
      #if IS_FP == 0
            //local_cum = local_cum + toFloat(local_exps[p]); // revisar el toFloat
            local_cum = local_cum + local_exps[p]; // revisar el toFloat

      #else
            local_cum = local_cum + toFloat(local_exps[p]);
            //local_cum = local_cum + local_exps[p]; // revisar el toFloat

      #endif

    }  // cumsum_in
    sum = sum + local_cum;
  }  // cumsum_out

  // compute scale
  //  
  #if USE_RECIPROCAL == 1
    scale = sum.reciprocal();
  #else
      scale = AccT(1.0) / AccT(sum);
  #endif

  


prod_out:
  for (uint64_t elem = 0; elem < size; elem += kPackets) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kTotalMaxSize max = kTotalMaxSize avg = \
    kTotalMaxSize
    RawDataT raw_in2 = in_stream.read();
    RawDataT raw_out = 0;
  norm_in:
    for (int p = 0; p < kPackets; ++p) {
      // Offsets
      const int offlow = p * kDataWidth;
      const int offhigh = offlow + kDataWidth - 1;
      ap_uint<kDataWidth> raw_bits = raw_in2.range(offhigh, offlow);

      DataT num = GET_NUMBER<DataT>(raw_bits);

      
      // Scale
      num =explut(num);
      //num = hls::exp(num);
      num = num*DataT(scale);


    
      // Store
      raw_out.range(offhigh, offlow) = GET_RAW(num);

    }
    out_stream << raw_out;
  }
}

static void store_result(RawDataT *out, hls::stream<RawDataT> &out_stream,
                         uint64_t size) {
#pragma HLS INLINE off
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
#pragma HLS INTERFACE m_axi offset = slave port = in1 bundle = gmem0 depth = 16
#pragma HLS INTERFACE m_axi offset = slave port = out bundle = gmem1 depth = 16
#pragma HLS INTERFACE s_axilite register port = size
#pragma HLS INTERFACE s_axilite register port = return

  static StreamT stream_a;
  static StreamT stream_c;
#pragma HLS stream variable = stream_a depth = 16
#pragma HLS stream variable = stream_c depth = 16

#pragma HLS dataflow
  load_input(in1, stream_a, size);
  compute(stream_a, stream_c, size);
  store_result(out, stream_c, size);
}
}
