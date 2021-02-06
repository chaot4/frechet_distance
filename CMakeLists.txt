# USING cmake:
#   mkdir build
#   cd build
#   cmake ..
#   make -j

# RUN unit tests:
#   cd build
#   ctest



cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(frechet CXX)

enable_testing()

include(CMakeDetermineCXXCompiler)

# default to RelWithDebInfo
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(EXTRA_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -pedantic" CACHE STRING "Extra flags used by the compiler during all build types.")
	set(EXTRA_EXE_LINKER_FLAGS "" CACHE STRING "Extra flags used by the linker.")
	set(EXTRA_EXE_LINKER_FLAGS_RELEASE "-flto" CACHE STRING "Extra flags used by the linker for the Release build type.")
	set(EXTRA_EXE_LINKER_FLAGS_RELWITHDEBINFO "-flto" CACHE STRING "Extra flags used by the linker for the RelWithDebInfo build type.")
else()
	set(EXTRA_CXX_FLAGS "" CACHE STRING "Extra flags used by the compiler during all build types.")
	set(EXTRA_EXE_LINKER_FLAGS "" CACHE STRING "Extra flags used by the linker.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS} -std=c++11 -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_EXE_LINKER_FLAGS} -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${EXTRA_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${EXTRA_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	add_compile_definitions(WITH_OPENMP)
endif()

option(VERBOSE "Verbose logging" OFF)
if(NOT VERBOSE)
	add_definitions(-DNVERBOSE)
endif()

# compile shared sources only once, and reuse object files in both,
# as they are compiled with the same options anyway
add_library(common OBJECT
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(common PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable(frechet
	src/main.cpp
	$<TARGET_OBJECTS:common>
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(frechet PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable(run_tests
	src/run_tests.cpp
	src/unit_tests.cpp
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/freespace_light_vis.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/certificate.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(run_tests PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties(run_tests PROPERTIES
    COMPILE_FLAGS "-DVIS -DCERTIFY"
)

add_executable(test_curves
	src/test_curves.cpp
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/freespace_light_vis.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/certificate.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(test_curves PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties(test_curves PROPERTIES
  COMPILE_FLAGS "-DVIS -DCERTIFY"
)

add_executable(pruning_progress
	src/pruning_progress.cpp
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/freespace_light_vis.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/certificate.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(pruning_progress PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties(pruning_progress PROPERTIES
  COMPILE_FLAGS "-DVIS -DCERTIFY"
)

add_executable(export_freespace_diagram
	src/export_freespace_diagram.cpp
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/freespace_light_vis.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/certificate.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(export_freespace_diagram PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties(export_freespace_diagram PROPERTIES
  COMPILE_FLAGS "-DVIS -DCERTIFY"
)

add_executable(compare_implementations
	src/compare_implementations.cpp
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/certificate.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(compare_implementations PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties(compare_implementations PROPERTIES
  COMPILE_FLAGS "-DCERTIFY"
)

add_executable(calc_frechet_distance
	src/calc_frechet_distance.cpp
	src/frechet_light.cpp
	src/frechet_naive.cpp
	src/geometry_basics.cpp
	src/filter.cpp
	src/freespace_light_vis.cpp
	src/orth_range_search.cpp
	src/parser.cpp
	src/query.cpp
	src/times.cpp
	src/certificate.cpp
	src/curve.cpp
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(calc_frechet_distance PUBLIC OpenMP::OpenMP_CXX)
endif()
set_target_properties(calc_frechet_distance PROPERTIES
  COMPILE_FLAGS "-DVIS -DCERTIFY"
)

add_executable(performance_test
	src/performance_test.cpp
	$<TARGET_OBJECTS:common>
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(performance_test PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable(paper_experiments
	src/paper_experiments.cpp
	$<TARGET_OBJECTS:common>
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(paper_experiments PUBLIC OpenMP::OpenMP_CXX)
endif()

# add_executable(paper_experiments
#     src/frechet_light.cpp
#     src/frechet_naive.cpp
#     src/geometry_basics.cpp
#     src/filter.cpp
#     src/orth_range_search.cpp
#     src/parser.cpp
#     src/query.cpp
#     src/times.cpp
#     src/certificate.cpp
#     src/paper_experiments.cpp
#     src/curve.cpp
# )
# set_target_properties(paper_experiments PROPERTIES
#   COMPILE_FLAGS "-DCERTIFY"
# )

add_executable(create_benchmark
	src/create_benchmark.cpp
	$<TARGET_OBJECTS:common>
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(create_benchmark PUBLIC OpenMP::OpenMP_CXX)
endif()

add_executable(create_benchmark_decider
	src/create_benchmark_decider.cpp
	$<TARGET_OBJECTS:common>
)
if(OpenMP_CXX_FOUND)
	target_link_libraries(create_benchmark_decider PUBLIC OpenMP::OpenMP_CXX)
endif()


# add_test(NAME unit-test
#     WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src"
#     COMMAND $<TARGET_FILE:run_tests>
# )
