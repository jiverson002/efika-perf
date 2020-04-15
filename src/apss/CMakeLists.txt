cmake_minimum_required(VERSION 3.14)

# Subproject of efika-perf
project(efika-perf-apss)

# Check for existence of specific implementations
include(CheckPrototypeDefinition)

get_target_property(CMAKE_REQUIRED_INCLUDES
  Efika::efika INTERFACE_INCLUDE_DIRECTORIES)

check_prototype_definition(EFIKA_apss_bruteforce
  "int EFIKA_apss_bruteforce(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_BRUTEFORCE)
check_prototype_definition(EFIKA_apss_idxjoin
  "int EFIKA_apss_idxjoin(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_IDXJOIN)
check_prototype_definition(EFIKA_apss_allpairs
  "int EFIKA_apss_allpairs(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_ALLPAIRS)
check_prototype_definition(EFIKA_apss_mmjoin
  "int EFIKA_apss_mmjoin(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_MMJOIN)
check_prototype_definition(EFIKA_apss_l2ap
  "int EFIKA_apss_l2ap(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_L2AP)
check_prototype_definition(EFIKA_apss_sfrkd
  "int EFIKA_apss_sfrkd(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_SFRKD)

if(NOT HAS_BRUTEFORCE AND NOT HAS_IDXJOIN)
  message(FATAL_ERROR "There is no baseline implementation available. The Celero benchmark framework being used requires a baseline. As such, this build will not complete until until a suitable baseline function prototype is available.")
endif()

# ...
add_library(${PROJECT_NAME} OBJECT)

target_sources(${PROJECT_NAME}
  PRIVATE bench_apss.cc)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<${HAS_BRUTEFORCE}:HAS_BRUTEFORCE> $<${HAS_IDXJOIN}:HAS_IDXJOIN>
          $<${HAS_ALLPAIRS}:HAS_ALLPAIRS> $<${HAS_MMJOIN}:HAS_MMJOIN>
          $<${HAS_L2AP}:HAS_L2AP> $<${HAS_SFRKD}:HAS_SFRKD>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Celero::celero Efika::efika)

add_library(EfikaPerf::apss ALIAS ${PROJECT_NAME})
