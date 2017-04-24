cmake_minimum_required(VERSION 3.0)
get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${app_name})

# get app config user specified
include(app_config.cmake)

# if al_path not defined, set to default one
if (NOT al_path)
	set(al_path ${CMAKE_CURRENT_SOURCE_DIR}/../al_lib)
endif (NOT al_path)

set(app_path ${CMAKE_CURRENT_SOURCE_DIR})

# APP FLAGS
if (WINDOWS)
  # set(CMAKE_CXX_FLAGS "/W2")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-unused ${CMAKE_CXX_FLAGS}")
endif (WINDOWS)

include(${al_path}/cmake_modules/configure_platform.cmake)
include(${al_path}/cmake_modules/find_core_dependencies.cmake)
include(${al_path}/cmake_modules/app_set_directories.cmake)
add_executable(${app_name} ${app_files_list})
include(${al_path}/cmake_modules/app_link_include_and_properties.cmake)
include(${al_path}/cmake_modules/app_postbuild_events.cmake)