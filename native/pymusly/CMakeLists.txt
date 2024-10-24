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
    PRIVATE pybind11::headers libmusly
)
target_include_directories(_pymusly
    PRIVATE libmusly)
target_compile_definitions(_pymusly
    PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS _pymusly DESTINATION pymusly)

if (DEFINED ENV{CIBUILDWHEEL})
    install(IMPORTED_RUNTIME_ARTIFACTS libmusly LIBRARY DESTINATION pymusly)
    set_target_properties(_pymusly PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    install(FILES ${libmusly_SOURCE_DIR}/COPYING
        DESTINATION ${SKBUILD_METADATA_DIR}/licenses
        RENAME LICENSE.libmusly.txt)
endif()

if(APPLE)
    set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "@loader_path")
else()
    set_target_properties(_pymusly PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()
