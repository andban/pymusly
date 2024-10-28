#include "MuslyTrack.h"
#include "MuslyJukebox.h"
#include "musly_error.h"

#include <pybind11/pybind11.h>
#include <musly/musly.h>


#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(_pymusly, m) {
    py::options options;
    //options.disable_function_signatures();

    m.def("get_musly_version", musly_version, R"pbdoc(
        Return the version of Musly.
    )pbdoc");

    m.def("set_musly_loglevel", musly_debug, py::arg("level"), R"pbdoc(
        Set the musly debug level.

        Valid levels are 0 (Quiet, DEFAULT), 1 (Error), 2 (Warning), 3 (Info), 4 (Debug), 5 (Trace). All output will be sent to stderr.
    )pbdoc");

    m.def("musly_jukebox_listmethods", musly_jukebox_listmethods, R"pbdoc(
        All available music similarity methods as comma separated string.

        Use a method name to create a Musly jukebox.
        The methods are used to analyze sample data in MuslyJukebox#track_from_audiofile and MuslyJukebox#track_from_audiodata
    )pbdoc");

    m.def("musly_jukebox_listdecoders", musly_jukebox_listdecoders, R"pbdoc(
            All available audio file decoders as comma separated string.

            Use a decoder name to create a MuslyJukebox.
            The decoders are used to load sample data for analysis in MuslyJukebox#track_from_audiofile.
    )pbdoc");

    MuslyJukebox::register_class(m);
    MuslyTrack::register_class(m);
    musly_error::register_with_module(m);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}