#!/bin/sh
FOLDERS="synth"

if [ $# -eq 0 ] ; then
    echo " === ERROR At the very least, we need the backend name in the first parameter. === "
    exit 1
fi

backend=$1  # e.g.: greentm

htm_retries=5
htm_capacity_abort_strategy=2
adaptivity_disabled=0
adaptivity_starting_mode=0
if [ $# -eq 5 ] ; then
    htm_retries=$2 # e.g.: 5
    htm_capacity_abort_strategy=$3 # e.g.: 0 for "give up"
    adaptivity_disabled=$4 # e.g.: 1 to disable
    adaptivity_starting_mode=$5 # e.g.: 3 for SwissTM
fi

rm lib/*.o || true

rm Defines.common.mk
rm Makefile
rm Makefile.flags
rm lib/thread.h
rm lib/thread.c
rm lib/tm.h
rm lib/rectm.h
rm lib/workload.h

cp ../../backends/greentm/workload.h lib/
cp ../../backends/greentm/rectm.h lib/
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
    if [ $# -eq 1 ] ; then
        make -f Makefile
    else
        make_command="make -f Makefile HTM_RETRIES=-DHTM_RETRIES=$htm_retries RETRY_POLICY=-DRETRY_POLICY=$htm_capacity_abort_strategy STARTING_MODE=-DSTARTING_MODE=$adaptivity_starting_mode"
        if [ $adaptivity_disabled -eq 1 ] ; then
            make_command="$make_command NO_ADAPTIVITY=-DNO_ADAPTIVITY=1"
        fi
        $make_command
    fi
    rc=$?
    if [[ $rc != 0 ]] ; then
        echo ""
        echo "=================================== ERROR BUILDING $F - $name ===================================="
        echo ""
        exit 1
    fi
    cd ..
done

