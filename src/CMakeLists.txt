#-------------------------------------------------------------------------------
# EXECUTABLE configuration
#-------------------------------------------------------------------------------
add_executable( ${PROJECT_NAME} )
target_sources( ${PROJECT_NAME} PRIVATE bench_main.cc )
target_link_libraries( ${PROJECT_NAME} PRIVATE celero efika-data )

#-------------------------------------------------------------------------------
# Declare efika dependencies
#-------------------------------------------------------------------------------
foreach( dep data impl )
  FetchContent_Declare( efika-${dep}
    GIT_REPOSITORY git@github.com:jiverson002/efika-${dep}.git )
  FetchContent_MakeAvailable( efika-${dep} )

  add_subdirectory( ${dep} )

  target_link_libraries( ${PROJECT_NAME} PUBLIC  Efika::${dep} )
  target_link_libraries( ${PROJECT_NAME} PUBLIC  EfikaPerf::${dep} )
endforeach()
