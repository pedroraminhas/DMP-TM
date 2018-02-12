#!/bin/sh
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

workspace="$PWD"
trace=""
outName=""

#bash generate_bench_list.sh
source bench_list.sh



#Define backends (config), locking schemes (alias), benchmarks and their parameters

backend[1]="stm-norec"
backend[2]="stm-tl2"
backend[3]="stm-swisstm"
backend[4]="stm-tinystm"
backend[5]="htm-sgl"
backend[6]="hybrid-norec-ptr"
backend[7]="hybrid-tl2-ptr"


htm_retry_policy="0 1 2"
htm_retry_budget="20 10 4 2 1 0"
LINEAR=1
HALF=2
ZERO=0


#Number of runs, to mask failed runs and perform averages
for a in {1..5};do
#Backends
 for c in {1..7};do
      #Benchmarks
      for b in {1..46};do
      #Setup HTM if necessary
      if [ ${c} -ge 5 ];then
         echo "HTM with adaptive retry"
         for fb in ${htm_retry_budget}; do
            for lck in ${htm_retry_policy}; do
               # ZERO or HALF with retry budget == 1 are equal to LINEAR with retry budget == 1
               if [ ${r} -ne ${LINEAR} && ${b} -eq 1 ]; then
                  continue;
               fi
               cd $workspace;
               bash build-all.sh ${backend[$c]} ${fb} ${lck}
                cd ${folder[$b]};
               #Thread number
               for t in {1..8};do
                  trace="${backend[$c]} | ${bStr[$b]} | threads $t | attempt $a  | budget ${fb} | retry_policy ${lck}"
                  outName="${backend[$c]}-${bStr[$b]}-th${t}-run${a}-budget${fb}-rh${lck}"
                  echo "Launching ${benchmarks[$b]} ${params[$b]}$t"
                  ./${benchmarks[$b]} ${params[$b]}$t > ${workspace}/auto-results/${outName}.data &
                  pid=$!
                  wait_until_finish $pid
                  wait $pid
                  rc=$?
                  if [[ $rc != 0 ]] ; then
                     echo "Error within: ${trace}" >> ${workspace}/auto-results/error.out
                  fi
               done    #t
            done  #lck
         done  #fb
      else
         cd $workspace;
         bash build-all.sh ${backend[$c]} 1 1
         cd ${folder[$b]};
         for t in {1..8};do
            trace="${backend[$c]} | ${bStr[$b]} | threads $t | attempt $a"
            outName="${backend[$c]}-${bStr[$b]}-th${t}-run${a}"
            echo "Launching ${benchmarks[$b]} ${params[$b]}$t"
            ./${benchmarks[$b]} ${params[$b]}$t > ${workspace}/auto-results/${outName}.data &
            pid=$!
            wait_until_finish $pid
            wait $pid
            rc=$?
            if [[ $rc != 0 ]] ; then
               echo "Error within: ${trace}" >> ${workspace}/auto-results/error.out
            fi
         done     #t
      fi
        done
    done
done
