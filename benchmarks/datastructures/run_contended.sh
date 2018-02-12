#!/bin/bash

ulimit -Sv 6000000000
## declare an array variable
#declare -a orr=("htm-sgl" "tinystm" "hybrid-norec-opt" "hybrid-lsa" "hybrid-tl2-opt" "stm-norec" "hybrid-norec-opt-suspend")
declare -a orr=("DMP-SGL-tune-patched")

export "LD_LIBRARY_PATH=/home/shady/lib/"


for i in "${orr[@]}"
do
    bash build-datastructures.sh $i 1000 1
    cd contended-hashmap-sched;
    for writes in 90  
    do  
        for pboth in 0
        do    
            for pbias in 1 2 5 10 #15 50 90 100
            do  
            #    for threads in 1 8 80
            for threads in 1 8 10 40 80
                do  
                    for sched in 0 #1 10 100 1000 10000 1000000
                    do

                        for rep in 1 2 3 
                        do 
                            echo "$i $threads threads - $sched sched - $pbias pbias - $writes updates - attempt $rep"
                            ./hashmap $threads $pboth $pbias $writes 1000000 $sched > ./results-128/$pboth-pboth-$threads-touchlargedatastructure-$writes-writes-$i-$sched-$pbias-$rep.data
                        done
                    done
                done
            done
        done
    done
    cd ..; 
done

declare -a orr=()
declare -a orr=("htm-sgl" "tinystm" "stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt")
export "LD_LIBRARY_PATH=/home/shady/lib/"


#for i in "${orr[@]}"
#do
#    bash build-datastructures.sh $i 10 1
#    cd contended-hashmap-sched;
#    for writes in 90  
#    do  
#        for pboth in 0
#        do    
#            for pbias in 1 #2 5 10 15 50 90 100
#            do  
            #    for threads in 1 8 80
#            for threads in 1 8 80
#                do  
#                    for sched in 0 #1 10 100 1000 10000 1000000
#                    do
#
#                        for rep in 1 2 3 
#                        do 
#                            echo "$i $threads threads - $sched sched - $pbias pbias - $writes updates - attempt $rep"
#                            ./hashmap $threads $pboth $pbias $writes 1000000 $sched > ./results-128/$pboth-pboth-$threads-touchlargedatastructure-$writes-writes-$i-$sched-$pbias-$rep.data
#                        done
#                    done
#                done
#done
#        done
#    done
#    cd ..; 
#done

echo "test sucessfully concluded"


