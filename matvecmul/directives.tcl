# Compute number of packets
set npackets [expr { $bus / $dwidth }]

for { set a 0}  {$a < $npackets} {incr a} {
  set_directive_stream -depth 16 "matvecmul" stream_a\[${a}\]
  set_directive_stream -depth 16 "matvecmul" stream_b\[${a}\]
  set_directive_stream -depth 32 "matvecmul" stream_out\[${a}\]
}
