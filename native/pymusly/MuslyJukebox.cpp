#include "MuslyJukebox.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <musly/musly.h>
#include <exception>
#include <string>

#include <iostream>


namespace py = pybind11;

MuslyJukebox::MuslyJukebox(const char* method, const char* decoder)
{
    m_jukebox = musly_jukebox_poweron(method, decoder);
}

MuslyJukebox::~MuslyJukebox()
{
    if (this->m_jukebox)
    {
        musly_jukebox_poweroff(this->m_jukebox);
        m_jukebox = nullptr;
    }

}

const char*
MuslyJukebox::method() const
{
    return musly_jukebox_methodname(m_jukebox);
}

const char*
MuslyJukebox::method_info() const
{
    return musly_jukebox_aboutmethod(m_jukebox);
}

const char*
MuslyJukebox::decoder() const
{
    return musly_jukebox_decodername(m_jukebox);
}

int
MuslyJukebox::track_size() const
{
    const int ret = musly_track_binsize(m_jukebox);
    if (ret < 0)
    {
        throw std::runtime_error("could not get jukebox track size");
    }

    return ret;
}

int
MuslyJukebox::track_count() const
{
    const int ret = musly_jukebox_trackcount(m_jukebox);
    if (ret < 0)
    {
        throw std::runtime_error("could not get jukebox track count");
    }
    return ret;
}

musly_trackid
MuslyJukebox::highest_track_id() const
{
    const int ret = musly_jukebox_maxtrackid(m_jukebox);
    if (ret < 0)
    {
        throw std::runtime_error("could not get last track id from jukebox");
    }
    return ret;
}

MuslyJukebox::musly_trackid_vec
MuslyJukebox::track_ids() const
{
    MuslyJukebox::musly_trackid_vec track_ids(track_count());
    const int ret = musly_jukebox_gettrackids(m_jukebox, track_ids.data());
    if (ret < 0)
    {
        throw std::runtime_error("could not get track ids from jukebox");
    }

    return track_ids;
}

MuslyTrack*
MuslyJukebox::track_from_audiofile(const char* filename, int start, int length)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr)
    {
        throw std::runtime_error("could not allocate track");
    }

    if (musly_track_analyze_audiofile(m_jukebox, filename, start, length, track) < 0)
    {
        std::string message("could not load track from audio file: ");
        message += filename;

        throw std::runtime_error(message);
    }

    return new MuslyTrack(track);
}

MuslyTrack*
MuslyJukebox::track_from_audiodata(const std::vector<float>& pcm_data)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr)
    {
        throw std::runtime_error("could not allocate track");
    }

    if (musly_track_analyze_pcm(m_jukebox, const_cast<float*>(pcm_data.data()), pcm_data.size(), track))
    {
        throw std::runtime_error("could not load track from pcm");
    }

    return new MuslyTrack(track);
}

MuslyJukebox::musly_trackid_vec
MuslyJukebox::add_tracks(const musly_track_vec& tracks, const musly_trackid_vec& track_ids)
{
    if (tracks.size() != track_ids.size())
    {
        throw std::length_error("track_list and track_id_list must have same number of elements");
    }

    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(
            tracks.begin(),
            tracks.end(),
            musly_tracks.begin(),
            [](MuslyTrack* t) { return t->data(); });

    int ret = musly_jukebox_addtracks(
        m_jukebox,
        const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()),
        tracks.size(),
        0
    );
    if (ret < 0)
    {
        throw std::runtime_error("pymusly: failure while adding tracks to jukebox. maybe set_style was not called before?");
    }

    return track_ids;
}

void
MuslyJukebox::remove_tracks(const musly_trackid_vec& track_ids)
{
    if (musly_jukebox_removetracks(
            m_jukebox,
            const_cast<musly_trackid*>(track_ids.data()),
            track_ids.size()
    ) < 0) {
        throw std::runtime_error("pymusly: failure while removing tracks from jukebox");
    }
}

void
MuslyJukebox::set_style(const musly_track_vec& tracks)
{
    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(
            tracks.begin(),
            tracks.end(),
            musly_tracks.begin(),
            [](MuslyTrack* t) { return t->data(); });

    int ret = musly_jukebox_setmusicstyle(
            m_jukebox,
            const_cast<musly_track**>(musly_tracks.data()),
            tracks.size());
    if (ret < 0)
    {
        throw std::runtime_error("pymusly: failure while setting style of jukebox");
    }
}

std::vector<float>
MuslyJukebox::compute_similarity(MuslyTrack* seed_track,
                                 musly_trackid seed_track_id,
                                 const musly_track_vec& tracks,
                                 const musly_trackid_vec& track_ids)
{
    if (tracks.size() != track_ids.size())
    {
        throw std::length_error("pymusly: tracks and track_ids must have same number of items");
    }

    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(
            tracks.begin(),
            tracks.end(),
            musly_tracks.begin(),
            [](MuslyTrack* t) { return t->data(); });

    std::vector<float> similarities(tracks.size(), 0.0f);
    int ret = musly_jukebox_similarity(
        m_jukebox,
        seed_track->data(),
        seed_track_id,
        const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()),
        tracks.size(),
        similarities.data()
    );
    if (ret < 0)
    {
        throw std::runtime_error("pymusly: failure while computing track similarity");
    }

    return similarities;
}

py::bytes
MuslyJukebox::serialize_track(MuslyTrack* track)
{
    if (!track)
    {
        throw std::invalid_argument("pymusly: track must not be none");
    }

    char* bytes = new char[track_size()];

    int err = musly_track_tobin(m_jukebox, track->data(), reinterpret_cast<unsigned char*>(bytes));
    if (err < 0)
    {
        delete [] bytes;
        throw std::runtime_error("pymusly: failed to convert track to bytearray");
    }

    return py::bytes(bytes, track_size());
}

MuslyTrack*
MuslyJukebox::deserialize_track(py::bytes bytes)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr)
    {
        throw std::runtime_error("could not allocate track");
    }

    int ret = musly_track_frombin(m_jukebox, reinterpret_cast<unsigned char*>(bytes.ptr()), track);
    if (ret < 0)
    {
        throw std::runtime_error("pymusly: failed to convert bytearray to track");
    }

    return new MuslyTrack(track);
}

void
init_MuslyJukebox(py::module_& m)
{

    py::class_<MuslyJukebox>(m, "MuslyJukebox")
        .def(py::init<const char*, const char*>(), py::arg("method") = nullptr, py::arg("decoder") = nullptr)
        .def_property_readonly("method", &MuslyJukebox::method)
        .def_property_readonly("method_info", &MuslyJukebox::method_info)
        .def_property_readonly("decoder", &MuslyJukebox::decoder)
        .def_property_readonly("track_size", &MuslyJukebox::track_size)
        .def_property_readonly("track_count", &MuslyJukebox::track_count)
        .def_property_readonly("highest_track_id", &MuslyJukebox::highest_track_id)
        .def_property_readonly("track_ids", &MuslyJukebox::track_ids)
        .def("track_from_audiofile", &MuslyJukebox::track_from_audiofile, py::return_value_policy::take_ownership)
        .def("track_from_audiodata", &MuslyJukebox::track_from_audiodata, py::return_value_policy::take_ownership)
        .def("serialize_track", &MuslyJukebox::serialize_track, py::return_value_policy::take_ownership)
        .def("deserialize_track", &MuslyJukebox::deserialize_track, py::return_value_policy::take_ownership)
        .def("set_style", &MuslyJukebox::set_style)
        .def("add_tracks", &MuslyJukebox::add_tracks)
        .def("remove_tracks", &MuslyJukebox::remove_tracks)
        .def("compute_similarity", &MuslyJukebox::compute_similarity);
}
