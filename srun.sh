#!/bin/bash

# BASH_SOURCE has the script's path
SCRIPT_PATH=$(dirname ${BASH_SOURCE})
# forward to allolib run script
${SCRIPT_PATH}/allolib/srun.sh $@