#include "MuslyJukebox.h"
#include "musly_error.h"

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
        throw musly_error("could not get jukebox track size");
    }

    return ret;
}

int
MuslyJukebox::track_count() const
{
    const int ret = musly_jukebox_trackcount(m_jukebox);
    if (ret < 0)
    {
        throw musly_error("could not get jukebox track count");
    }
    return ret;
}

musly_trackid
MuslyJukebox::highest_track_id() const
{
    const int ret = musly_jukebox_maxtrackid(m_jukebox);
    if (ret < 0)
    {
        throw musly_error("could not get last track id from jukebox");
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
        throw musly_error("could not get track ids from jukebox");
    }

    return track_ids;
}

MuslyTrack*
MuslyJukebox::track_from_audiofile(const char* filename, int start, int length)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr)
    {
        throw musly_error("could not allocate track");
    }

    if (musly_track_analyze_audiofile(m_jukebox, filename, length, start, track) < 0)
    {
        std::string message("could not load track from audio file: ");
        message += filename;

        throw musly_error(message);
    }

    return new MuslyTrack(track);
}

MuslyTrack*
MuslyJukebox::track_from_audiodata(const std::vector<float>& pcm_data)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr)
    {
        throw musly_error("could not allocate track");
    }

    if (musly_track_analyze_pcm(m_jukebox, const_cast<float*>(pcm_data.data()), pcm_data.size(), track))
    {
        throw musly_error("could not load track from pcm");
    }

    return new MuslyTrack(track);
}

MuslyJukebox::musly_trackid_vec
MuslyJukebox::add_tracks(const musly_track_vec& tracks, const musly_trackid_vec& track_ids)
{
    if (tracks.size() != track_ids.size())
    {
        throw musly_error("track_list and track_id_list must have same number of elements");
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
        throw musly_error("pymusly: failure while adding tracks to jukebox. maybe set_style was not called before?");
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
        throw musly_error("pymusly: failure while removing tracks from jukebox");
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
        throw musly_error("pymusly: failure while setting style of jukebox");
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
        throw musly_error("pymusly: tracks and track_ids must have same number of items");
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
        throw musly_error("pymusly: failure while computing track similarity");
    }

    return similarities;
}

py::bytes
MuslyJukebox::serialize_track(MuslyTrack* track)
{
    if (!track)
    {
        throw musly_error("pymusly: track must not be none");
    }

    char* bytes = new char[track_size()];

    int err = musly_track_tobin(m_jukebox, track->data(), reinterpret_cast<unsigned char*>(bytes));
    if (err < 0)
    {
        delete [] bytes;
        throw musly_error("pymusly: failed to convert track to bytearray");
    }

    return py::bytes(bytes, track_size());
}

MuslyTrack*
MuslyJukebox::deserialize_track(py::bytes bytes)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr)
    {
        throw musly_error("pymusly: could not allocate track");
    }

    int ret = musly_track_frombin(m_jukebox, reinterpret_cast<unsigned char*>(bytes.ptr()), track);
    if (ret < 0)
    {
        throw musly_error("pymusly: failed to convert bytearray to track");
    }

    return new MuslyTrack(track);
}

void
MuslyJukebox::serialize(std::ostream& o)
{
    const int tracks_per_chunk = 100;
    const int endian = 0x01020304;

    const int header_size = musly_jukebox_binsize(m_jukebox, 1, 0);
    if (header_size < 0)
    {
        throw musly_error("pymusly: could not get jukebox header size");
    }

    // write current musly_version, sizeof(int) and known int value for
    // compatibility checks when deserializing the file at a later point in time
    o << musly_version() << '\0'
      << (uint8_t) sizeof(int);
    o.write(reinterpret_cast<const char*>(&endian), sizeof(int));

    // write method and decoder info
    o << method() << '\0' << decoder() << '\0';

    // write jukebox header together with its size in bytes
    const int total_tracks_to_write = track_count();
    int tracks_to_write;
    int tracks_written = 0;
    int bytes_written;
    const int buffer_length = std::max(header_size, tracks_per_chunk * track_size());
    unsigned char* buffer = new unsigned char[buffer_length];

    std::string error;
    if (musly_jukebox_tobin(m_jukebox, buffer, 1, 0, 0) < 0)
    {
        error = "pymusly: could not serialize jukebox header";
        goto cleanup;
    }
    o.write(reinterpret_cast<const char*>(&header_size), 4);
    o.write(reinterpret_cast<const char*>(buffer), header_size);

    while (tracks_written < total_tracks_to_write)
    {
        tracks_to_write = std::min(tracks_per_chunk, total_tracks_to_write - tracks_written);
        bytes_written = musly_jukebox_tobin(m_jukebox, buffer, 0, tracks_to_write, tracks_written);
        if (bytes_written < 0)
        {
            error = "failed to write data into buffer";
            goto cleanup;
        }
        o.write(reinterpret_cast<const char*>(buffer), bytes_written);
        tracks_written += tracks_to_write;
    }

cleanup:
    delete [] buffer;
    o.flush();

    if (!error.empty())
    {
        throw musly_error(error);
    }
}

MuslyJukebox*
MuslyJukebox::create_from_stream(std::istream& i, bool ignore_decoder)
{
    std::string read_version;
    std::getline(i, read_version, '\0');
    if (read_version.empty() || read_version != musly_version())
    {
        throw musly_error("version not compatible");
    }

    uint8_t int_size;
    i.read(reinterpret_cast<char*>(&int_size), sizeof(uint8_t));
    if (int_size != sizeof(int))
    {
        throw musly_error("invalid integer size");
    }

    unsigned int byte_order;
    i.read(reinterpret_cast<char*>(&byte_order), sizeof(int));
    if (byte_order != 0x01020304)
    {
        throw musly_error("invalid byte order");
    }

    const std::string decoders = musly_jukebox_listdecoders();

    std::string method, decoder;
    std::getline(i, method, '\0');
    std::getline(i, decoder, '\0');
    if (decoder.empty() || decoders.find(decoder) == decoder.npos)
    {
        if (!ignore_decoder)
        {
            throw musly_error("pymusly: decoder not supported with the current libmusly: " + decoder);
        }
        decoder = "";
    }
    MuslyJukebox* jukebox = new MuslyJukebox(
            method.c_str(),
            decoder.empty() ? nullptr : decoder.c_str()
    );

    int track_size = musly_jukebox_binsize(jukebox->m_jukebox, 0, 1);
    int header_size;
    i.read(reinterpret_cast<char*>(&header_size), sizeof(int));

    unsigned char* header = new unsigned char[header_size];
    i.read(reinterpret_cast<char*>(header), header_size);
    int track_count = musly_jukebox_frombin(jukebox->m_jukebox, header, 1, 0);
    delete [] header;
    if (track_count < 0)
    {
        delete jukebox;
        throw musly_error("invalid header");
    }

    const int tracks_per_chunk = 100;
    int buffer_len = track_size * tracks_per_chunk;
    unsigned char* buffer = new unsigned char[buffer_len];
   
    int tracks_read = 0;
    int tracks_to_read = 0;
    while (tracks_read < track_count)
    {
        tracks_to_read = std::min(tracks_per_chunk, track_count - tracks_read);
        i.read(reinterpret_cast<char*>(buffer), tracks_to_read * track_size);
        if (i.fail())
        {
            delete [] buffer;
            delete jukebox;
            throw musly_error("received less tracks than expected");
        }
        if (musly_jukebox_frombin(jukebox->m_jukebox, buffer, 0, tracks_to_read) < 0)
        {
            delete [] buffer;
            delete jukebox;
            throw musly_error("failed to load track information");
        }

        tracks_read += tracks_to_read;
    }

    delete [] buffer;

    return jukebox;
}

void
MuslyJukebox::register_class(py::module_& m)
{
    py::class_<MuslyJukebox>(m, "MuslyJukebox")
        .def(py::init<const char*, const char*>(), py::arg("method") = nullptr, py::arg("decoder") = nullptr)
        .def(py::init(&MuslyJukebox::create_from_stream))
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
        .def("serialize_to_stream", &MuslyJukebox::serialize)
        .def("set_style", &MuslyJukebox::set_style)
        .def("add_tracks", &MuslyJukebox::add_tracks)
        .def("remove_tracks", &MuslyJukebox::remove_tracks)
        .def("compute_similarity", &MuslyJukebox::compute_similarity);
}
