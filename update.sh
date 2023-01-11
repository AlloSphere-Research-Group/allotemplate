#!/bin/bash

git pull

# update submodules with blobs omitted
git submodule update --recursive --init --filter=blob:none