set terminal png
set output "buffersize_udp.png"
set title "Buffer Size vs Throughput(UDP)"
set xlabel "Buffer Size(packets)"
set ylabel "Throughput(in mbps)"

set xrange [0:800]
plot "-"  title "UDP Throughput" with linespoints
10 57.5465
40 57.5021
70 57.4618
100 57.4443
130 57.4478
160 57.4448
190 57.4448
220 57.4448
300 57.4496
400 57.4482
500 57.4448
600 57.4448
700 57.4448
800 57.4448
e
