# Check for existence of specific implementations
project_check_prototype_definition(sfr1d
  "void sfr1d(Problem P, Vector * const A)" "\;"
  "efika/impl/sfr.h" HAS_SFR1D)
project_check_prototype_definition(sfrkd
  "void sfrkd(Problem P, Vector * const A)" "\;"
  "efika/impl/sfr.h" HAS_SFRKD)

# Check for existence of a baseline impl
if(NOT HAS_SFR1D)
  message(FATAL_ERROR "There is no baseline implementation available. The Celero benchmark framework being used requires a baseline. As such, this build will not complete until until a suitable baseline function prototype is available.")
endif()

# Add compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<${HAS_SFR1D}:HAS_SFR1D>
  $<${HAS_SFRKD}:HAS_SFRKD>
)

# Add this benchmark group to benchmark executable
target_sources(${PROJECT_NAME} PRIVATE
  bench_impl.cc
)
