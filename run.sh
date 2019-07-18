#!/bin/bash
(
#    cd build/release
    cmake --build build/release
)

result=$?
if [ ${result} == 0 ]; then
    ./bin/app
fi
