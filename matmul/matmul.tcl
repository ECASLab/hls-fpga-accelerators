#
# Copyright 2022-2024
# Author: Luis G. Leon-Vega <luis.leon@ieee.org>
#

catch {::common::set_param -quiet hls.xocc.mode csynth};

# Datatype default
if { [info exists ::env(DATATYPE) ] } {
  set datatype $::env(DATATYPE)
} else {
  set datatype "FIXED16"
}

# Bus default
if { [info exists ::env(BUS) ] } {
  set bus $::env(BUS)
} else {
  set bus 512
}

# FPGA Part
if { [info exists ::env(PART) ] } {
  set part $::env(PART)
} else {
  set part "xcu250-figd2104-2L-e"
}


# B Columns
if { [info exists ::env(B_COLS) ] } {
  set b_cols $::env(B_COLS)
} else {
  set b_cols 4096
}

# C Columns
if { [info exists ::env(C_COLS) ] } {
  set c_cols $::env(C_COLS)
} else {
  set c_cols 4096
}

open_project matmul
set_top matmul
# v++ -g, -D, -I, --advanced.prop kernel.matmul.kernel_flags
add_files "./matmul.cpp" -cflags " -DUSE_$datatype -DBUS=$bus -DB_COLS=$b_cols -DC_COLS=$c_cols "
open_solution -flow_target vitis solution
set_part $part
create_clock -period 300MHz -name default
# v++ --advanced.param compiler.hlsDataflowStrictMode
config_dataflow -strict_mode warning
# v++ --advanced.param compiler.deadlockDetection
config_rtl -deadlock_detection sim
# v++ --advanced.param compiler.axiDeadLockFree
config_interface -m_axi_conservative_mode=1
config_interface -m_axi_addr64
# v++ --hls.max_memory_ports
config_interface -m_axi_auto_max_ports=0
config_export -format xo -ipname matmul
csynth_design
close_project
puts "HLS completed successfully"
exit
