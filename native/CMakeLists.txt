cmake_minimum_required(VERSION 3.15)

project(
    ${SKBUILD_PROJECT_NAME}
    VERSION ${SKBUILD_PROJECT_VERSION}
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common installation directories
include(GNUInstallDirs)

# Use -fPIC even if statically compiled
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIB_INSTALL_DESTINATION "pymusly")

# make sure Eigen3 library is present
include(FetchContent)

set(BUILD_STATIC true)

FetchContent_Declare(libmusly
    GIT_REPOSITORY      https://github.com/andban/musly.git
    GIT_TAG             origin/feat/modernized-build-scripts
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS NO_MODULE
)
FetchContent_MakeAvailable(libmusly)


add_subdirectory(pymusly)