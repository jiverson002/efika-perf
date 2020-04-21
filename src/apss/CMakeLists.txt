cmake_minimum_required(VERSION 3.14)

# Subproject of efika-perf
project(efika-perf-apss)

# Check for existence of specific implementations
include(CheckPrototypeDefinition)

#-------------------------------------------------------------------------------
# EFIKA configuration
#-------------------------------------------------------------------------------
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
check_prototype_definition(EFIKA_apss_l2ap
  "int EFIKA_apss_l2ap(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_L2AP)
check_prototype_definition(EFIKA_apss_mmjoin
  "int EFIKA_apss_mmjoin(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_MMJOIN)
check_prototype_definition(EFIKA_apss_nova
  "int EFIKA_apss_nova(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_NOVA)
check_prototype_definition(EFIKA_apss_sfrkd
  "int EFIKA_apss_sfrkd(EFIKA_val_t const minsim, EFIKA_Matrix * const M, EFIKA_Matrix * const S)" "0"
  "efika/apss.h" HAS_SFRKD)

if(NOT HAS_BRUTEFORCE AND NOT HAS_IDXJOIN)
  message(FATAL_ERROR "There is no baseline implementation available. The Celero benchmark framework being used requires a baseline. As such, this build will not complete until until a suitable baseline function prototype is available.")
endif()

#-------------------------------------------------------------------------------
# LIBRARY configuration
#-------------------------------------------------------------------------------
add_library(${PROJECT_NAME} OBJECT)

target_sources(${PROJECT_NAME}
  PRIVATE bench_apss.cc)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<${HAS_BRUTEFORCE}:HAS_BRUTEFORCE> $<${HAS_IDXJOIN}:HAS_IDXJOIN>
          $<${HAS_ALLPAIRS}:HAS_ALLPAIRS> $<${HAS_L2AP}:HAS_L2AP>
          $<${HAS_MMJOIN}:HAS_MMJOIN> $<${HAS_NOVA}:HAS_NOVA>
          $<${HAS_SFRKD}:HAS_SFRKD>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Celero::celero Efika::efika)

add_library(EfikaPerf::apss ALIAS ${PROJECT_NAME})

#-------------------------------------------------------------------------------
# REFERENCE configuration
#-------------------------------------------------------------------------------
find_package(L2AP CONFIG)

if(L2AP_FOUND)
  get_target_property(L2AP_REQUIRED_INCLUDES
    L2AP::l2ap INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(OPENMP_REQUIRED_INCLUDES
    OpenMP::OpenMP_C INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND CMAKE_REQUIRED_INCLUDES
    ${L2AP_REQUIRED_INCLUDES}
    ${OPENMP_REQUIRED_INCLUDES})

  check_prototype_definition(L2AP
    "int L2AP(void)" "0"
    "l2ap/includes.h" HAS_REF_L2AP)

  target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<${HAS_REF_L2AP}:HAS_REF_L2AP>)

  target_link_libraries(${PROJECT_NAME}
    PRIVATE L2AP::l2ap)
endif()
