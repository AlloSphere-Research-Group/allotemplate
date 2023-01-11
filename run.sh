#!/bin/bash
(
  # utilizing cmake's parallel build options
  # for cmake >= 3.12: -j <number of processor cores + 1>
  # for older cmake: -- -j5
  cmake --build build/release --config Release -j 9
)

result=$?
if [ ${result} == 0 ]; then
  ./bin/app
fi