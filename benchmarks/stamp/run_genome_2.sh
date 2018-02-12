#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
export "LD_LIBRARY_PATH=/home/shady/lib/"

#declare -a irr=("stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt" "hybrid-lsa")
#for i in "${irr[@]}"
#do
#    bash build-stamp.sh $i 10 1
#    cd genome2;
    #for threads in 1 2 4 8 16 32 64 80
#    for threads in 1 2 4 8 16 32 64 80
#                do
#					for rep in 1 2 3 4 5
#                    do  
#                        echo "$i $threads threads $rep rep genome"
#                         ./genome -g65436 -s300 -n10777216 -r1 -t$threads> ../results-2/genome-$i-$threads-$rep.data
#                    done
                    #./main $i $j $y $p
#        done
#    cd ..;
#done

declare -a irr=("DMP-SGL-cm-patched")
for i in "${irr[@]}"
do
    bash build-stamp.sh $i 100 1
    cd genome2;
    for threads in 64 80 #1 2 4 8 16 32 64 80
    #for threads in 80
                do
                    for rep in 1 2 3 
                    do
                        echo "$i $threads threads $rep rep genome"
                         ./genome -g65436 -s300 -n10777216 -r1 -t$threads> ../results-2/genome-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..;
done
