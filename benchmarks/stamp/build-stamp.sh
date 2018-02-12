#!/bin/sh
#FOLDERS="genome intruder vacation ssca2 kmeans yada"
#FOLDERS="vacation-sched intruder genome kmeans vacation vacation-noclient" #intruder genome kmeans"
#FOLDERS="vacation"
FOLDERS="genome2 intruder" #intruder" #genome2 genome intruder"

if [ $# -eq 0 ] ; then
    echo " === ERROR At the very least, we need the backend name in the first parameter. === "
    exit 1
fi

backend=$1  # e.g.: herwl



if [ $# -eq 3 ] ; then
    fast_htm_retries=$2 # e.g.: 5
    slow_htm_retries=$3 # e.g.: 2, this can also be retry policy for tle
fi

rm lib/*.o || true

rm Defines.common.mk
rm Makefile
rm Makefile.flags
rm lib/thread.h
rm lib/thread.c
rm lib/tm.h


cp ../../backends/$backend/Defines.common.mk .
cp ../../backends/$backend/Makefile .
cp ../../backends/$backend/Makefile.flags .
cp ../../backends/$backend/thread.h lib/
cp ../../backends/$backend/thread.c lib/
cp ../../backends/$backend/tm.h lib/


for F in $FOLDERS
do
    cd $F
    rm *.o || true
    rm $F
		make_command="make -f Makefile HTM_RETRIES=-DHTM_RETRIES=$fast_htm_retries FAST_HTM_RETRIES=-DFAST_HTM_RETRIES=$fast_htm_retries RETRY_POLICY=-DRETRY_POLICY=$slow_htm_retries SLOW_HTM_RETRIES=-DSLOW_HTM_RETRIES=$slow_htm_retries"
    $make_command
    rc=$?
    if [[ $rc != 0 ]] ; then
        echo ""
        echo "=================================== ERROR BUILDING $F - $name ===================================="
        echo ""
        exit 1
    fi
    cd ..
done

