/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include "cascade.h"

#include "hls_math.h"

static constexpr int kARowsPacketised = kARows / kPackets;
static constexpr int kBColsPacketised = kBCols / kPackets;
static constexpr int kCColsPacketised = kCCols / kPackets;
static constexpr int kCElemsPacketised = (kCCols * kARows) / kPackets;
static constexpr int kStreamElements = kARowsPacketised * kBColsPacketised;

static constexpr int kNumReplicas = 2;
static constexpr int kARowsReplicas = kARows / kNumReplicas;
static constexpr int kBColsReplicas = kBCols / kNumReplicas;
static constexpr int kCColsReplicas = kCCols / kNumReplicas;
static constexpr int kCElemsReplicas = (kCCols * kARows) / kNumReplicas;
static constexpr int kStreamElementsReplicas =
    kARowsReplicas * kBColsPacketised;

template <int NT, int N>
struct AddPairs {
  static DataT Execute(DataT vec[NT], int idx = 0) {
#pragma HLS inline off
    DataT lhs = AddPairs<NT, N / 2>::Execute(vec, idx);
    DataT rhs = AddPairs<NT, N / 2>::Execute(vec, N / 2 + idx);
    return lhs + rhs;
  }
};

template <int NT>
struct AddPairs<NT, 1> {
  static DataT Execute(DataT vec[NT], int idx) {
#pragma HLS inline off
    return vec[idx];
  }
};

static void matvecmul_gemm_stream(StreamT &a, StreamT &b, StreamSingleT &c,
                                  const int b_cols) {
#pragma HLS INLINE off
#pragma HLS PIPELINE

  DataT res[kPackets];
#pragma HLS ARRAY_PARTITION dim = 0 type = complete variable = res

  AccT tres{0};
  for (int p = 0; p < kPackets; ++p) {
#pragma HLS unroll
    res[p] = DataT{0};
  }

gemv_reduce:
  for (int b_col = 0; b_col < b_cols; b_col += kPackets) {  // k
#pragma HLS LOOP_TRIPCOUNT min = kBColsPacketised max = kBColsPacketised avg = \
    kBColsPacketised
#pragma HLS PIPELINE
    RawDataT a_packet = a.read();
    RawDataT b_packet = b.read();

  gemv_reduce_packet:
    for (int p = 0; p < kPackets; ++p) {
#pragma HLS LOOP_TRIPCOUNT min = kPackets max = kPackets avg = kPackets
#pragma HLS UNROLL
      const int low = p * kDataWidth;
      const int high = low + kDataWidth - 1;
      AccT a_val, b_val;
      GET_RAW(a_val) = a_packet(high, low);
      GET_RAW(b_val) = b_packet(high, low);
      res[p] = GET_NUMBER(a_val) * GET_NUMBER(b_val);
    }
  }

  GET_NUMBER(tres) = AddPairs<kPackets, kPackets>::Execute(res);
  c.write(GET_RAW(tres));
}

static void matvecmul_gemm(StreamT a[kNumReplicas], StreamT b[kNumReplicas],
                           StreamSingleT c[kNumReplicas], const int a_rows,
                           const int b_cols) {
#pragma HLS INLINE off
#pragma HLS ARRAY_PARTITION variable = a type = complete dim = 0
#pragma HLS ARRAY_PARTITION variable = b type = complete dim = 0
#pragma HLS ARRAY_PARTITION variable = c type = complete dim = 0

  // TODO: the rows of C are grouped given that stream C is parallel
gemv_c_rows:
  for (int c_row = 0; c_row < a_rows; c_row += kNumReplicas) {  // m
#pragma HLS LOOP_TRIPCOUNT min = kARowsReplicas max = kARowsReplicas avg = \
    kARowsReplicas
#pragma HLS PIPELINE
    // TODO: the streams take place here
  gemv_c_streams:
    for (int s = 0; s < kNumReplicas; ++s) {
#pragma HLS UNROLL
#pragma HLS LOOP_TRIPCOUNT min = kNumReplicas max = kNumReplicas avg = \
    kNumReplicas
      matvecmul_gemm_stream(a[s], b[s], c[s], b_cols);
    }
  }
}

static void matvecmul_to_stream_a(RawDataT *a, StreamT sa[kNumReplicas],
                                  const int rows, const int cols) {
#pragma HLS INLINE off
  const int tcols = cols / kNumReplicas;

  // Repeated matrix transmission
a_rows:
  for (int row = 0; row < rows; row += kNumReplicas) {
#pragma HLS LOOP_TRIPCOUNT min = kARowsReplicas max = kARowsReplicas avg = \
    kARowsReplicas
    // Transmit columns
  a_cols:
    for (int col = 0; col < tcols; ++col) {
#pragma HLS LOOP_TRIPCOUNT min = kBColsPacketised max = kBColsPacketised avg = \
    kBColsPacketised
#pragma HLS LOOP_FLATTEN
#pragma HLS PIPELINE
      // Interleaved row access -> probably not the clevest but solvable
    a_streams:
      for (int s = 0; s < kNumReplicas; ++s) {
#pragma HLS LOOP_TRIPCOUNT min = kNumReplicas max = kNumReplicas avg = \
    kNumReplicas
#pragma HLS UNROLL
        const int row_shift = (row + s) * tcols;
        const int cols_shift = col;
        const int shift = cols_shift + row_shift;
        RawDataT packet = a[shift];
        sa[s].write(packet);
      }
    }
  }
}

// Requires splitting
static void fused_stream_b_scale(RawDataT *wmul, RawDataT *a, StreamT &sa,
                                 const int cols, const int rep_rows) {
#pragma HLS INLINE off
  const int tcols = cols / kPackets;

  // Repeated row transmission
b_mat_reps:
  for (int rep_row = 0; rep_row < rep_rows; rep_row += kPackets) {
#pragma HLS LOOP_TRIPCOUNT min = kARowsPacketised max = kARowsPacketised avg = \
    kARowsPacketised
#pragma HLS PIPELINE
    // Transmit columns
  b_mat_cols:
    for (int col = 0; col < tcols; ++col) {
#pragma HLS LOOP_TRIPCOUNT min = kBColsPacketised max = kBColsPacketised avg = \
    kBColsPacketised
#pragma HLS LOOP_FLATTEN off
#pragma HLS PIPELINE
      const int row_shift = 0;
      const int cols_shift = col;
      const int shift = cols_shift + row_shift;
      RawDataT packet = a[shift] * wmul[shift];
      sa.write(packet);
    }
  }
}

static void stream_splitter(StreamT &in, StreamT outs0[kNumReplicas],
                            StreamT outs1[kNumReplicas],
                            StreamT outs2[kNumReplicas], const int cols,
                            const int rep_rows) {
#pragma HLS INLINE off
  const int telems = cols * rep_rows;
  const int step = kPackets * kPackets;
  for (int elem = 0; elem < telems; elem += step) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kStreamElements max = kStreamElements avg = \
    kStreamElements
    RawDataT packet = in.read();
  b_mat_streams:
    for (int s = 0; s < kNumReplicas; ++s) {
#pragma HLS UNROLL
#pragma HLS LOOP_TRIPCOUNT min = kNumReplicas max = kNumReplicas avg = \
    kNumReplicas
      outs0[s].write(packet);
      outs1[s].write(packet);
      outs2[s].write(packet);
    }
  }
}

static void matvecmul_from_stream(RawDataT *a, StreamSingleT sa[kNumReplicas],
                                  const int rows) {
#pragma HLS INLINE off
  const int trows = rows / kNumReplicas;
c_rows:
  for (int i = 0; i < trows; ++i) {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = kARowsReplicas max = kARowsReplicas avg = \
    kARowsReplicas
  c_streams:
    RawDataT packet;
    for (int s = 0; s < kNumReplicas; ++s) {
#pragma HLS LOOP_TRIPCOUNT min = kNumReplicas max = kNumReplicas avg = \
    kNumReplicas
      const int low = s * kDataWidth;
      const int high = low + kDataWidth - 1;
      packet(high, low) = sa[s].read();
    }
    a[i] = packet;
  }
}

extern "C" {

/**
 * matrix: (rows, cols)
 * a: input (samples, inputs)
 * b: weights (outputs, inputs) assumed transposed [1, inputs]
 * c: output (samples, outputs). Assumed [samples, 1]
 */
void cascade(RawDataT *wmul, RawDataT *a0, RawDataT *a1, RawDataT *a2,
             RawDataT *b, RawDataT *c0, RawDataT *c1, RawDataT *c2, int a_rows,
             int b_cols, int c_cols) {
#pragma HLS INTERFACE m_axi offset = slave port = wmul bundle = gmem0
#pragma HLS INTERFACE m_axi offset = slave port = b bundle = gmemb
#pragma HLS INTERFACE m_axi offset = slave port = a0 bundle = gmem1
#pragma HLS INTERFACE m_axi offset = slave port = a1 bundle = gmem2
#pragma HLS INTERFACE m_axi offset = slave port = a2 bundle = gmem3
#pragma HLS INTERFACE m_axi offset = slave port = c0 bundle = gmem1
#pragma HLS INTERFACE m_axi offset = slave port = c1 bundle = gmem2
#pragma HLS INTERFACE m_axi offset = slave port = c2 bundle = gmem3
#pragma HLS INTERFACE s_axilite register port = a_rows
#pragma HLS INTERFACE s_axilite register port = b_cols
#pragma HLS INTERFACE s_axilite register port = c_cols
#pragma HLS INTERFACE s_axilite register port = return

  // TODO: Make this dynamic through the directive file. Here, we assume two
  // rows at a time
  static StreamT stream_b_single;

  // TODO: A stream is in charge of a row, whereas B stream is redundant
  static StreamT stream_a[3][kNumReplicas];
#pragma HLS ARRAY_PARTITION dim = 0 type = complete variable = stream_a
  static StreamT stream_b[3][kNumReplicas];
#pragma HLS ARRAY_PARTITION dim = 0 type = complete variable = stream_b

  // TODO: Make this dynamic through the directive file. Here we assume FLOAT32
  static StreamSingleT stream_out[3][kNumReplicas];
#pragma HLS ARRAY_PARTITION dim = 0 type = complete variable = stream_out

#pragma HLS dataflow
  // Get the input vector and multiply it
  fused_stream_b_scale(wmul, b, stream_b_single, b_cols, a_rows);
  stream_splitter(stream_b_single, stream_b[0], stream_b[1], stream_b[2],
                  b_cols, a_rows);

  matvecmul_to_stream_a(a0, stream_a[0], a_rows, b_cols);
  matvecmul_to_stream_a(a1, stream_a[1], a_rows, b_cols);
  matvecmul_to_stream_a(a2, stream_a[2], a_rows, b_cols);

  matvecmul_gemm(stream_a[0], stream_b[0], stream_out[0], a_rows, b_cols);
  matvecmul_gemm(stream_a[1], stream_b[1], stream_out[1], a_rows, b_cols);
  matvecmul_gemm(stream_a[2], stream_b[2], stream_out[2], a_rows, b_cols);

  matvecmul_from_stream(c0, stream_out[0], a_rows);
  matvecmul_from_stream(c1, stream_out[1], a_rows);
  matvecmul_from_stream(c2, stream_out[2], a_rows);
}
}
