#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
declare -a orr=("DMP-HTM-cm" "tinystm" "htm-sgl")
declare -a irr=("stm-norec")

export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${orr[@]}"
do
    bash build-tpcc.sh $i 10 1
    cd code;
#    for threads in 1 2 4 8 16 32 64 80
    for threads in  1 2 4 8 16 32 64 80
                do  
                    echo "$i $threads threads bias"
                    for rep in 1 2 3 4 5 
                    do  
                        ./tpcc -t60 -n$threads -w1 -s 100 -d 0 -o 0 -p 0 -r 0 >  ./results/100s-$i-$threads-$rep.data
                        ./tpcc -t60 -n$threads -w1 -s 0 -d 100 -o 0 -p 0 -r 0 >  ./results/100d-$i-$threads-$rep.data
						./tpcc -t60 -n$threads -w1 -s 0 -d 0 -o 100 -p 0 -r 0 >  ./results/100o-$i-$threads-$rep.data
						./tpcc -t60 -n$threads -w1 -s 0 -d 0 -o 0 -p 100 -r 0 >  ./results/100p-$i-$threads-$rep.data
						./tpcc -t60 -n$threads -w1 -s 0 -d 0 -o 0 -p 0 -r 100 >  ./results/100pr-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..; 
done

