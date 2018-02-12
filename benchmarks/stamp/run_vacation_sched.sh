#!/bin/bash

#ulimit -Sv 6000000000
## declare an array variable
declare -a irr=("DMP-SGL-sched") # "stm-norec" "hybrid-norec-opt" "hybrid-norec-opt-suspend" "hybrid-tl2-opt")

export "LD_LIBRARY_PATH=/home/shady/lib/"



for i in "${irr[@]}"
do
    bash build-stamp.sh $i 1000 1
    cd vacation;
    for threads in 1 2 4 8 16 32 64 80
                do
                    for rep in 1 2 3 
                    do
                        echo "$i $threads threads $rep rep vacation"
                        #timeout -s SIGKILL 30s  ./vacation  -n2 -q60 -u10 -r2000 -t10310720 -a1 -c$threads> ../results/vacation-sched-$i-$threads-$rep.data
                        #./vacation -n4 -q90 -u0 -r500 -t4194304 -a1 -c$threads > ../results-standard/vacation-sched-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..;
done

declare -a irr=("htm-sgl" "tinystm")

for i in "${irr[@]}"
do
    bash build-stamp.sh $i 10 1
    cd vacation;
    for threads in 1 2 4 8 16 32 64 80
                do
                    for rep in 1 2 3
                    do
                        echo "$i $threads threads $rep rep vacation"
                        #timeout -s SIGKILL 30s  ./vacation  -n2 -q60 -u10 -r2000 -t10310720 -a1 -c$threads> ../results/vacation-sched-$i-$threads-$rep.data
                        ./vacation -n4 -q90 -u0 -r500 -t4194304 -a1 -c$threads > ../results-standard/vacation-sched-$i-$threads-$rep.data
                    done
                    #./main $i $j $y $p
        done
    cd ..;
done
