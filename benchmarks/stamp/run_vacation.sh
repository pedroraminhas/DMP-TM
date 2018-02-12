#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
declare -a irr=("htm-sgl" "tinystm" "DMP-SGL" "stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt")

export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${irr[@]}"
do
    bash build-stamp.sh $i 100 1
    cd vacation;
    for threads in 1 2 4 8 16 32 64 80
                do
                    for rep in 1 2 3 4 5 
                    do
                        echo "$i $threads threads $rep rep vacation"
                        timeout -s SIGKILL 3m  ./vacation  -n2 -q90 -u98 -r1048576 -t4194304 -a1 -c$threads> ../results/low-contention-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..;
done


for i in "${irr[@]}"
do
    bash build-stamp.sh $i 100 1
    cd vacation;
   for threads in 1 2 4 8 16 32 64 80
                do  
                    for rep in 1 2 3 4 5 
                    do  
                        echo "$i $threads threads $rep rep vacation"
                        timeout -s SIGKILL 3m ./vacation -n4 -q60 -u90 -r1048576 -t4194304 -a1 -c$threads> ../results/high-contention-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..; 
done

for i in "${irr[@]}"
do
    bash build-stamp.sh $i 100 1
    cd vacation-noclient;
    for threads in 1 2 4 8 16 32 64 80
                do  
                    for rep in 1 2 3 4 5 
                    do  
                         echo "$i $threads threads $rep rep vacation"
                        timeout -s SIGKILL 3m  ./vacation  -n2 -q90 -u98 -r1048576 -t4194304 -a1 -c$threads> ../results/low-contention-noclient-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..; 
done


for i in "${irr[@]}"
do
    bash build-stamp.sh $i 100 1
    cd vacation-noclient;
   for threads in 1 2 4 8 16 32 64 80
                do  
                    for rep in 1 2 3 4 5 
                    do  
                         echo "$i $threads threads $rep rep vacation"
                        timeout -s SIGKILL 3m ./vacation -n4 -q60 -u90 -r1048576 -t4194304 -a1 -c$threads> ../results/high-contention-noclient-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..; 
done
#for i in "${orr[@]}"
#do
#for j in 1 2 3 4 5 6 7 
#do
#    bash build-stamp.sh $i 10 1
#    cd vacation$j;
#    for threads in 1 2 4 8 16 32 64 80
#                do  
#                    echo "$i $threads brute-force"
#                    for rep in 1 2 3 4 5 
#                   do  
#                        timeout -s SIGKILL 6m ./vacation  -n2 -q90 -u98 -r1048576 -t4194304 -a1 -c$threads> ../vacation/results/low-contention-$i-$j-$threads-$rep.data
#                        timeout -s SIGKILL 6m ./vacation -n4 -q60 -u90 -r1048576 -t4194304 -a1 -c$threads> ../vacation/results/high-contention-$i-$j-$threads-$rep.data
#                    done
                    #./main $i $j $y $p
#        done
#    cd ..; 
#done
#done

