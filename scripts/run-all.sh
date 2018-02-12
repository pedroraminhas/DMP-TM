#!/bin/sh

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

backend[0]="tinystm"
backend[1]="norec"
backend[2]="tl2"
backend[3]="swisstm"
backend[4]="htm"
backend[5]="hybrid-norec"
backend[6]="hybrid-tl2"

backend[10]="greentm"

backend[20]="seq" # unsafe for threads > 1

benchmarks[1]="stamp"
benchmarks[2]="datastructures"
benchmarks[3]="stmbench7"
benchmarks[4]="memcached-gcc"
benchmarks[5]="tpcc"

for a in {1..10}
do
for bench in {5..5}
do
    for be in 0 3 4
    do
        cd $DIR
        # in the next command we invoke with parameters: backend, htm retries, htm retry policy, disable adaptivity, tm starting algorithm
        retries=16
        # bash build-all.sh ${backend[$be]} $retries 5 2 0 0
        for t in {1..8}
        do
                cd $DIR/benchmarks/${benchmarks[$bench]}
                bash run.sh ${backend[$be]} $t $a $retries 1 $be
                # bash run.sh ${backend[$be]} $t $a $retries
        done
    done
done
done
