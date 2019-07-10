#!/bin/bash

#  check for remote get-url origin
if [ `git remote get-url origin` == "https://github.com/AlloSphere-Research-Group/allotemplate.git" ]; then
  rm -rf .git

  fi

rm init.sh