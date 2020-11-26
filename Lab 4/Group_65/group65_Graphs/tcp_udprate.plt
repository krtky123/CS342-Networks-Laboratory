set terminal png
set output "tcp_udprate.png"
set title "UDP Rate vs Throughput(TCP)"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "TCP Throughput" with linespoints
20 7.48456
30 7.66593
40 7.79248
50 7.84954
60 7.87958
70 7.91786
80 7.93874
90 7.94131
100 7.96819
e
