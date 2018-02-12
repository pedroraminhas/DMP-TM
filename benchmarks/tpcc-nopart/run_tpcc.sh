#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
declare -a orr=("htm-sgl" "tinystm")
declare -a irr=("hybrid-norec-opt")

export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${orr[@]}"
do
    bash build-tpcc.sh $i 100 1
    cd code;
    for threads in 1 2 4 8 16 32 64 80
                do  
                    echo "$i $threads threads bias"
                    for rep in 1 2 3 4 5
                    do  
                        ./tpcc -t10 -n$threads -w 10 -m 10 -s 4 -d 0 -o 4 -p 92  -r 0 >  ./results/nopart-92p-10w-$i-$threads-$rep.data
                        ./tpcc -t10 -n$threads -w 1 -m 1 -s 4 -d 4 -o 4 -p 83 -r 5 >  ./results/nopart-83p-10w-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..; 
done

