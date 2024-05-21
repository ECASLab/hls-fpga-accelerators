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

### Unary

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
| IMPLEXP               | LUT, STD | LUT |

The `xcu250-figd2104-2L-e` is an Alveo U250, whereas `xck26-sfvc784-2LV-c` is a Kria K26

IMPLEXP: implementation of the exponential. STD: standard HLS library and LUT: approximate LUT interpolation

## Authors

* Luis G. Leon Vega <luis.leon@ieee.org>
* Luis Prieto Sibaja <prieto.luisdaniel@estudiantec.cr>
