#include "MuslyTrack.h"

#include <musly/musly.h>
#include <iostream>


namespace py = pybind11;


void
MuslyTrack::register_class(py::module_& module)
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

musly_track*
MuslyTrack::data() const
{
    return m_track;
}
