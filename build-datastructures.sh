#!/bin/sh
#FOLDERS="skiplist linkedlist redblacktree hashmap"
FOLDERS="hashmap-part-not-ordered"

export "LD_LIBRARY_PATH=/home/praminhas/lib/"

if [ $# -eq 0 ] ; then
    echo " === ERROR At the very least, we need the backend name in the first parameter. === "
    exit 1
fi

backend=$1  # e.g.: greentm


htm_retries=5
htm_capacity_abort_strategy=2

if [ $# -eq 3 ] ; then
    htm_retries=$2 # e.g.: 5
    htm_capacity_abort_strategy=$3 # e.g.: 0 for "give up"
fi

rm lib/*.o || true

rm Defines.common.mk
rm Makefile
rm Makefile.flags
rm lib/thread.h
rm lib/thread.c
rm lib/stm_src.
rm lib/init_system.c
rm lib/tm.h
rm lib/stm_src.c
rm lib/init_system.c
rm lib/handler.c
rm lib/stm_src.h
rm lib/init_system.h
rm lib/handler.h

cp ../../backends/$backend/Defines.common.mk .
cp ../../backends/$backend/Makefile .
cp ../../backends/$backend/Makefile.flags .
cp ../../backends/$backend/thread.h lib/
cp ../../backends/$backend/thread.c lib/
cp ../../backends/$backend/tm.h lib/
#cp ../../backends/$backend/stm_src.c lib/
#cp ../../backends/$backend/init_system.c lib/
#cp ../../backends/$backend/handler.c lib/
#cp ../../backends/$backend/stm_src.h lib/
#cp ../../backends/$backend/init_system.h lib/
#cp ../../backends/$backend/handler.h lib/

for F in $FOLDERS
do
    cd $F
    rm *.o || true
    rm $F
    make_command="make -f Makefile HTM_RETRIES=-DHTM_RETRIES=$htm_retries RETRY_POLICY=-DRETRY_POLICY=$htm_capacity_abort_strategy"
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

