#
# Copyright 2022-2024
# Author: Anthony Leiva <anleva1720@gmail.com>
#

catch {::common::set_param -quiet hls.xocc.mode csynth};



open_project -reset softmax_taylor
set_top softmax_taylor

add_files "softmax_taylor.cpp" -cflags " -DALLOW_EMPTY_HLS_STREAM_READS -I ./"
add_files -tb "softmax_taylor_tb.cpp" -cflags " -DALLOW_EMPTY_HLS_STREAM_READS -I ./"
open_solution -flow_target vitis solution
set_part xcu55c-fsvh2892-2L-e
create_clock -period 250MHz -name default

config_dataflow -strict_mode warning
config_rtl -deadlock_detection sim

config_interface -m_axi_conservative_mode=1
config_interface -m_axi_addr64
config_interface -m_axi_auto_max_ports=0

config_export -format xo -ipname softmax_taylor

#csim_design
csynth_design
#cosim_design

close_project
puts "HLS completed successfully"
exit