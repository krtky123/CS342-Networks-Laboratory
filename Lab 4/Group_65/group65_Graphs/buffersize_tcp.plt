set terminal png
set output "buffersize_tcp.png"
set title "Buffer Size vs Throughput(TCP)"
set xlabel "Buffer Size(packets)"
set ylabel "Throughput(in mbps)"

set xrange [0:800]
plot "-"  title "TCP Throughput" with linespoints
10 7.96819
40 18.9106
70 29.2735
100 30.2182
130 30.1942
160 30.3842
190 30.4211
220 30.4579
300 30.5096
400 30.5434
500 30.802
600 30.8069
700 30.8069
800 30.8069
e
