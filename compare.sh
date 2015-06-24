#!/bin/bash
riss3g $1 
R1=$?
Default/cnf2aig $1 | aigtocnf | riss3g
R2=$?
if [ $R1 == $R2 ];
then
    echo "ok"
    exit 10
fi
echo "nok"
exit 20

