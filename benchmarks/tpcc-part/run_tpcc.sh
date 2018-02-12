#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
declare -a irr=("stm-norec")
declare -a orr=("DMP-SGL-tune-patched")

export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${orr[@]}"
do
    bash build-tpcc.sh $i 1000 1
    cd code;
    for threads in 1 2 4 8 16 32 64 80
                do  
                    echo "$i $threads threads bias"
                    #for rep in 1 2 3 4 5 
                    # do  
                    #    ./tpcc -t10 -n$threads -w 1 -s 2 -d 4 -o 0 -p 93  -r 1 > ./results/p93-s2s-d4-r1-1w-$i-$threads-$rep.data
                    #done
                    for rep in 1 2 3  
                    do  
                        ./tpcc -t30 -n$threads -w 1 -s 4 -d 4 -o 4 -p 87  -r 1 > ./results-patched/standard-1w-$i-$threads-$rep.data
                    done       
					#for rep in 1 2 3 
                    #do
                    #    ./tpcc -t10 -n$threads -w 1 -s 1 -d 4 -o 0 -p 95  -r 0 > ./results/p95-1s-4d-1w-$i-$threads-$rep.data
                    #done 
                    #./main $i $j $y $p
        done
    cd ..; 
done
