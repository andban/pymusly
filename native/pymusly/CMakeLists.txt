find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

python_add_library(_pymusly
    MODULE
        main.cpp
        MuslyJukebox.cpp
        MuslyTrack.cpp
    WITH_SOABI
)
target_link_libraries(_pymusly
    PRIVATE
        pybind11::headers
        Musly::libmusly
)
target_include_directories(_pymusly
    PRIVATE Musly::libmusly
)

target_compile_definitions(_pymusly
    PRIVATE VERSION_INFO=${PROJECT_VERSION}
)

install(TARGETS _pymusly
    LIBRARY DESTINATION pymusly
    RUNTIME DESTINATION pymusly
)

install(FILES $<TARGET_FILE:libmusly> DESTINATION pymusly)
set_target_properties(_pymusly PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

if (${SKBUILD_STATE} STREQUAL "wheel")
    install(FILES ${musly_SOURCE_DIR}/COPYING
        DESTINATION ${SKBUILD_METADATA_DIR}/licenses
        RENAME LICENSE.musly.txt)
endif()

if(APPLE)
    set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "@loader_path")
else()
    set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()
