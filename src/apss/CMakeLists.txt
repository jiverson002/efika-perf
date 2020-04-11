cmake_minimum_required(VERSION 3.14)

# Subproject of efika-perf
project(efika-perf-apss)

# Check for existence of specific implementations
include(CheckPrototypeDefinition)

get_target_property(CMAKE_REQUIRED_INCLUDES
  Efika::efika INTERFACE_INCLUDE_DIRECTORIES)

check_prototype_definition(EFIKA_apss_bruteforce
  "int EFIKA_apss_bruteforce(EFIKA_val_t const minsim, EFIKA_Matrix const * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_BRUTEFORCE)
check_prototype_definition(EFIKA_apss_sfrkd
  "int EFIKA_apss_sfrkd(EFIKA_val_t const minsim, EFIKA_Matrix const * const M, EFIKA_Matrix const * const I, Vector * const A)" "0"
  "efika/apss.h" HAS_SFRKD)

if(NOT HAS_BRUTEFORCE)
  message(FATAL_ERROR "There is no baseline implementation available. The Celero benchmark framework being used requires a baseline. As such, this build will not complete until until a suitable baseline function prototype is available.")
endif()

# ...
add_library(${PROJECT_NAME} OBJECT)

target_sources(${PROJECT_NAME}
  PRIVATE bench_apss.cc)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<${HAS_SFR0D}:HAS_BRUTEFORCE> $<${HAS_SFRKD}:HAS_SFRKD>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Celero::celero Efika::data Efika::efika)

add_library(EfikaPerf::apss ALIAS ${PROJECT_NAME})
