#!/bin/bash

workspace=$1
if [ $4 == "no_random" ];then
resultsdir=$2
runsdir=$resultsdir/runs
else
resultsdir=$2/$RANDOM
mkdir -p $resultsdir/runs
runsdir=$resultsdir/runs
mkdir $resultsdir/plots
mkdir $resultsdir/summary
echo $3 > $resultsdir/desc.txt
fi

export "LD_LIBRARY_PATH=/home/shady/lib/"
export STM_STATS=True

htm_retry_policy="2"
htm_retry_budget="5"

backends[3]="htm-sgl"
backends[1]="stm-norec"
backends[2]="stm-tinystm"
backends[4]="hybrid-norec-opt"
backends[5]="hybrid-norec-ptr"
backends[6]="hybrid-norec-sr-opt"
backends[7]="hybrid-norec-sr-noop-opt"

benchmarks[2]="labyrinth"
benchmarks[1]="genome"
benchmarks[3]="intruder"
benchmarks[4]="kmeans"
benchmarks[5]="ssca2"
benchmarks[6]="vacation"
benchmarks[7]="yada"
benchmarks[8]="kmeans"
benchmarks[9]="vacation"

bStr[2]="labyrinth"
bStr[1]="genome"
bStr[3]="intruder"
bStr[4]="kmeans-high"
bStr[5]="ssca2"
bStr[6]="vacation-high"
bStr[7]="yada"
bStr[8]="kmeans-low"
bStr[9]="vacation-low"

comb[1]="-c3 -a10 -l128 -n262144 -c2 -a20 -l128 -n20000"

params[2]="-i inputs/random-x512-y512-z7-n512.txt -t"
params[1]="-g16384 -s64 -n16777216 -r1 -t"
params[3]="${comb[1]} -s1 -r5 -t"
params[4]="-m15 -n15 -t0.00001 -i inputs/random-n65536-d32-c16.txt -p"
params[5]="-s20 -i1.0 -u1.0 -l3 -p3 -r 1 -t"
params[6]="-n4 -q60 -u90 -r1048576 -t4194304 -a 1 -c"
params[7]="-a15 -i inputs/ttimeu1000000.2 -r 1 -t"
params[8]="-m40 -n40 -t0.00001 -i inputs/random-n65536-d32-c16.txt -p"
params[9]="-n2 -q90 -u98 -r1048576 -t4194304 -a 1 -c"

wait_until_finish() {
    pid3=$1
    echo "process is $pid3"
    LIMIT=300
    for ((j = 0; j < $LIMIT; ++j)); do
        kill -s 0 $pid3
        rc=$?
        if [[ $rc != 0 ]] ; then
            echo "returning"
            return;
        fi
        sleep 1s
    done
    kill -9 $pid3
}

for c in 7
do
	for r in $htm_retry_budget
	do
		for p in $htm_retry_policy
		do
			cd $workspace;
			bash build-all.sh ${backends[$c]} $r $p 1 3
			cd benchmarks/stamp
			for b in 1 5 7 #4 6 8 9 #1 2 3 4 5 6 7 8 9 #11 12 13 15 16 17 18 19 20
			do
				cd ${benchmarks[$b]};
        			for t in 1 4 8 16 32 64 80
       				do
            				for a in 1 2 3 #4 5 #6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
            				do
                				echo "${benchmarks[$b]} | ${backends[$c]}-$r-$p | threads $t | attempt $a"
                				./${benchmarks[$b]} ${params[$b]}$t > $runsdir/${bStr[$b]}-${backends[$c]}-$r-$p-$t-$a.data 2> $runsdir/${bStr[$b]}-${backends[$c]}-$r-$p-$t-$a.err &
						pid3=$!
						wait_until_finish $pid3
						wait $pid3
                				rc=$?
                				if [[ $rc != 0 ]] ; then
                    					echo "Error within: | ${benchmarks[$b]} | ${backends[$c]}-$r-$p | threads $t | attempt $a" >> $runsdir/error.out
               					fi
	    				done
        			done
				cd ..
			done
		done
	done
done

exit 0

for c in 1 2
do
	cd $workspace;
        bash build-all.sh ${backends[$c]} $r $p
        cd benchmarks/stamp
        for b in 1 2 5 7 #4 6 8 9 #1 2 3 4 5 6 7 8 9 #11 12 13 15 16 17 18 19 20
        do
        	cd ${benchmarks[$b]};
                for t in 1 4 8 16 32 64 80
                do
                	for a in 1 2 3 #4 5 #6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
                        do
                        	echo "${benchmarks[$b]} | ${backends[$c]} | threads $t | attempt $a"
                                ./${benchmarks[$b]} ${params[$b]}$t > $runsdir/${bStr[$b]}-${backends[$c]}-$t-$a.data 2> $runsdir/${bStr[$b]}-${backends[$c]}-$t-$a.err &
                                pid3=$!
                                wait_until_finish $pid3
                                wait $pid3
                                rc=$?
                                if [[ $rc != 0 ]] ; then
                                	echo "Error within: | ${benchmarks[$b]} | ${backends[$c]} | threads $t | attempt $a" >> $runsdir/error.out
                                fi
                        done
                done
		cd ..
        done
done

echo $resultsdir

