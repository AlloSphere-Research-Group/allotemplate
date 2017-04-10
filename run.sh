#!/bin/bash

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
echo "building al_lib"
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

cd .. # back to al_proj/build

# then build the app ###########################################################
echo "building ${APP_NAME}"
APP_NAME="$1" # first argument (assumming we consumed all the options above)
# discard '/'' in the end of the input directory (if it has)
LASTCHAR=${APP_NAME:(-1)}
if [ ${LASTCHAR} == "/" ]; then
    # ${string%substring}
    # Strips shortest match of $substring from back of $string.
    APP_NAME=${APP_NAME%/}
fi
# Replace all periods and slashes with underscores.
# to see how it works, try: echo "t_ f\\ e/" | sed 's/[\.\/\\]/_/g'
APP_NAME=$(echo "${APP_NAME}" | sed 's/[\.\/\\]/_/g')

mkdir -p ${APP_NAME}
cd ${APP_NAME}
cmake ../../${APP_NAME}/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make
APP_BUILD_RESULT=$?
# if app failed to build, exit
if [ ${APP_BUILD_RESULT} != 0 ]; then
	exit 1
fi

cd ../.. # back to al_proj

# run app ######################################################################
cd ${APP_NAME}/bin # go to where excutable is so we have cwd there
echo " "
echo "___ running: ${APP_NAME} __________"
echo " "
./"${APP_NAME}${POSTFIX}"