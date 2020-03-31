cmake_minimum_required(VERSION 3.14)

# Subproject of efika
project(efika-perf)

# Include build configurations (compiler standards, options, etc.)
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND EFIKA_BUILDCONFIG_FILE)
  include(${EFIKA_BUILDCONFIG_FILE})
endif()

# Include required CMake modules
include(FetchContent)

#-------------------------------------------------------------------------------
# EXECUTABLE configuration
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE src/bench_main.cc)

#-------------------------------------------------------------------------------
# DEPENDENCY configuration
#-------------------------------------------------------------------------------
# Celero::celero
find_package(Celero CONFIG REQUIRED)

# Efika::data
FetchContent_Declare(efika-data
  GIT_REPOSITORY git@github.com:jiverson002/efika-data.git)

FetchContent_MakeAvailable(efika-data)

# Efika::efika
find_package(Efika CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Celero::celero Efika::data Efika::efika)

#-------------------------------------------------------------------------------
# BENCHMARKS configuration
#-------------------------------------------------------------------------------
foreach(bench impl)
  add_subdirectory(src/${bench})

  target_link_libraries(${PROJECT_NAME} PRIVATE EfikaPerf::${bench})
endforeach()
