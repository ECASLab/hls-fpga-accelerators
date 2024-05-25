/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef __MATVECMUL_H__
#define __MATVECMUL_H__

#define AP_INT_MAX_W 32768

#include "../common/config.h"

#ifndef A_ROWS
static constexpr int kARows = 2;
#else
static constexpr int kARows = A_ROWS;
#pragma message "Refef"
#endif
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
void matvecmul(RawDataT *a, RawDataT *b, RawDataT *c, int a_rows, int b_cols, int c_cols);
}

#endif // __MATVECMUL_H__
