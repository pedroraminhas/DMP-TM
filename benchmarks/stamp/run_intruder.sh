#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
#declare -a irr=("htm-sgl" "tinystm" "DMP-SGL" "stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt")
#declare -a irr=("hybrid-tl2-opt" "hybrid-norec-opt" "hybrid-norec-opt-suspend")
declare -a irr=("DMP-SGL-patched")
export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${irr[@]}"
do
    bash build-stamp.sh $i 1000 1
    cd intruder;
    for threads in 1 2 4 8 16 32 64 80
                do
					for rep in 1 2 3
                    do  
                        echo "$i $threads threads $rep rep intruder"
                        timeout -s SIGKILL 5m ./intruder  -a10 -l128 -n262144 -s1 -r1 -t$threads> ../results-2/intruder-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..;
done


