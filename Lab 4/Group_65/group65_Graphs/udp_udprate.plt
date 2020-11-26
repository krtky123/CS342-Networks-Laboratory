set terminal png
set output "udp_udprate.png"
set title "UDP Rate vs Throughput(UDP)"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "UDP Throughput" with linespoints
20 23.1907
30 26.5011
40 30.588
50 34.9832
60 39.5308
70 44.1686
80 48.8615
90 53.5898
100 57.5465
e
