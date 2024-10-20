find_file(HAVE_INTTYPES inttypes.h)
configure_file(src/config.h.in config.h)

add_library(libresample STATIC
    src/filterkit.c
    src/resamplesubs.c
    src/resample.c
)

target_include_directories(libresample
    PRIVATE include/ src/ "${CMAKE_CURRENT_BINARY_DIR}"
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#install(TARGETS libresample DESTINATION ${LIB_INSTALL_DESTINATION})
