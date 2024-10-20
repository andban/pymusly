# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

set(MUSLY_VERSION "0.2")

add_definitions(-DMUSLY_VERSION="${MUSLY_VERSION}")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

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

find_package(Eigen3 REQUIRED)

if(LINUX)
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil REQUIRED)
else()
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil REQUIRED)
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
    # link against winsock2 for ntohl() and htonl()
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ws2_32)
endif()


add_library(libmusly STATIC ${SOURCES})


target_link_libraries(libmusly
    libresample
    ${ADDITIONAL_LIBRARIES}
)


if(EXISTS "${LIBAV_INCLUDE_DIRS}/libavutil/channel_layout.h")
    set_source_files_properties(decoders/libav.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_AVUTIL_CHANNEL_LAYOUT")
endif()

set_target_properties(libmusly PROPERTIES COMPILE_FLAGS "-DBUILD_STATIC")

target_include_directories(libmusly
    PRIVATE src/ include/ ${EIGEN3_INCLUDE_DIR} ${ADDITIONAL_INCLUDES}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)


if (BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()
