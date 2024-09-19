#!/bin/bash

mkdir -p build/release
cd build/release
cmake -DCMAKE_BUILD_TYPE=Release -Wno-deprecated -DBUILD_EXAMPLES=0 ../..