python_add_library(_pymusly MODULE main.cpp MuslyJukebox.cpp MuslyTrack.cpp WITH_SOABI)

target_link_libraries(_pymusly PRIVATE pybind11::headers Musly::libmusly)
target_include_directories(_pymusly PRIVATE Musly::libmusly)

target_compile_definitions(_pymusly PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(
  TARGETS _pymusly
  LIBRARY DESTINATION pymusly
  RUNTIME DESTINATION pymusly
)

install(FILES $<TARGET_FILE:Musly::libmusly> DESTINATION pymusly)
set_target_properties(_pymusly PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

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

if(APPLE)
  set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "@loader_path")
else()
  set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()
