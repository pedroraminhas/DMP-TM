#!/bin/sh

cd stms;
cd norec;
make clean;
cd ../tinystm;
make clean;
cd ../tl2;
make clean;
cd ../swisstm;
make clean;
cd ../../
cd benchmarks/datastructures
bash clean-datastructures.sh
cd ../stamp
bash clean-stamp.sh
cd ../stmbench7
bash clean-stmbench7.sh
cd ../..
