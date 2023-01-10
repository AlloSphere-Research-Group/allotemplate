#!/bin/bash

rm -rf .git
git init

# add allolib and al_ext as submodules
git submodule add -b devel https://github.com/AlloSphere-Research-Group/allolib.git
git submodule add -b devel https://github.com/AlloSphere-Research-Group/al_ext.git

# update submodules with blobs omitted
git submodule update --recursive --init --filter=blob:none