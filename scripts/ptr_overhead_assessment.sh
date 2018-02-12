#!/bin/sh

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

backend[1]="sgl"
backend[2]="stm-norec"
backend[3]="stm-tl2"
backend[4]="stm-swisstm"
backend[5]="stm-tinystm"
backend[6]="htm-sgl"
backend[7]="hybrid-norec-opt"
backend[8]="hybrid-tl2-opt"

backend[10]="greentm"

backend[17]="hybrid-norec-ptr"
backend[18]="hybrid-tl2-ptr"

backend[20]="seq" # unsafe for threads > 1

benchmarks[1]="stamp"
benchmarks[2]="datastructures"
benchmarks[3]="stmbench7"

#bash clean-all.sh;

cd stms;
cd norec;
make;
cd ../tinystm;
make;
cd ../tl2;
make;
cd ../swisstm;
make;
cd ../../


#first run greentm with the 5 backends
bash greentm_run_all.sh

for bench in {1..3}
do
    for be in {2..6} # run norec, tl2, swiss, tiny, htm-sgl
    do
        cd $DIR
        # in the next command we invoke with parameters: backend, htm retries, htm retry policy, disable adaptivity, tm starting algorithm
        retries=5
        bash build-all.sh ${backend[$be]} $retries 5 1 1 1
        for t in 1 4 8 #{1..8}
        do
            for a in 1 #{1..3}
            do
                cd $DIR/benchmarks/${benchmarks[$bench]}
                bash run.sh ${backend[$be]} $t $a $retries
            done
        done
    done
done
