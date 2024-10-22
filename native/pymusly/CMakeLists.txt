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
    PRIVATE VERSION_INFO=${PROJECT_VERSION} -DMUSLY_STATIC)

install(TARGETS _pymusly DESTINATION pymusly)
