set terminal png
set output "buffersize_fairness.png"
set title "Buffer Size vs Fairness plot"
set xlabel "Buffer Size(packets)"
set ylabel "Fairness"

set xrange [0:800]
plot "-"  title "Fairness" with linespoints
10 0.248351
40 0.326634
70 0.388305
100 0.396124
130 0.395763
160 0.397189
190 0.397347
220 0.397506
300 0.397547
400 0.39734
500 0.398968
600 0.398988
700 0.398988
800 0.398988
e
