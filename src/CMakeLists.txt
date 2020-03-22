#-------------------------------------------------------------------------------
# ...
#-------------------------------------------------------------------------------
include(CheckPrototypeDefinition)
function(project_check_prototype_definition_process_library lib)
  get_target_property(LIB_TYPE ${lib} TYPE)

  if(LIB_TYPE STREQUAL "INTERFACE_LIBRARY")
    get_target_property(LIB_INCLUDE_DIRECTORIES ${lib}
      INTERFACE_INCLUDE_DIRECTORIES)
  else()
    get_target_property(LIB_INCLUDE_DIRECTORIES ${lib} INCLUDE_DIRECTORIES)
  endif()

  list(APPEND CMAKE_REQUIRED_INCLUDES ${LIB_INCLUDE_DIRECTORIES})

  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} PARENT_SCOPE)
endfunction()

function(project_check_prototype_definition)
  get_target_property(BS_PROJECT_LINK_LIBRARIES ${BS_PROJECT_NAME}
    INTERFACE_LINK_LIBRARIES)
  foreach(lib ${BS_PROJECT_NAME} ${BS_PROJECT_LINK_LIBRARIES})
    string(REGEX REPLACE "::.*$" "" newlib ${lib})
    project_check_prototype_definition_process_library(${newlib})
  endforeach()

  check_prototype_definition(${ARGN})
endfunction()

#-------------------------------------------------------------------------------
# EXECUTABLE configuration
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
  bench_main.cc
)

target_link_libraries(${PROJECT_NAME}
  ${BS_PROJECT_NAME}
)

project_add_bench(${PROJECT_NAME})

#-------------------------------------------------------------------------------
# ...
#-------------------------------------------------------------------------------
add_subdirectory(impl)
