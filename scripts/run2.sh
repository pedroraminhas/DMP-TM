#!/bin/bash

ulimit -Sv 6000000000
## declare an array variable
declare -a arr=("hybrid-partition-STM-SGL")
declare -a err=("tinystm")
declare -a orr=("DMP-STM-cm" "DMP-STM")
#declare -a orr=("htm-sgl" "tinystm" "stm-norec" "hybrid-norec-opt" "DMP-SGL" "DMP-STM" "DMP-SGL-cm")

export "LD_LIBRARY_PATH=/home/shady/lib/"


for i in "${orr[@]}"
do
    bash build-datastructures.sh $i 10 1 
    cd hashmap-part-not-ordered-cm;
    for writes in 90  
    do  
        for pboth in 0
        do    
            for pbias in 0 1 2 5 10 50 100
            do  
                for threads in 1 2 4 8 16 32 64 80
                do  
                    echo "$i $threads threads - $pboth pboth - $pbias pbias - $writes second test"
                    #echo "$threads threads" >>./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    for rep in 1 2 3 4  5
                    do  
                        ./hashmap $threads $pboth $pbias $writes "1000000" > ./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i-$threads-$rep.data
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

for i in "${err[@]}"
do
    bash build-datastructures.sh $i 10 1 
    cd hashmap-part-not-ordered-cm;
    for writes in 90  
    do  
        for pboth in 0
        do    
            for pbias in 100 
            do  
                for threads in 1 2 4 8 16 32 64 80
                do  
                    echo "$i $threads threads - $pboth pboth - $pbias pbias - $writes second test"
                    #echo "$threads threads" >>./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i.txt
                    for rep in 1 2 3 4  5
                    do  
                        ./hashmap $threads $pboth $pbias $writes "1000000" > ./results/$pboth-pboth-$pbias-touchlargedatastructure-$writes-writes-$i-$threads-$rep.data
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

