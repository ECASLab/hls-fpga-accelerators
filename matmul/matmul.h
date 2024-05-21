/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef __MATMUL_H__
#define __MATMUL_H__

#include "../common/config.h"

static constexpr int kARows = 2;
#ifndef B_COLS
static constexpr int kBCols = 32768;
#else
static constexpr int kBCols = B_COLS;
#endif
#ifndef C_COLS
static constexpr int kCCols = 32768;
#else
static constexpr int kCCols = C_COLS;
#endif

extern "C" {
void matmul(RawDataT *a, RawDataT *b, RawDataT *c, int a_rows, int b_cols, int c_cols);
}

#endif // __MATMUL_H__
