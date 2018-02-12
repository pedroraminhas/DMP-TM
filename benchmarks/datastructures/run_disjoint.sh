#!/bin/bash

ulimit -Sv 6000000000
## declare an array variable
declare -a orr=("DMP-SGL-patched") #"DMP-SGL-tune-patched") # "stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt")

export "LD_LIBRARY_PATH=/home/shady/lib/"


for i in "${orr[@]}"
do
    bash build-datastructures.sh $i 1000 1
    cd hashmap-part-not-ordered-cm;
    for writes in 90  
    do  
        for pboth in 0
        do    
            for pbias in 1 #2 5 10 15 50 90 100
            do  
            #    for threads in 1 8 80
            for threads in 1 #8 10 40 80
                do  
                        for rep in 1 2 3 4 5
                        do 
                            #./hashmap $threads $pboth $pbias $writes 1000000
                            #./hashmap $threads $pboth $pbias $writes 1000000
                            echo "$i $threads threads - $pbias pbias - $writes updates - attempt $rep"
                            #./hashmap $threads $pboth $pbias $writes 1000000 > ./results-128/$pboth-pboth-$threads-touchlargedatastructure-$writes-writes-$i-$pbias-$rep.data
                            ./hashmap $threads $pboth $pbias $writes 1000000 > ./results-128/$pboth-pboth-$threads-touchlargedatastructure-$writes-writes-$i-$pbias-$rep.data
                        done
                done
            done
        done
    done
    cd ..; 
done

exit 0

declare -a orr=("")

for i in "${orr[@]}"
do
    bash build-datastructures.sh $i 1000 1
    cd hashmap-part-not-ordered-cm;
    for writes in 90
    do
        for pboth in 0
        do
            for pbias in 1 #2 5 10 15 50 90 100
            do
            #    for threads in 1 8 80
            for threads in 1 8 80
                do
                        for rep in 1 2 3
                        do
                            echo "$i $threads threads - $pbias pbias - $writes updates - attempt $rep"
                            #./hashmap $threads $pboth $pbias $writes 1000000 > ./results-128/$pboth-pboth-$threads-touchlargedatastructure-$writes-writes-$i-$pbias-$rep.data
                        done
                done
            done
        done
    done
    cd ..;
done

echo "test sucessfully concluded"

