#!/bin/bash

INITIALDIR=${PWD} # gives absolute path
# echo "Script executed from: ${INITIALDIR}"

# BASH_SOURCE has the script's path could be absolute, could be relative
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})

FIRSTCHAR=${SCRIPT_PATH:0:1}
if [ ${FIRSTCHAR} == "/" ]; then # it's asolute path
  AL_TEMPLATE_PATH=${SCRIPT_PATH}
  AL_LIB_PATH=${AL_TEMPLATE_PATH}/allolib
else # SCRIPT_PATH was relative
  AL_TEMPLATE_PATH=${INITIALDIR}/${SCRIPT_PATH} # make it absolute
  AL_LIB_PATH=${AL_TEMPLATE_PATH}/allolib
fi

# resolve flags
BUILD_TYPE=Release # release build by default
while getopts ":dn" opt; do
  case $opt in
  d)
  BUILD_TYPE=Debug
  POSTFIX=_debug # if release, there's no postfix
  shift # consume option
    ;;
  n)
  EXIT_AFTER_BUILD=1
  shift
    ;;
  esac
done
echo "BUILD TYPE: ${BUILD_TYPE}"

# build allolib
echo " "
echo "___ building allolib __________"
echo " "

cd ${AL_LIB_PATH}
git submodule init
git submodule update
mkdir -p build
cd build
mkdir -p "${BUILD_TYPE}"
cd "${BUILD_TYPE}"
cmake ../.. -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
make
LIB_BUILD_RESULT=$?
if [ ${LIB_BUILD_RESULT} != 0 ]; then
  echo "allolib failed to build"
  exit 1 # if lib failed to build, exit
fi

# build gamma if it exists
cd ${AL_TEMPLATE_PATH}
if [ -d "Gamma" ]; then
  echo " "
  echo "___ Gamma found, building Gamma __________"
  echo " "
  cd Gamma
  mkdir -p build
  cd build
  mkdir -p "${BUILD_TYPE}"
  cd "${BUILD_TYPE}"
  cmake ../.. -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  make
  GAMMA_BUILD_RESULT=$?
  if [ ${GAMMA_BUILD_RESULT} != 0 ]; then
    echo "Gamma failed to build"
  else
    GAMMA_INCLUDE_DIRS=${AL_TEMPLATE_PATH}/Gamma
    GAMMA_LINK_LIBS=${AL_TEMPLATE_PATH}/Gamma/lib/libGamma.a
  fi
fi

# build app

APP_FILE_INPUT="$1" # first argument (assumming we consumed all the options above)
APP_PATH=$(dirname ${APP_FILE_INPUT})
APP_FILE=$(basename ${APP_FILE_INPUT})
APP_NAME=${APP_FILE%.*} # remove extension (once, assuming .cpp)
echo " "
echo "___ building ${APP_NAME} __________"
echo " "
echo "    app path: ${APP_PATH}"
echo "    app file: ${APP_FILE}"
echo "    app name: ${APP_NAME}"

echo "${GAMMA_INCLUDE_DIRS}"
echo "${GAMMA_LINK_LIBS}"

cd ${INITIALDIR}
cd ${APP_PATH}
mkdir -p build
cd build
mkdir -p ${APP_NAME}
cd ${APP_NAME}
cmake ${AL_LIB_PATH}/cmake/single_file -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -Dal_path=${AL_LIB_PATH} -DAL_APP_FILE=../../${APP_FILE} -Dapp_include_dirs=${GAMMA_INCLUDE_DIRS}\; -Dapp_link_libs=${GAMMA_LINK_LIBS}\;
make

APP_BUILD_RESULT=$?
if [ ${APP_BUILD_RESULT} != 0 ]; then
  exit 1 # if app failed to build, exit
fi

if [ ${EXIT_AFTER_BUILD} ]; then
  exit 0
fi

# run app
# go to where the binary is so we have cwd there
# (app's cmake is set to put binary in 'bin')
cd ${INITIALDIR}
cd ${APP_PATH}/bin
echo " "
echo "___ running ${APP_NAME} __________"
echo " "
./"${APP_NAME}${POSTFIX}"