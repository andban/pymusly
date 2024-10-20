#include "MuslyTrack.h"
#include "MuslyJukebox.h"

#include <pybind11/pybind11.h>
#include <musly/musly.h>


#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;


PYBIND11_MODULE(_libmusly, m) {
    m.doc() = R"pbdoc(

    )pbdoc";

    m.def("musly_version", musly_version, R"pbdoc(
        Return the version of Musly.
    )pbdoc");

    m.def("musly_debug", musly_debug, R"pbdoc(
        Set the musly debug level.

        Valid levels are 0 (Quiet, DEFAULT), 1 (Error), 2 (Warning), 3 (Info), 4 (Debug), 5 (Trace). All output will be sent to stderr.
    )pbdoc");

    m.def("musly_jukebox_listmethods", musly_jukebox_listmethods, R"pbdoc(
        Lists all available music similarity methods.

        Use a method name to power on a Musly jukebox.
    )pbdoc");

    m.def("musly_jukebox_listdecoders", musly_jukebox_listdecoders, R"pbdoc(
            Lists all available audio file decoders.

            Use a decoder name to power on a Musly jukebox.
    )pbdoc");

    init_MuslyJukebox(m);
    MuslyTrack::register_class(m);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}