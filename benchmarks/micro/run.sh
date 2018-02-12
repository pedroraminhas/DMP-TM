#!/bin/sh

benchmarks[1]="synth"

bStr[1]="synth-r"

params[1]=""

wait_until_finish() {
    pid3=$1
    echo "process is $pid3"
    LIMIT=180
    for ((j = 0; j < $LIMIT; ++j)); do
        kill -s 0 $pid3
        rc=$?
        if [[ $rc != 0 ]] ; then
            echo "returning"
            return;
        fi
        sleep 1s
    done
    kill $pid3
}

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

for b in {1..1}
do
    cd $DIR;
    cd ${benchmarks[$b]};
    echo "${bStr[$b]} | backend $1 | htm_retries $4 | threads $2 | attempt $3 | retry_policy $4"
    ./${benchmarks[$b]} ${params[$b]}$2 > ../../../auto-results/$1-${bStr[$b]}-retries$4-th$2-att$3.data &
    pid3=$!; wait_until_finish $pid3; wait $pid3; rc=$?
    if [ $rc -eq 13 ] ; then
        echo "Error within: backend $1 | ${bStr[$b]} | htm_retries $4 | threads $2 | attempt $3" >> ../../../auto-results/slow.out
    elif [[ $rc != 0 ]] ; then
        echo "Error within: backend $1 | ${bStr[$b]} | htm_retries $4 | threads $2 | attempt $3" >> ../../../auto-results/error.out
    fi
done
