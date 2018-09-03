# target_compile_definitions requires CMake 2.8.11
cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(foreigncc C CXX)

# foreigncc version number format:
#   <major>.<minor>.<patch> <suffix>
# <suffix> could be:
#   "stable" "rc" "beta" "alpha" "unstable"
set(FOREIGNCC_VERSION_MAJOR 0)
set(FOREIGNCC_VERSION_MINOR 0)
set(FOREIGNCC_VERSION_PATCH 1)
set(FOREIGNCC_VERSION_SUFFIX "unstable")


# Third-party libraries
# Directory thirdparty/${THIRDPARTY_XXX} should be found
set(THIRDPARTY_BOOST boost_1_68_0)


# Add source files and third-party libraries to include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/${THIRDPARTY_BOOST})


# Configure config.h and add the destination to include directories
configure_file(
    "src/config.h.cmakein"
    "cmakein/generated/config.h"
    @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/cmakein")


set(COMMON_SOURCE_FILES
    src/common/common.h
    "${CMAKE_CURRENT_BINARY_DIR}/cmakein/generated/config.h")

set(DIST_COMMON_SOURCE_FILES
    src/main.cpp
    ${COMMON_SOURCE_FILES})

add_executable(foreigncc ${DIST_COMMON_SOURCE_FILES})
target_compile_definitions(foreigncc PRIVATE -DBUILD_FOREIGNCC=1)

add_executable(foreignccd ${DIST_COMMON_SOURCE_FILES})
target_compile_definitions(foreignccd PRIVATE -DBUILD_FOREIGNCCD=1)

add_executable(foreignccctl ${DIST_COMMON_SOURCE_FILES})
target_compile_definitions(foreignccctl PRIVATE -DBUILD_FOREIGNCCCTL=1)
