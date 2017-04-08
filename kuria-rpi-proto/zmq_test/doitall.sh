#! /bin/sh

mkdir build
cd build
cmake ..
make clean
make
cd ..
mkdir mylocal
./build/zmq_pub_test
./build/zmq_sub_test
