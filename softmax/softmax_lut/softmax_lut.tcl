

#
# Copyright 2022-2024
# Author: Anthony Leiva <anleva1720@gmail.com>
#

catch {::common::set_param -quiet hls.xocc.mode csynth};



open_project -reset softmax_lut
set_top softmax_lut

add_files "softmax_lut.cpp" -cflags " -DALLOW_EMPTY_HLS_STREAM_READS -I ./"
add_files -tb "softmax_lut_tb.cpp" -cflags " -DALLOW_EMPTY_HLS_STREAM_READS -I ./"
open_solution -flow_target vitis solution
set_part xck26-sfvc784-2LV-c

create_clock -period 200MHz -name default

config_dataflow -strict_mode warning
config_rtl -deadlock_detection sim

config_interface -m_axi_conservative_mode=1
config_interface -m_axi_addr64
config_interface -m_axi_auto_max_ports=0

config_export -format xo -ipname softmax_lut

#csim_design
csynth_design
#cosim_design

close_project
puts "HLS completed successfully"
exit