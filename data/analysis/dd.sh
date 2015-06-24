file=$1

../cnf2aig $file > dd.aig
aigtoaig dd.aig > /dev/null
