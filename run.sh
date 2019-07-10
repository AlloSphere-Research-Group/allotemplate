#!/bin/bash
(
    cd build/release
    make -j 7
)

result=$?
if [ ${result} == 0 ]; then
    ./bin/test
fi