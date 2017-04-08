#! /bin/sh

cd build
make clean
make
cd ..
build/kuria_proto_test
