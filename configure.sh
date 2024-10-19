#!/bin/bash

mkdir -p build/Release
cmake -DCMAKE_BUILD_TYPE=Release -Wno-deprecated -DBUILD_EXAMPLES=0 -B build/Release -S .