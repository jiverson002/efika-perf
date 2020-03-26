# Check for existence of specific implementations
include( CheckPrototypeDefinition )

get_target_property( CMAKE_REQUIRED_INCLUDES Efika::impl INCLUDE_DIRECTORIES )

check_prototype_definition( EFIKA_Impl_sfr1d
  "void EFIKA_Impl_sfr1d(Problem P, Vector * const A)" "\;"
  "efika/impl.h" HAS_SFR1D )
check_prototype_definition( EFIKA_Impl_sfrkd
  "void EFIKA_Impl_sfrkd(Problem P, Vector * const A)" "\;"
  "efika/impl.h" HAS_SFRKD )

if( NOT HAS_SFR1D )
  message( FATAL_ERROR "There is no baseline implementation available. The Celero benchmark framework being used requires a baseline. As such, this build will not complete until until a suitable baseline function prototype is available." )
endif()

# ...
add_library( efika-perf-impl OBJECT )

target_sources( efika-perf-impl PRIVATE bench_impl.cc )

target_compile_definitions( efika-perf-impl
  PRIVATE $<${HAS_SFR1D}:HAS_SFR1D> $<${HAS_SFRKD}:HAS_SFRKD> )

target_link_libraries( efika-perf-impl
  PRIVATE celero Efika::data Efika::impl )

add_library( EfikaPerf::impl ALIAS efika-perf-impl )
