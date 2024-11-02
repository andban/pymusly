# cmake-format: off
python_add_library(_pymusly
    MODULE
        common.h
        BytesIO.h
        main.cpp
        musly_error.h
        MuslyJukebox.cpp
        MuslyJukebox.h
        MuslyTrack.cpp
        MuslyTrack.h
    WITH_SOABI
)
# cmake-format: on

target_link_libraries(_pymusly PRIVATE pybind11::headers Musly::libmusly)
target_include_directories(_pymusly PRIVATE Musly::libmusly)

target_compile_definitions(_pymusly PRIVATE VERSION_INFO=${PROJECT_VERSION})

if(${CMAKE_BUILD_TYPE} STREQUAL "Profile" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(_pymusly PRIVATE --coverage)
  target_link_options(_pymusly PRIVATE --coverage)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # cmake-format: off
  target_link_options(_pymusly
    PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic,--whole-archive
        -lwinpthread
        -Wl,--no-whole-archive
  )
  # cmake-format: on
endif()

if(APPLE)
  set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "@loader_path")
else()
  set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()

set_target_properties(_pymusly PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

install(
  TARGETS _pymusly
  LIBRARY DESTINATION pymusly
  RUNTIME DESTINATION pymusly
)

install(FILES $<TARGET_FILE:Musly::libmusly> DESTINATION pymusly)

if(DEFINED musly_SOURCE_DIR AND ${SKBUILD_STATE} STREQUAL "wheel")
  install(
    FILES ${musly_SOURCE_DIR}/COPYING
    DESTINATION ${SKBUILD_METADATA_DIR}/licenses
    RENAME LICENSE.musly.txt
  )
  install(
    FILES ${musly_SOURCE_DIR}/libresample/LICENSE-BSD.txt
    DESTINATION ${SKBUILD_METADATA_DIR}/licenses
    RENAME LICENSE.resample.txt
  )
  install(
    FILES ${musly_SOURCE_DIR}/libkissfft/COPYING
    DESTINATION ${SKBUILD_METADATA_DIR}/licenses
    RENAME LICENSE.kissfft.txt
  )
endif()
