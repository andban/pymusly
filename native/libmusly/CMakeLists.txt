# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

set(MUSLY_VERSION "0.2")

add_definitions(-DMUSLY_VERSION="${MUSLY_VERSION}")

set(SOURCES
    src/kissfft/kiss_fft.c
    src/kissfft/kiss_fftr.c
    src/methods/mandelellis.cpp
    src/methods/timbre.cpp
    src/resampler.cpp
    src/plugins.cpp
    src/method.cpp
    src/decoder.cpp
    src/windowfunction.cpp
    src/powerspectrum.cpp
    src/melspectrum.cpp
    src/discretecosinetransform.cpp
    src/mfcc.cpp
    src/gaussianstatistics.cpp
    src/mutualproximity.cpp
    src/lib.cpp
)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(NOT TARGET Eigen3::Eigen)
    set(EIGEN3_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/vendor/eigen3)
    include(ExternalProject)
    ExternalProject_Add(eigen3
        URL      https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
        URL_HASH SHA256=8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        PREFIX   ${EIGEN3_INSTALL_PREFIX}
        CMAKE_ARGS
            "-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/vendor"
        UPDATE_DISCONNECTED 1
    )
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    set_target_properties(Eigen3::Eigen PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/vendor/include/eigen3)
endif()


if(LINUX)
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil REQUIRED)
else()
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil)
endif()

if(LIBAV_FOUND)
    set(SOURCES ${SOURCES} src/decoders/libav.cpp)
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ${LIBAV_LIBRARIES})
    set(ADDITIONAL_INCLUDES ${LIBAV_INCLUDE_DIRS})
    add_definitions(-DMUSLY_DECODER_LIBAV)
endif()

if(APPLE)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    mark_as_advanced(AUDIOTOOLBOX_LIBRARY COREFOUNDATION_LIBRARY)

    set(SOURCES ${SOURCES} src/decoders/coreaudio.cpp)
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ${AUDIOTOOLBOX_LIBRARY} ${COREFOUNDATION_LIBRARY})
    add_definitions(-DMUSLY_DECODER_COREAUDIO)
endif()

if(WIN32 OR MINGW)
    set(SOURCES ${SOURCES} src/decoders/mediafoundation.cpp)
    add_definitions(-DMUSLY_DECODER_MEDIAFOUNDATION -DNOMINMAX -D_USE_MATH_DEFINES)
    # link against winsock2 for ntohl() and htonl()
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ws2_32 Mf Mfplat mfreadwrite mfuuid ole32)
endif()


add_library(libmusly STATIC ${SOURCES})
install(TARGETS libmusly DESTINATION pymusly)

target_link_libraries(libmusly
    libresample
    Eigen3::Eigen
    ${ADDITIONAL_LIBRARIES}
)

if(EXISTS "${LIBAV_INCLUDE_DIRS}/libavutil/channel_layout.h")
    set_source_files_properties(decoders/libav.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_AVUTIL_CHANNEL_LAYOUT")
endif()

set_target_properties(libmusly PROPERTIES COMPILE_FLAGS "-DMUSLY_STATIC -DMUSLY_BUILDING_LIBRARY")

target_include_directories(libmusly
    PRIVATE src/ include/ Eigen3::Eigen ${ADDITIONAL_INCLUDES}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)


if (BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()
