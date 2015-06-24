file=$1

../cnf2aig -t 0 $file > dd.aig
aigtoaig dd.aig > /dev/null
