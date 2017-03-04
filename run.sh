#!/bin/sh

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
echo "${BUILD_TYPE}"

# first build al_lib ###########################################################
echo "building al_lib"
mkdir -p build
cd build
mkdir -p "al_lib_build_${BUILD_TYPE}"
cd "al_lib_build_${BUILD_TYPE}"
cmake ../../al_lib/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make
LIB_BUILD_RESULT=$?
cd .. # back to al_proj/build

# then build the app ###########################################################
echo "building ${APP_NAME}"
APP_NAME="$1" # first argument (assumming we consumed all the options above)
# discard / in the end of the input directory (if it has)
LASTCHAR=${APP_NAME:(-1)}
if [ ${LASTCHAR} == "/" ]; then
    # ${string%substring}
    # Strips shortest match of $substring from back of $string.
    APP_NAME=${APP_NAME%/}
fi
mkdir -p ${APP_NAME}
cd ${APP_NAME}
cmake ../../${APP_NAME}/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make
APP_BUILD_RESULT=$?

# only run executable if return value from make was 0 (success) ################
if [ ${LIB_BUILD_RESULT} == 0 ]; then
	if [ ${APP_BUILD_RESULT} == 0 ]; then
		cd ../.. # back to al_proj
		echo "running ${APP_NAME}"
		cd ${APP_NAME}/bin # go to where excutable is so we have cwd there
		./"${APP_NAME}${POSTFIX}"
	fi
fi