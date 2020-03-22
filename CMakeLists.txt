cmake_minimum_required(VERSION 3.10)

#-------------------------------------------------------------------------------
# BOOTSTRAP w/ latest bootstrap/cmake scripts
#-------------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(cmake
  GIT_REPOSITORY git@github.com:jiverson002/bootstrap-cmake.git
)
FetchContent_MakeAvailable(cmake)

#-------------------------------------------------------------------------------
# Import dependencies
#-------------------------------------------------------------------------------
set(core_GIT_REPOSITORY git@github.com:jiverson002/efika-core.git)
set(data_GIT_REPOSITORY git@github.com:jiverson002/efika-data.git)
set(impl_GIT_REPOSITORY git@github.com:jiverson002/efika-impl.git)

import(core data impl)

#-------------------------------------------------------------------------------
# SUBPROJECT configuration
#-------------------------------------------------------------------------------
project(${BS_PROJECT_NAME}-perf LANGUAGES C CXX)

cmake_policy(SET CMP0076 NEW)

#-------------------------------------------------------------------------------
# Require C++17 support from compiler for benchmarks
#-------------------------------------------------------------------------------
cxx_17()

#-------------------------------------------------------------------------------
# ...
#-------------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(BSBench)
endif()

#-------------------------------------------------------------------------------
# ...
#-------------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_BENCHMARKING)
  add_subdirectory(src)
endif()
