#-------------------------------------------------------------------------------
# ...
#-------------------------------------------------------------------------------
if(${CMAKE_VERSION} VERSION_LESS "3.14.0")
  macro(FetchContent_MakeAvailable name)
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)
      FetchContent_Populate(${name})
      add_subdirectory(${${name}_SOURCE_DIR} ${${name}_BINARY_DIR})
    endif()
  endmacro()
endif()

#-------------------------------------------------------------------------------
# BOOTSTRAP w/ latest bootstrap/cmake scripts
#-------------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(bootstrap
  GIT_REPOSITORY git@github.com:jiverson002/bootstrap-cmake.git
)
FetchContent_MakeAvailable(bootstrap)

#-------------------------------------------------------------------------------
# Include project dependencies
#-------------------------------------------------------------------------------
Bootstrap_dependency_file(${CMAKE_CURRENT_LIST_DIR}/Dependencies.cmake)
