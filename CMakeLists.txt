cmake_minimum_required(VERSION 3.14)

# Subproject of efika
project(efika-perf)

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

# Efika::efika
find_package(Efika CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Celero::celero)

#-------------------------------------------------------------------------------
# BENCHMARKS configuration
#-------------------------------------------------------------------------------
foreach(bench apss)
  add_subdirectory(src/${bench})

  target_link_libraries(${PROJECT_NAME} PRIVATE EfikaPerf::${bench})
endforeach()
