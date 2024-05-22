# hls-fpga-accelerators

Collection of kernel accelerators optimised for LLM execution

## Compilation

### Matrix Multiplication

```bash
cd matmul
vitis_hls -f matmul.tcl
```

Possible adjustments through environment variables:

| Environment Variable | Possible Values | Default |
|----------------------|-----------------|---------|
| DATATYPE             | FLOAT4, FLOAT8, FLOAT16, FLOAT32, FIXED8, FIXED16 | FIXED16 |
| BUS             | 64, 128, 256, 512, 1024, 2048 | 512 |
| B_COLS             | Power of two from 64 on | 4096 |
| C_COLS             | Power of two from 64 on | 4096 |
| PART               | xcu250-figd2104-2L-e, xck26-sfvc784-2LV-c | xcu250-figd2104-2L-e |

The `xcu250-figd2104-2L-e` is an Alveo U250, whereas `xck26-sfvc784-2LV-c` is a Kria K26

Function signature:

```c
void matmul(RawDataT *a, RawDataT *b, RawDataT *c, int a_rows, int b_cols, int c_cols)
```

* `a`: memory-mapped matrix A
* `b`: memory-mapped matrix B (assumed transposed)
* `c`: memory-mapped matrix C
* `a_rows`: rows of matrix A
* `b_cols`: columns of matrix A
* `c_cols`: columns of matrix C

### Matrix/Vector Elementwise

```bash
cd elementwise
vitis_hls -f elementwise.tcl
```

Possible adjustments through environment variables:

| Environment Variable | Possible Values | Default |
|----------------------|-----------------|---------|
| DATATYPE             | FLOAT4, FLOAT8, FLOAT16, FLOAT32, FIXED8, FIXED16 | FIXED16 |
| BUS             | 64, 128, 256, 512, 1024, 2048 | 512 |
| M_COLS             | Power of two from 64 on | 4096 |
| M_ROWS             | Power of two from 64 on | 4096 |
| PART               | xcu250-figd2104-2L-e, xck26-sfvc784-2LV-c | xcu250-figd2104-2L-e |

The `xcu250-figd2104-2L-e` is an Alveo U250, whereas `xck26-sfvc784-2LV-c` is a Kria K26

Function signature:

```c
void elementwise(RawDataT *in1, RawDataT *in2, RawDataT *out, uint64_t size,
                 int op);
```

* `in1`: memory-mapped matrix A
* `in2`: memory-mapped matrix B
* `out`: memory-mapped matrix C
* `size`: total number of elements of the matrix: cols * rows
* `op`: 0: add, 1: multiply

### Unary

```bash
cd unary
vitis_hls -f unary.tcl
```

Possible adjustments through environment variables:

| Environment Variable | Possible Values | Default |
|----------------------|-----------------|---------|
| DATATYPE             | FLOAT4, FLOAT8, FLOAT16, FLOAT32, FIXED8, FIXED16 | FIXED16 |
| BUS             | 64, 128, 256, 512, 1024, 2048 | 512 |
| M_COLS             | Power of two from 64 on | 4096 |
| M_ROWS             | Power of two from 64 on | 4096 |
| PART               | xcu250-figd2104-2L-e, xck26-sfvc784-2LV-c | xcu250-figd2104-2L-e |
| IMPLEXP               | LUT, STD | LUT |

The `xcu250-figd2104-2L-e` is an Alveo U250, whereas `xck26-sfvc784-2LV-c` is a Kria K26

IMPLEXP: implementation of the exponential. STD: standard HLS library and LUT: approximate LUT interpolation

Function signature:

```c
void unary(RawDataT *in, RawDataT *out, uint64_t size, int op);
```

* `in1`: memory-mapped matrix A
* `out`: memory-mapped matrix C
* `size`: total number of elements of the matrix: cols * rows
* `op`: 0: none, 1: ReLU, 2: SILU

### RMSNORM

```bash
cd rmsnorm
vitis_hls -f rmsnorm.tcl
```

Possible adjustments through environment variables:

| Environment Variable | Possible Values | Default |
|----------------------|-----------------|---------|
| DATATYPE             | FLOAT4, FLOAT8, FLOAT16, FLOAT32, FIXED8, FIXED16 | FIXED16 |
| BUS             | 64, 128, 256, 512, 1024, 2048 | 512 |
| M_COLS             | Power of two from 64 on | 4096 |
| M_ROWS             | Power of two from 64 on | 4096 |
| PART               | xcu250-figd2104-2L-e, xck26-sfvc784-2LV-c | xcu250-figd2104-2L-e |

The `xcu250-figd2104-2L-e` is an Alveo U250, whereas `xck26-sfvc784-2LV-c` is a Kria K26

> It is better to use FLOAT data types given the nature of the normalisation.

Function signature:

```c
void rmsnorm(RawDataT *in, RawDataT *out, uint64_t size);
```

* `in1`: memory-mapped matrix A
* `out`: memory-mapped matrix C
* `size`: total number of elements of the matrix: cols * rows

## Authors

* Luis G. Leon Vega <luis.leon@ieee.org>
* Luis Prieto Sibaja <prieto.luisdaniel@estudiantec.cr>
