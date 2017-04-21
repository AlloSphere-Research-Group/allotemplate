#!/bin/bash

# get this run script's abosolute path
INITIALDIR=${PWD}
echo "Script executed from: ${INITIALDIR}"
# BASH_SOURCE has the script's path
# could be absolute, could be relative
SCRIPT_PATH=$(dirname ${BASH_SOURCE})
FIRSTCHAR=${SCRIPT_PATH:0:1}
# echo "FIRST CHAR: ${FIRSTCHAR}"
if [ ${FIRSTCHAR} == "/" ]; then
  # it's asolute path
  ALPROJ_PATH=${SCRIPT_PATH}
else
  # SCRIPT_PATH was relative
  ALPROJ_PATH=${INITIALDIR}/${SCRIPT_PATH}
fi
echo "absolute path to scipt: ${ALPROJ_PATH}"
# and cd to where the scipt is
cd ${ALPROJ_PATH}
echo "changed working directory to where the script is. now in ${PWD}"

# check if we want debug build
BUILD_TYPE=Release
while getopts ":d" opt; do
  case $opt in
  d)
	BUILD_TYPE=Debug
	POSTFIX=_debug
	shift # consume option
    ;;
  esac
done
echo "BUILD TYPE: ${BUILD_TYPE}"

# first build al_lib ###########################################################
echo " "
echo "___ building al_lib __________"
echo " "
mkdir -p build
cd build
mkdir -p "al_lib_build_${BUILD_TYPE}"
cd "al_lib_build_${BUILD_TYPE}"
cmake ../../al_lib/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make
LIB_BUILD_RESULT=$?
# if lib failed to build, exit
if [ ${LIB_BUILD_RESULT} != 0 ]; then
	exit 1
fi

# then build the app ###########################################################
APP_NAME="$1" # first argument (assumming we consumed all the options above)
APP_PATH=${INITIALDIR}/${APP_NAME}

# discard '/'' in the end of the input directory (if it has)
LASTCHAR=${APP_NAME:(-1)}
if [ ${LASTCHAR} == "/" ]; then
    # ${string%substring}
    # Strips shortest match of $substring from back of $string.
    APP_NAME=${APP_NAME%/}
fi
# get only last foldername
APP_NAME=$(basename "$1")
# Replace all periods and slashes with underscores.
# to see how it works, try: echo "t_ f\\ e/" | sed 's/[\.\/\\]/_/g'
APP_NAME=$(echo "${APP_NAME}" | sed 's/[\.\/\\]/_/g')

echo " "
echo "___ building ${APP_NAME} __________"
echo " "

echo "app path: ${APP_PATH}"
# go to where app is
cd ${APP_PATH}
mkdir -p build
cd build
cmake ${APP_PATH}/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -Dal_path=${ALPROJ_PATH}/al_lib
make
APP_BUILD_RESULT=$?
# if app failed to build, exit
if [ ${APP_BUILD_RESULT} != 0 ]; then
	exit 1
fi

# run app ######################################################################
# go to where excutable is so we have cwd there
# (app's cmake is set to put binary in 'bin')
cd ${APP_PATH}/bin
echo " "
echo "___ running ${APP_NAME} __________"
echo " "
./"${APP_NAME}${POSTFIX}"