find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

python_add_library(_libmusly
    MODULE
        main.cpp
        MuslyJukebox.cpp
        MuslyTrack.cpp
    WITH_SOABI
)
target_link_libraries(_libmusly PRIVATE pybind11::headers libmusly)


target_compile_definitions(_libmusly PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS _libmusly DESTINATION pymusly)
