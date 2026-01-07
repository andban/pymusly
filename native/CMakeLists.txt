cmake_minimum_required(VERSION 3.23)

project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Common installation directories
include(GNUInstallDirs)

# Use -fPIC even if statically compiled
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATIONS OFF)
endif()

set(LIB_INSTALL_DESTINATION "pymusly")

# make sure Musly library is present
include(FetchContent)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

set(MUSLY_USE_OWN_LIBRESAMPLE TRUE)
set(MUSLY_USE_OWN_LIBKISSFFT TRUE)
FetchContent_Declare(
  Musly
  URL https://github.com/andban/musly/archive/refs/tags/v2.0-cmake.6.tar.gz
  URL_HASH SHA256=223208b5e64075c92d09163b8293905d6bd517ee31e26c98e492679936a711f1
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      CMAKE_ARGS -DCMAKE_INTERPROCEDURAL_OPTIMIZATIONS=OFF EXCLUDE_FROM_ALL FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Musly)

add_subdirectory(pymusly)
