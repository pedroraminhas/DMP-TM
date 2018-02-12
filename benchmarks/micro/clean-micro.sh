#!/bin/sh
FOLDERS="synth"
name=$1

rm lib/*.o || true

rm Defines.common.mk
rm Makefile
rm Makefile.flags
rm lib/thread.h
rm lib/thread.c
rm lib/tm.h
rm lib/rectm.h

for F in $FOLDERS
do
    cd $F
    rm *.o || true
    rm $F
    cd ..
done
