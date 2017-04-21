cmake_minimum_required(VERSION 3.0)
get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(${app_name})
project(${app_name})
set(al_path ${CMAKE_CURRENT_SOURCE_DIR}/../al_lib)
set(app_path ${CMAKE_CURRENT_SOURCE_DIR})
include(${al_path}/cmake_modules/configure_platform.cmake)
include(${al_path}/cmake_modules/find_core_dependencies.cmake)

# APP PATH DEFINE ##############################################################
set(APP_OUTPUT_PATH ${app_path}/bin)
if (WINDOWS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${APP_OUTPUT_PATH})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${APP_OUTPUT_PATH})
else ()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_OUTPUT_PATH})
endif (WINDOWS)

# APP FLAGS ####################################################################
if (WINDOWS)
  # set(CMAKE_CXX_FLAGS "/W2")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-unused")
endif (WINDOWS)

include(app_config.cmake) # get app config list user specified
include(${al_path}/cmake_modules/app_set_directories.cmake)
add_executable(${app_name} ${app_files_list})
include(${al_path}/cmake_modules/app_link_include_and_properties.cmake)
include(${al_path}/cmake_modules/app_postbuild_events.cmake)
