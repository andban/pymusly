#include "MuslyJukebox.h"
#include "musly_error.h"

#include <exception>
#include <musly/musly.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>

#include <iostream>

namespace py = pybind11;

namespace {

const uint32_t _ENDIAN_MAGIC_NUMBER = 0x01020304;

} // namespace

MuslyJukebox::MuslyJukebox(const char* method, const char* decoder)
{
    m_jukebox = musly_jukebox_poweron(method, decoder);
    if (m_jukebox == nullptr) {
        throw musly_error("failed to initialize musly jukebox");
    }
}

MuslyJukebox::~MuslyJukebox()
{
    if (m_jukebox != nullptr) {
        musly_jukebox_poweroff(m_jukebox);
        m_jukebox = nullptr;
    }
}

const char* MuslyJukebox::method() const
{
    return musly_jukebox_methodname(m_jukebox);
}

const char* MuslyJukebox::method_info() const
{
    return musly_jukebox_aboutmethod(m_jukebox);
}

const char* MuslyJukebox::decoder() const
{
    return musly_jukebox_decodername(m_jukebox);
}

int MuslyJukebox::track_size() const
{
    const int ret = musly_track_binsize(m_jukebox);
    if (ret < 0) {
        throw musly_error("could not get jukebox track size");
    }

    return ret;
}

int MuslyJukebox::track_count() const
{
    const int ret = musly_jukebox_trackcount(m_jukebox);
    if (ret < 0) {
        throw musly_error("could not get jukebox track count");
    }
    return ret;
}

musly_trackid MuslyJukebox::highest_track_id() const
{
    const int ret = musly_jukebox_maxtrackid(m_jukebox);
    if (ret < 0) {
        throw musly_error("could not get last track id from jukebox");
    }
    return ret;
}

MuslyJukebox::musly_trackid_vec MuslyJukebox::track_ids() const
{
    MuslyJukebox::musly_trackid_vec track_ids(track_count());
    const int ret = musly_jukebox_gettrackids(m_jukebox, track_ids.data());
    if (ret < 0) {
        throw musly_error("could not get track ids from jukebox");
    }

    return track_ids;
}

MuslyTrack* MuslyJukebox::track_from_audiofile(const char* filename, int length, int start)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr) {
        throw musly_error("could not allocate track");
    }

    if (musly_track_analyze_audiofile(m_jukebox, filename, length, start, track) < 0) {
        std::string message("could not load track from audio file: ");
        message += filename;

        throw musly_error(message);
    }

    return new MuslyTrack(track);
}

MuslyTrack* MuslyJukebox::track_from_audiodata(const std::vector<float>& pcm_data)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr) {
        throw musly_error("could not allocate track");
    }

    if (musly_track_analyze_pcm(m_jukebox, const_cast<float*>(pcm_data.data()), pcm_data.size(), track) < 0) {
        throw musly_error("could not load track from pcm");
    }

    return new MuslyTrack(track);
}

MuslyJukebox::musly_trackid_vec MuslyJukebox::add_tracks(const musly_track_vec& tracks,
    const musly_trackid_vec& track_ids)
{
    if (tracks.size() != track_ids.size()) {
        throw musly_error("track_list and track_id_list must have same number of elements");
    }

    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(tracks.begin(), tracks.end(), musly_tracks.begin(), [](MuslyTrack* track) { return track->data(); });

    int ret = musly_jukebox_addtracks(m_jukebox, const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()), tracks.size(), 0);
    if (ret < 0) {
        throw musly_error("pymusly: failure while adding tracks to jukebox. "
                          "maybe set_style was not called before?");
    }

    return track_ids;
}

void MuslyJukebox::remove_tracks(const musly_trackid_vec& track_ids)
{
    if (musly_jukebox_removetracks(m_jukebox, const_cast<musly_trackid*>(track_ids.data()), track_ids.size()) < 0) {
        throw musly_error("pymusly: failure while removing tracks from jukebox");
    }
}

void MuslyJukebox::set_style(const musly_track_vec& tracks)
{
    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(tracks.begin(), tracks.end(), musly_tracks.begin(), [](MuslyTrack* track) { return track->data(); });

    int ret = musly_jukebox_setmusicstyle(m_jukebox, const_cast<musly_track**>(musly_tracks.data()), tracks.size());
    if (ret < 0) {
        throw musly_error("pymusly: failure while setting style of jukebox");
    }
}

std::vector<float> MuslyJukebox::compute_similarity(MuslyTrack* seed_track, musly_trackid seed_track_id,
    const musly_track_vec& tracks, const musly_trackid_vec& track_ids)
{
    if (tracks.size() != track_ids.size()) {
        throw musly_error("pymusly: tracks and track_ids must have same number of items");
    }

    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(tracks.begin(), tracks.end(), musly_tracks.begin(), [](MuslyTrack* track) { return track->data(); });

    std::vector<float> similarities(tracks.size(), 0.0F);
    int ret = musly_jukebox_similarity(
        m_jukebox, seed_track->data(), seed_track_id, const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()), tracks.size(), similarities.data());
    if (ret < 0) {
        throw musly_error("pymusly: failure while computing track similarity");
    }

    return similarities;
}

py::bytes MuslyJukebox::serialize_track(MuslyTrack* track)
{
    if (track == nullptr) {
        throw musly_error("pymusly: track must not be none");
    }

    char* bytes = new char[track_size()];
    int err = musly_track_tobin(m_jukebox, track->data(), reinterpret_cast<unsigned char*>(bytes));
    if (err < 0) {
        delete[] bytes;
        throw musly_error("pymusly: failed to convert track to bytearray");
    }

    return py::bytes(bytes, track_size());
}

MuslyTrack* MuslyJukebox::deserialize_track(py::bytes bytes)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr) {
        throw musly_error("pymusly: could not allocate track");
    }

    int ret = musly_track_frombin(m_jukebox, reinterpret_cast<unsigned char*>(bytes.ptr()), track);
    if (ret < 0) {
        throw musly_error("pymusly: failed to convert bytearray to track");
    }

    return new MuslyTrack(track);
}

void MuslyJukebox::serialize(std::ostream& out_stream)
{
    const int tracks_per_chunk = 100;

    const int header_size = musly_jukebox_binsize(m_jukebox, 1, 0);
    if (header_size < 0) {
        throw musly_error("pymusly: could not get jukebox header size");
    }

    // write current musly_version, sizeof(int) and known int value for
    // compatibility checks when deserializing the file at a later point in time
    out_stream << musly_version() << '\0' << (uint8_t)sizeof(int);
    out_stream.write(reinterpret_cast<const char*>(&_ENDIAN_MAGIC_NUMBER), sizeof(int));

    // write method and decoder info
    out_stream << method() << '\0' << decoder() << '\0';

    // write jukebox header together with its size in bytes
    const int total_tracks_to_write = track_count();
    int tracks_to_write;
    int tracks_written = 0;
    int bytes_written;
    const int buffer_length = std::max(header_size, tracks_per_chunk * track_size());
    unsigned char* buffer = new unsigned char[buffer_length];

    std::string error;
    if (musly_jukebox_tobin(m_jukebox, buffer, 1, 0, 0) < 0) {
        error = "pymusly: could not serialize jukebox header";
        goto cleanup;
    }
    out_stream.write(reinterpret_cast<const char*>(&header_size), 4);
    out_stream.write(reinterpret_cast<const char*>(buffer), header_size);

    while (tracks_written < total_tracks_to_write) {
        tracks_to_write = std::min(tracks_per_chunk, total_tracks_to_write - tracks_written);
        bytes_written = musly_jukebox_tobin(m_jukebox, buffer, 0, tracks_to_write, tracks_written);
        if (bytes_written < 0) {
            error = "failed to write data into buffer";
            goto cleanup;
        }
        out_stream.write(reinterpret_cast<const char*>(buffer), bytes_written);
        tracks_written += tracks_to_write;
    }

cleanup:
    delete[] buffer;
    out_stream.flush();

    if (!error.empty()) {
        throw musly_error(error);
    }
}

MuslyJukebox* MuslyJukebox::create_from_stream(std::istream& in_stream, bool ignore_decoder)
{
    std::string read_version;
    std::getline(in_stream, read_version, '\0');
    if (read_version.empty() || read_version != musly_version()) {
        throw musly_error("version not compatible");
    }

    uint8_t int_size;
    in_stream.read(reinterpret_cast<char*>(&int_size), sizeof(uint8_t));
    if (int_size != sizeof(int)) {
        throw musly_error("invalid integer size");
    }

    unsigned int byte_order;
    in_stream.read(reinterpret_cast<char*>(&byte_order), sizeof(int));
    if (byte_order != _ENDIAN_MAGIC_NUMBER) {
        throw musly_error("invalid byte order");
    }

    const std::string decoders = musly_jukebox_listdecoders();

    std::string method;
    std::getline(in_stream, method, '\0');

    std::string decoder;
    std::getline(in_stream, decoder, '\0');

    if (decoder.empty() || decoders.find(decoder) == std::string::npos) {
        if (!ignore_decoder) {
            throw musly_error("pymusly: decoder not supported with the current libmusly: " + decoder);
        }
        decoder = "";
    }
    MuslyJukebox* jukebox = new MuslyJukebox(method.c_str(), decoder.empty() ? nullptr : decoder.c_str());

    int track_size = musly_jukebox_binsize(jukebox->m_jukebox, 0, 1);
    int header_size;
    in_stream.read(reinterpret_cast<char*>(&header_size), sizeof(int));

    unsigned char* header = new unsigned char[header_size];
    in_stream.read(reinterpret_cast<char*>(header), header_size);
    int track_count = musly_jukebox_frombin(jukebox->m_jukebox, header, 1, 0);
    delete[] header;
    if (track_count < 0) {
        delete jukebox;
        throw musly_error("invalid header");
    }

    const int tracks_per_chunk = 100;
    int buffer_len = track_size * tracks_per_chunk;
    unsigned char* buffer = new unsigned char[buffer_len];

    int tracks_read = 0;
    int tracks_to_read = 0;
    while (tracks_read < track_count) {
        tracks_to_read = std::min(tracks_per_chunk, track_count - tracks_read);
        in_stream.read(reinterpret_cast<char*>(buffer), tracks_to_read * track_size);
        if (in_stream.fail()) {
            delete[] buffer;
            delete jukebox;
            throw musly_error("received less tracks than expected");
        }
        if (musly_jukebox_frombin(jukebox->m_jukebox, buffer, 0, tracks_to_read) < 0) {
            delete[] buffer;
            delete jukebox;
            throw musly_error("failed to load track information");
        }

        tracks_read += tracks_to_read;
    }

    delete[] buffer;

    return jukebox;
}

void MuslyJukebox::register_class(py::module_& module)
{

    py::class_<MuslyJukebox>(module, "MuslyJukebox")
        .def(py::init<const char*, const char*>(), py::arg("method") = nullptr, py::arg("decoder") = nullptr, R"pbdoc(
            Create a new jukebox instance using the given analysis method and audio decoder.

            For a list of supported analysis methods and audio decoders, you can call pymusly.get_musly_methods / pymusly.get_musly_decoders.

            :param method:
                the method to use for audio data analysis.
                Call pymusly.get_musly_methods() to get a list of available options.
                If `None` is given, the default method is used.
            :param decoder:
                the decoder to use to analyze audio data loaded from files.
                Call pymusly.get_musly_decoders() to get a list of available options.
                If `None`, a default decoder is used.
        )pbdoc")

        .def_static("create_from_stream", &MuslyJukebox::create_from_stream, py::arg("input_stream"),
            py::arg("ignore_decoder"), py::return_value_policy::take_ownership, R"pbdoc(
            Load previously serialized MuslyJukebox from an io.BytesIO stream.

            :param stream:
                an readable binary stream, like the result of `open('electronic-music.jukebox', 'rb')`.
            :param ignore_decoder:
                when `True`, the resulting jukebox will use the default decoder, in case the original decoder is not available.

            :return: the deserialized jukebox
            :rtype: MuslyJukebox
            :raises MuslyError: if the deserialization failed
        )pbdoc")

        .def_property_readonly("method", &MuslyJukebox::method, R"pbdoc(
            The method for audio data analysis used by this jukebox instance.
        )pbdoc")

        .def_property_readonly("method_info", &MuslyJukebox::method_info, R"pbdoc(
            A description of the analysis method used by this jukebox instance.
        )pbdoc")

        .def_property_readonly("decoder", &MuslyJukebox::decoder, R"pbdoc(
            The decoder for reading audio files used by this jukebox instance.
        )pbdoc")

        .def_property_readonly("track_size", &MuslyJukebox::track_size, R"pbdoc(
            The size in bytes of MuslyTrack instances created by this jukebox instance.
        )pbdoc")

        .def_property_readonly("track_count", &MuslyJukebox::track_count, R"pbdoc(
            The number of tracks that were added to this jukebox using the add_tracks method.
        )pbdoc")

        .def_property_readonly("highest_track_id", &MuslyJukebox::highest_track_id, R"pbdoc(
            The highest track id that was assigned to tracks added to this jukebox instance.
        )pbdoc")

        .def_property_readonly("track_ids", &MuslyJukebox::track_ids, R"pbdoc("
            A list of all track ids assigned to tracks added to this jukebox instance.
        )pbdoc")

        .def("track_from_audiofile", &MuslyJukebox::track_from_audiofile, py::arg("input_stream"), py::arg("length"),
            py::arg("start"), py::return_value_policy::take_ownership, R"pbdoc(
            Create a MuslyTrack by analysing an excerpt of the given audio file.

            The audio file is decoded by using the decoder selected during MuslyJukebox creation. The decoded audio signal is then down- and resampled into a 20,050Hz mono signal which is used as input for track_from_audiodata().

            :param input_stream:
                an input stream to the audio file to decode, like the result of `open('test.mp3', 'rb')`.
            :param ignore_decoder:
                when True, the resulting jukebox will use the default decoder, when the original decoder is not available.
        )pbdoc")

        .def("track_from_audiodata", &MuslyJukebox::track_from_audiodata, py::arg("pcm_data"),
            py::return_value_policy::take_ownership, R"pbdoc(
            Create a MuslyTrack by analyzing the provided PCM samples.

            The input samples are expected to represent a mono signal with 22050Hz sample rate using float values.

            :param pcm_data:
                the sample data to analyze.
        )pbdoc")

        .def("serialize_track", &MuslyJukebox::serialize_track, py::arg("track"),
            py::return_value_policy::take_ownership, R"pbdoc(
            Serialize a MuslyTrack into a `bytes` object.

            :param track:
                a MuslyTrack object.
        )pbdoc")

        .def("deserialize_track", &MuslyJukebox::deserialize_track, py::arg("bytes_track"),
            py::return_value_policy::take_ownership, R"pbdoc(
            Deserialize a MuslyTrack from a `bytes` object.

            :param bytes_track:
                a previously with :func:`serialize_track` serialized MuslyTrack.
        )pbdoc")

        .def("serialize_to_stream", &MuslyJukebox::serialize, py::arg("output_stream"), R"pbdoc(
            Serialize jukebox instance into a `io.BytesIO` stream`.

            :param output_stream:
                an output stream, like one created by `open('electronic-music.jukebox', 'wb')`.
        )pbdoc")

        .def("set_style", &MuslyJukebox::set_style, py::arg("tracks"), R"pbdoc(
            Initialize jukebox with a set of tracks that are used as reference by the similarity computation function.

            As a rule of thumb, use a maximum of 1000 randomly selected tracks to set the music style (random selection
            is important to get a representative sample; if the sample is biased, results will be suboptimal).
            The tracks are analyzed and copied to internal storage as needed, so you may safely deallocate the given tracks after the call.

            :param tracks:
                a list of MuslyTrack instances.
        )pbdoc")

        .def("add_tracks", &MuslyJukebox::add_tracks, py::arg("tracks"), py::arg("track_ids"), R"pbdoc(
            Register tracks with the Musly jukebox.

            To use the music similarity function, each Musly track has to be registered with a jukebox.
            Internally, Musly computes an indexing and normalization vector for each registered track based on the set of tracks passed to :func:`set_style`.

            :param tracks:
                a list of MuslyTrack instances.
            :param track_ids:
                a list with an unique id for each MuslyTrack in `tracks`.
        )pbdoc")

        .def("remove_tracks", &MuslyJukebox::remove_tracks, py::arg("track_ids"), R"pbdoc(
            Remove tracks that were previously added to the jukebox via :func:`add_tracks`.

            :param track_ids:
                a list of track ids that belong to previously added tracks.
        )pbdoc")

        .def("compute_similarity", &MuslyJukebox::compute_similarity, py::arg("seed_track"), py::arg("seed_track_id"),
            py::arg("tracks"), py::arg("track_ids"), R"pbdoc(
            Compute the similarity between a seed track and a list of other tracks.

            To compute similarities between two music tracks, the following steps have to been taken:

            - analyze audio files, e.g. with :func:`track_from_audiofile` or :func:`track_from_audiodata`
            - set the music style of the jukebox by using a representative sample of analyzed tracks with :func:`set_style`
            - register the audio tracks with the jukebox using :func:`add_tracks`

            :param seed_track:
                the MuslyTrack used as reference.
            :param seed_track_id:
                the track id of the seed track.
            :param tracks:
                a list of MuslyTrack instances for which the similarities to the `seed_track` should be computed.
            :param track_ids:
                a list of track ids for the tracks given in `tracks`.
        )pbdoc");
}
