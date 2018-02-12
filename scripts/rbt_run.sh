#!/bin/sh
BUILD="build-all.sh"

#NB: the name encoding here follows the usual one: this
wait_until_finish() {
    pid3=$1
    echo "process is $pid3"
    LIMIT=30
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

#This has to comply with thread.c in rectm
backend_string_to_int(){
   string=$1
   if [ "${string}" = "stm-tinystm" ];then
      echo "0"
   elif [ "${string}" = "stm-norec" ];then
      echo "1"
   elif [ "${string}" = "stm-tl2" ];then
      echo "2"
   elif [ "${string}" = "stm-swisstm" ];then
      echo "3"
   elif [ "${string}" = "htm-sgl" ];then
      echo "4"
   elif [ "${string}" = "hybrid-norec-ptr" ];then
      echo "5"
   elif [ "${string}" = "hybrid-tl2-ptr" ];then
      echo "6"
   else
      echo "${string} is not a supported backend"
      exit 1
   fi
}

workspace="$PWD"
trace=""
outName=""
#Disable adaptivity
NO_ADAPTIVITY=1

benchmarks[1]="redblacktree"
benchmarks[2]="redblacktree"
benchmarks[3]="redblacktree"

folder[1]="benchmarks/datastructures/redblacktree"
folder[2]="benchmarks/datastructures/redblacktree"
folder[3]="benchmarks/datastructures/redblacktree"

bStr[1]="rbt_i1048576_r1000000_a1_c400000000_u10_o10_z1"
bStr[2]="rbt_i1048576_r1000000_a1_c400000000_u5_o1_z1"
bStr[3]="rbt_i1048576_r1000000_a1_c400000000_u95_o10_z0.00001"

params[1]="-d 1000000 -i 1048576 -r 1000000 -a1 -c 1000000 -u 10 -o 10 -z 1 -n"
params[2]="-d 1000000 -i 1048576 -r 1000000 -a1 -c 1000000 -u 5 -o 1 -z 1 -n"
params[3]="-d 1000000 -i 1048576 -r 1000000 -a1 -c 1000000 -u 95 -o 10 -z 0.00001 -n"


#Define backends (config), locking schemes (alias), benchmarks and their parameters

backend[1]="stm-norec"
backend[2]="stm-tl2"
backend[3]="stm-swisstm"
backend[4]="stm-tinystm"
backend[5]="htm-sgl"
backend[6]="hybrid-norec-ptr"
backend[7]="hybrid-tl2-ptr"


htm_retry_policy="0 1 2"
htm_retry_budget="20 10 4 2 1"
LINEAR=1
HALF=2
ZERO=0


#Number of runs, to mask failed runs and perform averages
for a in {1..5};do
#Benchmarks
for b in {1..3};do
#Backends
 for c in 1 2 3 4 5;do
      #Setup HTM if necessary
      if [ ${c} -ge 5 ];then
         echo "HTM with adaptive retry"
         for fb in ${htm_retry_budget};do
            for lck in ${htm_retry_policy};do
               # ZERO or HALF with retry budget == 1 are equal to LINEAR with retry budget == 1
               if [ "${r}" -ne "${HALF}" -a "${b}" -eq "1" ];then
                  continue;
               fi
               cd $workspace;
               echo "Going to traduce ${backend[$c]} to int"
               int_backend=$(backend_string_to_int ${backend[$c]})
               bash ${BUILD} greentm ${fb} ${lck} ${NO_ADAPTIVITY} ${int_backend}
               cd ${folder[$b]};
               #Thread number
               for t in {1..8};do
                  trace="greentm_${backend[$c]} | ${bStr[$b]} | threads $t | attempt $a  | budget ${fb} | retry_policy ${lck} | noadaptivity ${NO_ADAPTIVITY} | intBackend ${int_backend}"
                  outName="greentm_${backend[$c]}=${bStr[$b]}=th${t}=run${a}=budget${fb}=rh${lck}=noadaptivity${NO_ADAPTIVITY}=intBackend${int_backend}"
                  echo "${PWD}"
                  echo "Launching ${benchmarks[$b]} ${params[$b]}$t with ${trace}"
                  ./${benchmarks[$b]} ${params[$b]}$t > ${workspace}/auto-results/${outName}.data &
                  pid=$!
                  wait_until_finish $pid
                  wait $pid
                  rc=$?
                  if [ $rc -eq 13 ] ; then
                          echo "Error within: ${trace}" >> ../../../auto-results/slow.out
                      elif [[ $rc != 0 ]] ; then
                          echo "Error within: ${trace}" >> ../../../auto-results/error.out
                  fi
               done    #t
            done  #lck
         done  #fb
      else
         cd $workspace;
         echo "Going to traduce ${backend[$c]} to int"
         int_backend=$(backend_string_to_int ${backend[$c]})
         bash ${BUILD} greentm 1 1 ${NO_ADAPTIVITY} ${int_backend}
         cd ${folder[$b]};
         for t in {1..8};do
            trace="greentm_${backend[$c]} | ${bStr[$b]} | threads $t | attempt $a  | budget 1 | retry_policy 1 | noadaptivity ${NO_ADAPTIVITY} | intBackend ${int_backend}"
            outName="greentm_${backend[$c]}=${bStr[$b]}=th${t}=run${a}=budget1=rh1=noadaptivity${NO_ADAPTIVITY}=intBackend${int_backend}"
            echo "${PWD}"
            echo "Launching ${benchmarks[$b]} ${params[$b]}$t with ${trace}"
            ./${benchmarks[$b]} ${params[$b]}$t > ${workspace}/auto-results/${outName}.data &
            pid=$!
            wait_until_finish $pid
            wait $pid
            rc=$?
            if [ $rc -eq 13 ] ; then
                    echo "Error within: ${trace}" >> ../../../auto-results/slow.out
                elif [[ $rc != 0 ]] ; then
                    echo "Error within: ${trace}" >> ../../../auto-results/error.out
                fi
         done     #t
      fi
        done
    done
done
