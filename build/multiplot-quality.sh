#!/bin/bash
name=(festive panda tobasco osmp raahs fdash sftm svaa)
file1=dash_festive_result.txt
file2=dash_panda_result.txt
file3=dash_tobasco_result.txt
file4=dash_osmp_result.txt
file5=dash_raahs_result.txt
file6=dash_fdash_result.txt
file7=dash_sftm_result.txt
file8=dash_svaa_result.txt

pic=dash-pic-1
gnuplot -persist <<-EOF
set term "png"
set output "${pic}.png"
TOP=0.98
DY = 0.45
seam=0.01
x_gap=0.02
central_axis=0.52

set multiplot
set offset 0,0,graph 0.05, graph 0.05
set xlabel 'time'
set ylabel 'loss/%' offset 1
set lmargin at screen 0.08
set rmargin at screen central_axis-x_gap
set tmargin at screen TOP-seam
set bmargin at screen TOP-2*DY-seam
set xrange [0:200000]
set ytics 0,1,5
plot "${file1}" u 1:3  title "${name[0]}" with steps lw 2 ,\
"${file2}" u 1:3   title "${name[1]}" with steps lw 2 ,\

set xlabel 'index'
unset ylabel
set lmargin at screen central_axis+x_gap
set rmargin at screen 0.99
set tmargin at screen TOP-seam
set bmargin at screen TOP-2*DY-seam
plot "${file3}" u 1:3  title "${name[2]}" with steps lw 2 ,\
"${file4}" u 1:3   title "${name[3]}" with steps lw 2
unset multiplot;
set output
EOF

pic=dash-pic-2
gnuplot -persist <<-EOF
set term "png"
set output "${pic}.png"
TOP=0.98
DY = 0.45
seam=0.01
x_gap=0.02
central_axis=0.52

set multiplot
set offset 0,0,graph 0.05, graph 0.05
set xlabel 'time'
set ylabel 'loss/%' offset 1
set lmargin at screen 0.08
set rmargin at screen central_axis-x_gap
set tmargin at screen TOP-seam
set bmargin at screen TOP-2*DY-seam
set xrange [0:200000]
set ytics 0,1,5
plot "${file5}" u 1:3  title "${name[4]}" with steps lw 2 ,\
"${file6}" u 1:3   title "${name[5]}" with steps lw 2 ,\

set xlabel 'index'
unset ylabel
set lmargin at screen central_axis+x_gap
set rmargin at screen 0.99
set tmargin at screen TOP-seam
set bmargin at screen TOP-2*DY-seam
plot "${file7}" u 1:3  title "${name[6]}" with steps lw 2 ,\
"${file8}" u 1:3   title "${name[7]}" with steps lw 2
unset multiplot;
set output
EOF




