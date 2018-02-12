#!/bin/sh

OUT=${PWD}/bench_list.sh
rm ${OUT}
touch ${OUT}

echo ${OUT}

INIT=1
cd benchmarks/datastructures/hashmap
echo "hashmap starting from ${INIT}"
INIT=$(bash generate_hashmap.sh ${INIT} ${OUT})

cd ../skiplist
echo "skiplist starting from ${INIT}"
INIT=$(bash generate_sl.sh ${INIT} ${OUT})

cd ../redblacktree
echo "rbt starting from ${INIT}"
INIT=$(bash generate_rbt.sh ${INIT} ${OUT})


cd ../linkedlist
echo "linkedlist starting from ${INIT}"
INIT=$(bash generate_ll.sh ${INIT} ${OUT})


cd ../../stamp/genome
echo "genome starting from ${INIT}"
INIT=$(bash generate_genome.sh ${INIT} ${OUT})


cd ../intruder
echo "intruder starting from ${INIT}"
INIT=$(bash generate_intruder.sh ${INIT} ${OUT})


cd ../ssca2
echo "ssca2 starting from ${INIT}"
INIT=$(bash generate_ssca2.sh ${INIT} ${OUT})


cd ../vacation
echo "vacation starting from ${INIT}"
INIT=$(bash generate_vacation.sh ${INIT} ${OUT})


cd ../../stmbench7
echo "stmb7 starting from ${INIT}"
INIT=$(bash generate_stmb7.sh ${INIT} ${OUT})


cd ../stamp/labyrinth
echo "labyrinth starting from ${INIT}"
INIT=$(bash generate_labyrinth.sh ${INIT} ${OUT})



