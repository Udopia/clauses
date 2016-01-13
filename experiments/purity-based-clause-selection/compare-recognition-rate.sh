#!/bin/bash

rm stc*.tmp

echo 'set key top left invert reverse Left'
echo 'set logscale y'
echo 'set datafile missing "0"'
echo 'set xlabel "Problems"'
echo 'set ylabel "#Gates / #Vars"'
count=0

if [ $# -lt 1 ]; then 
  echo "missing argument: results directory"
  exit 1
fi

dir=$1

for f in $dir/*; do 
  method=$(basename $f)
  total=`cat $f | wc -l`
  #for i in `seq $total 300`; do echo "0"; done >> stc$count.tmp
  awk -F "," '{ printf "%.2f\n", $4 / $2 }' $f | sort -g >> stc$count.tmp
  
  if [ $count -eq 0 ]; then
    echo 'plot "stc'$count.tmp'" t "'$f'" w l'
  else
    echo 'replot "stc'$count'.tmp" t "'$f'" w l'
  fi
  ((count+=1))
  
  echo 'set term pdf'
  echo "set output 'result.pdf'"
  echo 'replot'
done
