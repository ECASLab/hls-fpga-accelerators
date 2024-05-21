/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef __ELEMENTWISE_H__
#define __ELEMENTWISE_H__

#include "../common/config.h"

// Adjustable for Element Wise
#ifndef M_COLS
static constexpr int kCols = 4096;
#else
static constexpr int kCols = M_COLS;
#endif
#ifndef M_ROWS
static constexpr int kRows = 4096;
#else
static constexpr int kRows = M_ROWS;
#endif

static constexpr uint64_t kTotalMaxSize = kCols * kRows / kPackets;

extern "C" {
void elementwise(RawDataT *in1, RawDataT *in2, RawDataT *out, uint64_t size,
                 int op);
}

#endif // __ELEMENTWISE_H__
