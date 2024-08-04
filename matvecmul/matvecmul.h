/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef __MATVECMUL_H__
#define __MATVECMUL_H__

#define AP_INT_MAX_W 32768

#include "../common/config.h"

#ifndef A_ROWS
static constexpr int kARows = kPackets;
#else
static constexpr int kARows = A_ROWS;
#endif
#ifndef B_COLS
static constexpr int kBCols = 32768;
#else
static constexpr int kBCols = B_COLS;
#endif

#ifndef REPLICAS
static constexpr int kReplicas = 2;
#else
static constexpr int kReplicas = REPLICAS;
#endif

extern "C" {
void matvecmul(RawDataT *a, RawDataT *b, RawDataT *c, int a_rows, int b_cols,
               int c_cols);
}

#endif  // __MATVECMUL_H__
