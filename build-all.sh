#!/bin/sh

backend=$1  # e.g.: greentm
htm_retries=$2 # e.g.: 5
htm_capacity_abort_strategy=$3 # e.g.: 0 for "give up"
adaptivity_disabled=$4 # e.g.: 1 to disable
adaptivity_starting_mode=$5 # e.g.: 3 for SwissTM

bash clean-all.sh;

cd stms;
#cd norec;
#make;
cd tinystm;
make;
#cd ../tl2;
#make;
#cd ../swisstm;
#make;
echo '--------------------GOING TO BENCHMARK----------------------------------------------------'
cd ../../
cd benchmarks/datastructures
bash build-datastructures.sh $backend $htm_retries $htm_capacity_abort_strategy $adaptivity_disabled $adaptivity_starting_mode
#cd ../stamp
#bash build-stamp.sh $backend $htm_retries $htm_capacity_abort_strategy $adaptivity_disabled $adaptivity_starting_mode
#cd ../stmbench7
#bash build-stmbench7.sh $backend $htm_retries $htm_capacity_abort_strategy $adaptivity_disabled $adaptivity_starting_mode
