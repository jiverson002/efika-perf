cmake_minimum_required(VERSION 3.14)

# Subproject of efika-perf
project(efika-perf-impl)

# Check for existence of specific implementations
include(CheckPrototypeDefinition)

get_target_property(CMAKE_REQUIRED_INCLUDES
  Efika::efika INTERFACE_INCLUDE_DIRECTORIES)

check_prototype_definition(EFIKA_Impl_sfr0d
  "int EFIKA_Impl_sfr0d(EFIKA_val_t const minsim, EFIKA_Matrix const * const M, Vector * const A)" "0"
  "efika/impl.h" HAS_SFR0D)
check_prototype_definition(EFIKA_Impl_sfrkd
  "int EFIKA_Impl_sfrkd(EFIKA_val_t const minsim, EFIKA_Matrix const * const M, EFIKA_Matrix const * const I, Vector * const A)" "0"
  "efika/impl.h" HAS_SFRKD)

if(NOT HAS_SFR0D)
  message(FATAL_ERROR "There is no baseline implementation available. The Celero benchmark framework being used requires a baseline. As such, this build will not complete until until a suitable baseline function prototype is available.")
endif()

# ...
add_library(${PROJECT_NAME} OBJECT)

target_sources(${PROJECT_NAME}
  PRIVATE bench_impl.cc)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<${HAS_SFR0D}:HAS_SFR0D> $<${HAS_SFRKD}:HAS_SFRKD>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Celero::celero Efika::data Efika::efika)

add_library(EfikaPerf::impl ALIAS ${PROJECT_NAME})
