# Compute number of packets
set npackets [expr { $bus / $dwidth }]

for { set b 0}  {$b < 3} {incr b} {
for { set a 0}  {$a < $npackets} {incr a} {
  set_directive_stream -depth 16 "cascade" stream_a\[${b}\]\[${a}\]
  set_directive_stream -depth 16 "cascade" stream_b\[${b}\]\[${a}\]
  set_directive_stream -depth 32 "cascade" stream_out\[${b}\]\[${a}\]
}
}
