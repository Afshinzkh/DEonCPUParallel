#!/bin/bash
# For C
g++ -fopenmp -std=c++11 DE.cpp Test.cpp CIR.cpp Vasicek.cpp

./a.out data12.csv cir CR 3
./a.out data12.csv cir CR 4
./a.out data12.csv cir CR 5

./a.out data12.csv cir F 1
./a.out data12.csv cir F 2
./a.out data12.csv cir F 3
./a.out data12.csv cir F 4
./a.out data12.csv cir F 5
