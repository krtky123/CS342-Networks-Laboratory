set terminal png
set output "otherudp_udprate.png"
set title "UDP Rate vs Throughput of other UDP connection"
set xlabel "UDP Rate(Mbps)"
set ylabel "Throughput(Mbps)"

set xrange [10:100]
plot "-"  title "UDP Throughput of Other UDP Connection" with linespoints
20 3.95201
30 4.02776
40 4.06857
50 4.09249
60 4.10747
70 4.11889
80 4.12779
90 4.13388
100 4.13735
e
