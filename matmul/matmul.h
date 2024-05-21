/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef __MATMUL_H__
#define __MATMUL_H__

#include "../common/config.h"

extern "C" {
void matmul(RawDataT *a, RawDataT *b, RawDataT *c, int a_rows, int b_cols, int c_cols);
}

#endif // __MATMUL_H__
