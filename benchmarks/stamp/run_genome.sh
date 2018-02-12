#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
declare -a irr=("htm-sgl" "tinystm" "DMP-SGL" "stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt")
#declare -a irr=("hybrid-norec-opt-suspend"  "hybrid-norec-opt")
declare -a irr=("DMP-SGL")
declare -a irr=("stm-norec" "hybrid-norec-opt")
export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${irr[@]}"
do
    bash build-stamp.sh $i 10 1
    cd genome;
    #for threads in 1 2 4 8 16 32 64 80
    for threads in 1 2 4 8 180
                do
					for rep in 1 2 3 4 5
                    do  
                        echo "$i $threads threads $rep rep genome"
                         ./genome -g65436 -s64 -n16777216 -r1 -t$threads> ../results/genome-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..;
done


