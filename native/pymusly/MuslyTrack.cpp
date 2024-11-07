#include "MuslyTrack.h"

#include <iostream>
#include <musly/musly.h>

namespace py = pybind11;

namespace pymusly {

void MuslyTrack::register_class(py::module_& module)
{
    py::class_<MuslyTrack>(module, "MuslyTrack", "Musly track data");
}

MuslyTrack::MuslyTrack(musly_track* track)
    : m_track(track)
{
    // empty
}

MuslyTrack::~MuslyTrack()
{
    musly_track_free(m_track);
    m_track = nullptr;
}

musly_track* MuslyTrack::data() const
{
    return m_track;
}

} // namespace pymusly
