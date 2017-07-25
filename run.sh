#!/bin/bash

INITIALDIR=${PWD}
# echo "Script executed from: ${INITIALDIR}"

# BASH_SOURCE has the script's path
# could be absolute, could be relative
SCRIPT_PATH=$(dirname ${BASH_SOURCE})

FIRSTCHAR=${SCRIPT_PATH:0:1}
if [ ${FIRSTCHAR} == "/" ]; then
  # it's asolute path
  AL_PROJ_PATH=${SCRIPT_PATH}
else
  # SCRIPT_PATH was relative
  AL_PROJ_PATH=${INITIALDIR}/${SCRIPT_PATH}
fi

# forward to allolib run script
${AL_PROJ_PATH}/allolib/run.sh $@