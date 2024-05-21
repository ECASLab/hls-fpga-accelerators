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

## Authors

* Luis G. Leon Vega <luis.leon@ieee.org>
* Luis Prieto Sibaja <prieto.luisdaniel@estudiantec.cr>
