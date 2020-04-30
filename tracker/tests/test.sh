#!/bin/bash

#On fait écouter le tracker sur le port 8500
./../build/tracker $2 &

test__announce()
{
    a1=`echo "announce listen 10000 seed [test0.txt 1200 3 az85er12ty46]" | ./../../a.out $1 $2`
    a2=`echo "announce listen 12000 seed [test0.txt 1200 3 az85er12ty46]" | ./../../a.out $1 $2`
    a3=`echo "announce listen 10000 seed [test1.txt 1000 3 az85er13ty46]" | ./../../a.out $1 $2`
    a4=`echo "announce listen 10000 seed [test0.txt 1200 3 az85er12ty46]" | ./../../a.out $1 $2`
    if [ $a1 = "Please enter the message: > ok" ]
    then
        echo "SUCCESS"
    fi
}
test__announce $1 $2
