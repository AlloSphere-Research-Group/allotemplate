#!/bin/bash

mkdir -p build
cd build
mkdir -p release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -Wno-deprecated -DBUILD_EXAMPLES=0 ../..