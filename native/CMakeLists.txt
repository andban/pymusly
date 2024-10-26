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

# Common installation directories
include(GNUInstallDirs)

# Use -fPIC even if statically compiled
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIB_INSTALL_DESTINATION "pymusly")

# make sure Musly library is present
include(FetchContent)

FetchContent_Declare(Musly
    GIT_REPOSITORY      https://github.com/andban/musly.git
    GIT_TAG             origin/feat/modernized-build-scripts
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(Musly)


add_subdirectory(pymusly)
