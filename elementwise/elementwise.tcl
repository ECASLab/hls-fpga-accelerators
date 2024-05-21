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

# Matrix Cols
if { [info exists ::env(COLS) ] } {
  set cols $::env(COLS)
} else {
  set cols 4096
}

# Matrix Rows
if { [info exists ::env(ROWS) ] } {
  set rows $::env(ROWS)
} else {
  set rows 4096
}

open_project elementwise
set_top elementwise
# v++ -g, -D, -I, --advanced.prop kernel.elementwise.kernel_flags
add_files "./elementwise.cpp" -cflags " -DUSE_$datatype -DBUS=$bus -DM_COLS=$cols -DM_ROWS=$rows "
open_solution -flow_target vitis solution
set_part xck26-sfvc784-2LV-c
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
config_export -format xo -ipname elementwise
csynth_design
close_project
puts "HLS completed successfully"
exit
