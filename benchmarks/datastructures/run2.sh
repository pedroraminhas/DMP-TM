#!/bin/bash

ulimit -Sv 6000000000
## declare an array variable
declare -a err=("htm-sgl" "DMP-SGL" "tinystm" "hybrid-norec-opt" "hybrid-lsa" "hybrid-tl2-opt" "stm-norec" "hybrid-norec-opt-suspend")

export "LD_LIBRARY_PATH=/home/shady/lib/"

for i in "${err[@]}"
do
    bash build-datastructures.sh $i 10 1 
    cd hashmap-part-not-ordered-cm;
    for writes in 90  
    do  
        for pboth in 0
        do    
            for pbias in 90
            do  
                for threads in 10 40
                do  
                    echo "$i $threads threads - $pboth pboth - $pbias pbias - $writes second test"
                    #echo "$threads threads" >>./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    ./hashmap $threads $pboth $pbias $writes "16000000"
                    ./hashmap $threads $pboth $pbias $writes "16000000"
                    for rep in 1 2 3 4 5
                    do  
                        ./hashmap $threads $pboth $pbias $writes "16000000" > ./results-disjoint/$pboth-pboth-$threads-touchlargedatastructure-$writes-writes-$i-$pbias-$rep.data
                    #./hashmap $threads $pboth $pbias $writes "1000000">> ./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    #./hashmap $threads $pboth $pbias $writes "1000000">> ./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    #./hashmap $threads $pboth $pbias $writes "1000000">> ./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    #./hashmap $threads $pboth $pbias $writes "1000000">> ./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    #echo "-----------" >>./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    #./main $i $j $y $p
                    done
                done
            done
        done
    done
    cd ..; 
done


echo "test sucessfully concluded"

