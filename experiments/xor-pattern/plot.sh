#!/bin/bash
#./plotXY.sh $in 2 3

if [ $# -lt 1 ]; then
  echo "Usage: $0 filename"
  exit
fi

f=$1

title="Comparison of absolute numbers of recognized gates"

echo "set key outside left top"
echo "set key off"

#echo "set logscale x"
#echo "set logscale y"
echo "set title \"$title\""

echo "f(x)=x"
echo "timeout=3600"

echo "set datafile separator \" \""
echo "set size square 1,1"

echo "set xlabel 'Number of Gates (old)'"
echo "set ylabel 'Number of Gates (new)'"

#echo "set xrange [1:]"
#echo "set yrange [1:]"

echo "set style line 1 lt 1 lw 1 lc 26" #rgb 'red'"
echo "set style line 2 lt 1 lw 1 lc 27" #rgb 'green'"

#echo "plot timeout ls 1 title 'Timeout'"
#echo "replot '< echo \"3600 5000\"' w impulse ls 1 notitle"
echo "plot f(x) ls 2 notitle"
echo "replot \"$f\" u (\$3 / \$2):(\$4 / \$2) title \"Number of Gates\" with points lc 1"

if [ $# -lt 2 ]; then
  echo "pause 1000"
else
  echo "set term postscript eps enhanced color "
  #echo 'set term pdf'
  echo "set output '$2'"
  echo "replot"
fi
