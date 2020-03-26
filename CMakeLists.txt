cmake_minimum_required( VERSION 3.13 )

# Subproject of efika
project( efika-perf )

# Always use '-fPIC'/'-fPIE' option.
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

# Require C++17 compliance from compiler
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

#-------------------------------------------------------------------------------
# Celero configuration
#-------------------------------------------------------------------------------
include( FetchContent )

FetchContent_Declare( Celero
  GIT_REPOSITORY https://github.com/DigitalInBlue/Celero.git )

FetchContent_MakeAvailable( Celero )

#-------------------------------------------------------------------------------
# EXECUTABLE configuration
#-------------------------------------------------------------------------------
add_executable( ${PROJECT_NAME} )
target_sources( ${PROJECT_NAME} PRIVATE src/bench_main.cc )
target_link_libraries( ${PROJECT_NAME} PRIVATE celero )

#-------------------------------------------------------------------------------
# Declare efika dependencies
#-------------------------------------------------------------------------------
foreach( dep data impl )
  FetchContent_Declare( efika-${dep}
    GIT_REPOSITORY git@github.com:jiverson002/efika-${dep}.git )

  FetchContent_MakeAvailable( efika-${dep} )

  target_link_libraries( ${PROJECT_NAME}
    PUBLIC Efika::${dep} EfikaPerf::${dep} )

  add_subdirectory( src/${dep} )
endforeach()
