#!/bin/bash

# Configure debug build
(
  mkdir -p build
  cd build
  mkdir -p debug
  cd debug
  cmake -DCMAKE_BUILD_TYPE=Debug -Wno-deprecated -DBUILD_EXAMPLES=0 ../..
)

(
  # utilizing cmake's parallel build options
  # for cmake >= 3.12: -j <number of processor cores + 1>
  # for older cmake: -- -j5
  cmake --build build/debug -j 9
)

result=$?
if [ ${result} == 0 ]; then
  gdb -ex run ./bin/appd
fi