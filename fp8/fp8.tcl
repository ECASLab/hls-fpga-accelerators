#
# Copyright 2022-2025
# Author: Luis G. Leon-Vega <luis.leon@ieee.org>
#

catch {::common::set_param -quiet hls.xocc.mode csynth};

open_project fp8
set_top fp8

add_files "./fp8.cpp"
add_files -tb "./fp8-tb.cpp"
open_solution -flow_target vitis solution
set_part xck26-sfvc784-2LV-c

create_clock -period 300MHz -name default

config_dataflow -strict_mode warning
config_rtl -deadlock_detection sim

config_interface -m_axi_conservative_mode=1
config_interface -m_axi_addr64
config_interface -m_axi_auto_max_ports=0

config_export -format xo -ipname fp8
csim_design
close_project
puts "HLS completed successfully"
exit
