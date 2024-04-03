#!/bin/bash

# make a new git repository instead of the original allotemplate
rm -rf .git
git init

# need to re-add the submodules into the index
rmdir allolib
rmdir al_ext
git submodule add -b devel https://github.com/AlloSphere-Research-Group/allolib.git
git submodule add -b devel https://github.com/AlloSphere-Research-Group/al_ext.git

git submodule update --recursive --init --filter=blob:none

rm init.sh