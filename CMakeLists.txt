cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(app)

include(${CMAKE_CURRENT_LIST_DIR}/allolib/al/lib/cmake/Gamma/Gamma-targets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/allolib/al/lib/cmake/glew/glew-targets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/allolib/al/lib/cmake/glfw3/glfw3Targets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/allolib/al/lib/cmake/al/al-targets.cmake)

if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif ()

add_executable(bin "")

target_sources(bin PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/allolib/examples/sound/gam_sine.cpp
)

set_target_properties(bin PROPERTIES
  DEBUG_POSTFIX _debug
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)

target_compile_options(bin PRIVATE -Wall)

target_link_libraries(bin PRIVATE al)