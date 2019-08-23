# CMake build control file for my parallel algorithm experiments
# Copyright 2019 Jeff Trull <edaskel@att.net>

cmake_minimum_required( VERSION 3.13 )

# create a compilation database for clangd etc.
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CMAKE_CXX_STANDARD 17 )

# Release build by default
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Default build type (Debug, Release, RelWithDebInfo)" FORCE )
endif()

# download and use Google Benchmark
include( ExternalProject )
ExternalProject_Add( GBENCH
  GIT_REPOSITORY https://github.com/google/benchmark
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gbench
  CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DBENCHMARK_ENABLE_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property( GBENCH SOURCE_DIR BINARY_DIR )
add_library( gbench STATIC IMPORTED )

# create include directory, which will be absent at initial run
execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${SOURCE_DIR}/include )
set_target_properties( gbench PROPERTIES
  IMPORTED_LOCATION "${BINARY_DIR}/src/libbenchmark.a"
  INTERFACE_INCLUDE_DIRECTORIES "${SOURCE_DIR}/include"
  INTERFACE_LINK_LIBRARIES pthread
)
add_dependencies( gbench GBENCH )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror" )

# First executable: benchmarks std::accumulate (read only) vs std::partial_sum (intermediate writes)
add_executable( avps acc_vs_partsum.cpp )
target_link_libraries( avps gbench )

# Second executable: introduce an additional thread to do accumulate on first half
add_executable( is2t inclusive_scan_2t.cpp )
target_link_libraries( is2t gbench )

# Third: the same, but multiple threads for a family of curves
add_executable( isnt inclusive_scan_nt.cpp )
target_link_libraries( isnt gbench )

