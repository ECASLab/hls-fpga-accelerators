/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_stream.h>

#ifndef BUS
static constexpr int kBusWidth = 512;
#else
static constexpr int kBusWidth = BUS;
#endif
//#define USE_FLOAT8

#ifdef USE_FLOAT32
static constexpr int kDataWidth = 32;
#elif defined(USE_FLOAT16)
static constexpr int kDataWidth = 16;
#elif defined(USE_FLOAT8)
static constexpr int kDataWidth = 8;
#elif defined(USE_FLOAT4)
static constexpr int kDataWidth = 4;
#elif defined(USE_FIXED16)
static constexpr int kFxPDataWidth = 16;
static constexpr int kFxPDataInt = 6;
static constexpr int kDataWidth = kFxPDataWidth;
using DataT = ap_fixed<kFxPDataWidth, kFxPDataInt>;
#else
static constexpr int kFxPDataWidth = 8;
static constexpr int kFxPDataInt = 4;
static constexpr int kDataWidth = kFxPDataWidth;
using DataT = ap_fixed<kFxPDataWidth, kFxPDataInt>;
#endif

static constexpr int kPackets = kBusWidth / kDataWidth;

using RawDataT = ap_uint<kBusWidth>;
using StreamT = hls::stream<RawDataT>;

#ifdef USE_FLOAT32
#define USE_UNION
typedef union {
  uint32_t i;
  float f;
} d32;
using AccT = d32;
#elif defined(USE_FLOAT16) || defined(USE_FLOAT8) || defined(USE_FLOAT4)
#define USE_UNION
typedef union {
  uint16_t i;
  half f;
} d16;
using AccT = d16;
#else
using AccT = DataT;
#endif

#ifdef USE_UNION
#define GET_NUMBER(n) (n).f
#define GET_RAW(n) (n).i
#else
#define GET_NUMBER(n) (n)
#define GET_RAW(n) (n).V
#endif

#endif // __CONFIG_H__
