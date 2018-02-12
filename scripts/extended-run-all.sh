#!/bin/sh

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

backend[1]="sgl"
backend[2]="stm-norec"
backend[3]="stm-tl2"
backend[4]="stm-swisstm"
backend[5]="stm-tinystm"
backend[6]="htm-sgl"
backend[7]="hybrid-norec-ptr"
backend[8]="hybrid-tl2-ptr"
backend[9]="hybrid-norec-opt"
backend[10]="hybrid-tl2-opt"
backend[11]="seq" # unsafe for threads > 1

benchmarks[1]="stamp"
benchmarks[2]="datastructures"
benchmarks[3]="stmbench7"

htm_retry_policy="0 1 2"
LINEAR=1
HALF=2
ZERO=0
htm_retry_budget="20 10 4 2 1 0"

for a in {1..3};do
   for bench in {1..3};do
      #DO STM
      for be in {2..5};do
        cd $DIR
        bash build-all.sh ${backend[$be]} 1 1
        for t in {1..8};do
           cd $DIR/benchmarks/${benchmarks[$bench]}
           bash run.sh ${backend[$be]} $t $a 1 1
        done
      done

      #NOW DO HTM AND HYBRID
      for be in {6..8};do
         for r in ${htm_retry_policy}; do
            for b in ${htm_retry_budget};do
               # ZERO or HALF with retry budget == 1 are equal to LINEAR with retry budget == 1
               if [ ${r} -ne ${LINEAR} && ${b} -eq 1 ]; then
                  continue;
               fi
               cd $DIR
               bash build-all.sh ${backend[$be]} ${r} ${b} ${b}
               for t in {1..8};do
                  cd $DIR/benchmarks/${benchmarks[$bench]}
                  bash run.sh ${backend[$be]} $t $a ${b} ${r}
              done
         done
       done
      done
   done
done
