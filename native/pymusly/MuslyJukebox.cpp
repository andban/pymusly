#include "MuslyJukebox.h"
#include "musly_error.h"

#include <exception>
#include <musly/musly.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>

#include <iostream>

namespace py = pybind11;
using namespace pymusly;

namespace {

const int _ENDIAN_MAGIC_NUMBER = 0x01020304;

} // namespace

namespace pymusly {

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

std::vector<musly_trackid> MuslyJukebox::track_ids() const
{
    std::vector<musly_trackid> track_ids(track_count());
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

    if (musly_track_analyze_audiofile(m_jukebox, filename, length, start, track) != 0) {
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

    if (musly_track_analyze_pcm(m_jukebox, const_cast<float*>(pcm_data.data()), pcm_data.size(), track) != 0) {
        throw musly_error("could not load track from pcm");
    }

    return new MuslyTrack(track);
}

std::vector<musly_trackid> MuslyJukebox::add_tracks(const std::vector<MuslyTrack*>& tracks)
{
    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(tracks.begin(), tracks.end(), musly_tracks.begin(), [](MuslyTrack* track) { return track->data(); });

    std::vector<musly_trackid> track_ids(tracks.size());
    int ret = musly_jukebox_addtracks(m_jukebox, const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()), tracks.size(), 1);
    if (ret < 0) {
        throw musly_error("failure while adding tracks to jukebox. "
                          "maybe set_style has not been called?");
    }

    return track_ids;
}

std::vector<musly_trackid> MuslyJukebox::add_tracks(const std::vector<std::pair<musly_trackid, MuslyTrack*>>& track_tuples)
{
    std::vector<musly_trackid> track_ids(track_tuples.size());
    std::transform(
        track_tuples.begin(),
        track_tuples.end(),
        track_ids.begin(),
        [](auto pair) { return pair.first; });

    std::vector<musly_track*> musly_tracks(track_tuples.size());
    std::transform(
        track_tuples.begin(),
        track_tuples.end(),
        musly_tracks.begin(),
        [](auto pair) { return pair.second->data(); });

    int ret = musly_jukebox_addtracks(m_jukebox, const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()), track_tuples.size(), 0);
    if (ret < 0) {
        throw musly_error("failure while adding tracks to jukebox. "
                          "maybe set_style has not been called?");
    }

    return track_ids;
}

void MuslyJukebox::remove_tracks(const std::vector<musly_trackid>& track_ids)
{
    if (musly_jukebox_removetracks(m_jukebox, const_cast<musly_trackid*>(track_ids.data()), track_ids.size()) < 0) {
        throw musly_error("failure while removing tracks from jukebox");
    }
}

void MuslyJukebox::set_style(const std::vector<MuslyTrack*>& tracks)
{
    std::vector<musly_track*> musly_tracks(tracks.size());
    std::transform(tracks.begin(), tracks.end(), musly_tracks.begin(), [](MuslyTrack* track) { return track->data(); });

    int ret = musly_jukebox_setmusicstyle(m_jukebox, const_cast<musly_track**>(musly_tracks.data()), tracks.size());
    if (ret < 0) {
        throw musly_error("failure while setting style of jukebox");
    }
}

std::vector<float> MuslyJukebox::compute_similarity(track_tuple_t seed, const std::vector<track_tuple_t>& track_tuples)
{
    std::vector<musly_trackid> track_ids(track_tuples.size());
    std::transform(track_tuples.begin(), track_tuples.end(), track_ids.begin(), [](auto pair) { return pair.first; });

    std::vector<musly_track*> musly_tracks(track_tuples.size());
    std::transform(track_tuples.begin(), track_tuples.end(), musly_tracks.begin(), [](auto pair) { return pair.second->data(); });

    std::vector<float> similarities(track_tuples.size(), 0.0F);
    int ret = musly_jukebox_similarity(
        m_jukebox, seed.second->data(), seed.first, const_cast<musly_track**>(musly_tracks.data()),
        const_cast<musly_trackid*>(track_ids.data()), track_tuples.size(), similarities.data());
    if (ret < 0) {
        throw musly_error("failure while computing track similarity");
    }

    return similarities;
}

py::bytes MuslyJukebox::serialize_track(MuslyTrack* track)
{
    if (track == nullptr) {
        throw musly_error("track must not be none");
    }

    char* bytes = new char[track_size()];
    int err = musly_track_tobin(m_jukebox, track->data(), reinterpret_cast<unsigned char*>(bytes));
    if (err < 0) {
        delete[] bytes;
        throw musly_error("failed to convert track to bytearray");
    }

    return py::bytes(bytes, track_size());
}

MuslyTrack* MuslyJukebox::deserialize_track(py::bytes bytes)
{
    musly_track* track = musly_track_alloc(m_jukebox);
    if (track == nullptr) {
        throw musly_error("could not allocate track");
    }

    int ret = musly_track_frombin(m_jukebox, reinterpret_cast<unsigned char*>(PyBytes_AsString(bytes.ptr())), track);
    if (ret < 0) {
        throw musly_error("failed to convert bytearray to track");
    }

    return new MuslyTrack(track);
}

void MuslyJukebox::serialize(BytesIO& out_stream)
{
    const int tracks_per_chunk = 100;
    const uint8_t int_size = sizeof(int);

    // write current musly_version, sizeof(int) and known int value for
    // compatibility checks when deserializing the file at a later point in time
    out_stream.write_line(musly_version(), '\0');
    out_stream.write(&int_size, 1);
    out_stream.write(&_ENDIAN_MAGIC_NUMBER, int_size);
    out_stream.write_line(method(), '\0');
    out_stream.write_line(decoder(), '\0');

    const int header_size = musly_jukebox_binsize(m_jukebox, 1, 0);
    if (header_size < 0) {
        throw musly_error("could not get jukebox header size");
    }
    out_stream.write(&header_size, int_size);

    const int buffer_length = std::max(header_size, tracks_per_chunk * track_size());
    std::unique_ptr<unsigned char> buffer(new unsigned char[buffer_length]);

    if (musly_jukebox_tobin(m_jukebox, buffer.get(), 1, 0, 0) < 0) {
        throw musly_error("could not serialize jukebox header");
    }
    out_stream.write(buffer.get(), header_size);

    // write jukebox header together with its size in bytes
    const int total_tracks_to_write = track_count();
    int tracks_written = 0;
    int bytes_written;
    while (tracks_written < total_tracks_to_write) {
        const int tracks_to_write = std::min(tracks_per_chunk, total_tracks_to_write - tracks_written);
        const int bytes_to_write = musly_jukebox_tobin(m_jukebox, buffer.get(), 0, tracks_to_write, tracks_written);
        if (bytes_to_write < 0) {
            throw musly_error("failed to write data into buffer");
        }
        out_stream.write(buffer.get(), bytes_to_write);
        tracks_written += tracks_to_write;
    }

    out_stream.flush();
}

MuslyJukebox* MuslyJukebox::create_from_stream(BytesIO& in_stream, bool ignore_decoder)
{
    std::string version = in_stream.read_line('\0');
    if (version.empty() || version != musly_version()) {
        throw musly_error("failed loading jukebox: created with different musly version '" + version + "'");
    }

    uint8_t int_size = 0;
    in_stream.read(&int_size, sizeof(uint8_t));
    if (int_size != sizeof(int)) {
        throw musly_error("failed loading jukebox: different architecture");
    }

    unsigned int byte_order = 0;
    in_stream.read(&byte_order, int_size);
    if (byte_order != _ENDIAN_MAGIC_NUMBER) {
        throw musly_error("failed loading jukebox: invalid byte order");
    }

    const std::string decoders = musly_jukebox_listdecoders();
    const std::string method = in_stream.read_line('\0');
    std::string decoder = in_stream.read_line('\0');
    if (decoder.empty() || decoders.find(decoder) == std::string::npos) {
        if (!ignore_decoder) {
            throw musly_error("failed loading jukebox: decoder '" + decoder + "' not available");
        }
        decoder = "";
    }

    std::unique_ptr<MuslyJukebox> jukebox(new MuslyJukebox(method.c_str(), decoder.empty() ? nullptr : decoder.c_str()));

    int header_size;
    if (in_stream.read(&header_size, int_size) < int_size) {
        throw musly_error("failed loading jukebox: could not read header size");
    }

    std::unique_ptr<unsigned char> header(new unsigned char[header_size]);
    in_stream.read(header.get(), header_size);
    const int track_count = musly_jukebox_frombin(jukebox->m_jukebox, header.get(), 1, 0);

    if (track_count < 0) {
        throw musly_error("failed loading jukebox: invalid header");
    }

    const int track_size = musly_jukebox_binsize(jukebox->m_jukebox, 0, 1);
    const int tracks_per_chunk = 100;
    const int buffer_len = track_size * tracks_per_chunk;
    std::unique_ptr<unsigned char> buffer(new unsigned char[buffer_len]);

    int tracks_read = 0;
    while (tracks_read < track_count) {
        const int tracks_to_read = std::min(tracks_per_chunk, track_count - tracks_read);
        const int bytes_to_read = tracks_to_read * track_size;

        if (in_stream.read(buffer.get(), bytes_to_read) < bytes_to_read) {
            throw musly_error("failed loading jukebox: received less tracks than expected");
        }
        if (musly_jukebox_frombin(jukebox->m_jukebox, buffer.get(), 0, tracks_to_read) < 0) {
            throw musly_error("failed loading jukebox: failed to load track information");
        }

        tracks_read += tracks_to_read;
    }

    return jukebox.release();
}

void MuslyJukebox::register_class(py::module_& module)
{
    py::class_<MuslyJukebox>(module, "MuslyJukebox")
        .def(py::init<const char*, const char*>(), py::arg("method") = nullptr, py::arg("decoder") = nullptr, R"pbdoc(
            __init__(method: str = None, decoder: str = None) -> None


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
            :raises MuslyError:
                if no jukebox with the given parameters can be created.
        )pbdoc")

        .def_static("create_from_stream", &MuslyJukebox::create_from_stream, py::arg("input_stream"),
            py::arg("ignore_decoder"), py::return_value_policy::take_ownership, R"pbdoc(
            create_from_stream(input_stream: io.BytesIO, ignore_decoder: bool = False) -> MuslyJukebox


            Load previously serialized MuslyJukebox from an io.BytesIO stream.

            :param stream:
                an readable binary stream, like the result of `open('electronic-music.jukebox', 'rb')`.
            :param ignore_decoder:
                when `True`, the resulting jukebox will use the default decoder, in case the original decoder is not available.

            :return: the deserialized jukebox
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

        .def_property_readonly("track_ids", &MuslyJukebox::track_ids, R"pbdoc(
            A list of all track ids assigned to tracks added to this jukebox instance.
        )pbdoc")

        .def("track_from_audiofile", &MuslyJukebox::track_from_audiofile, py::arg("input_stream"), py::arg("length"),
            py::arg("start"), py::return_value_policy::take_ownership, R"pbdoc(
            track_from_audiofile(input_stream: io.BytesIO, length: int, start: int) -> MuslyTrack


            Create a MuslyTrack by analysing an excerpt of the given audio file.

            The audio file is decoded by using the decoder selected during MuslyJukebox creation. The decoded audio signal is then down- and resampled into a 20,050Hz mono signal which is used as input for track_from_audiodata().

            :param input_stream:
                an input stream to the audio file to decode, like the result of `open('test.mp3', 'rb')`.
            :param ignore_decoder:
                when True, the resulting jukebox will use the default decoder, when the original decoder is not available.
            :raises MuslyError:
                if no track can be created from the given input stream.
        )pbdoc")

        .def("track_from_audiodata", &MuslyJukebox::track_from_audiodata, py::arg("pcm_data"),
            py::return_value_policy::take_ownership, R"pbdoc(
            track_from_audiodata(pcm_data: list[float]) -> MuslyTrack


            Create a MuslyTrack by analyzing the provided PCM samples.

            The input samples are expected to represent a mono signal with 22050Hz sample rate using float values.

            :param pcm_data:
                the sample data to analyze.
            :raises MuslyError:
                if no track can be created from the given sample data.
        )pbdoc")

        .def("serialize_track", &MuslyJukebox::serialize_track, py::arg("track"),
            py::return_value_policy::take_ownership, R"pbdoc(
            serialize_track(track: MuslyTrack) -> bytes


            Serialize a MuslyTrack into a `bytes` object.

            :param track:
                a MuslyTrack object.
            :raises MuslyError:
                if the given track cannot be serialized.
        )pbdoc")

        .def("deserialize_track", &MuslyJukebox::deserialize_track, py::arg("bytes_track"),
            py::return_value_policy::take_ownership, R"pbdoc(
            deserialize_track(bytes_track: bytes) -> MuslyTrack


            Deserialize a MuslyTrack from a `bytes` object.

            :param bytes_track:
                a previously with :func:`serialize_track` serialized MuslyTrack.
            :return:
                a MuslyTrack instance.
            :raises MuslyError:
                if the given data cannot be deserialized into a MuslyTrack.
        )pbdoc")

        .def("serialize_to_stream", &MuslyJukebox::serialize, py::arg("output_stream"), R"pbdoc(
            serialize_to_stream(output_stream: io.BytesIO) -> None


            Serialize jukebox instance into a `io.BytesIO` stream`.

            :param output_stream:
                an output stream, like one created by `open('electronic-music.jukebox', 'wb')`.
            :raises MuslyError:
                if the jukebox cannot be written into the given output stream.
        )pbdoc")

        .def("set_style", &MuslyJukebox::set_style, py::arg("tracks"), R"pbdoc(
            set_style(tracks: list[MuslyTrack]) -> None


            Initialize jukebox with a set of tracks that are used as reference by the similarity computation function.

            As a rule of thumb, use a maximum of 1000 randomly selected tracks to set the music style (random selection
            is important to get a representative sample; if the sample is biased, results will be suboptimal).
            The tracks are analyzed and copied to internal storage as needed, so you may safely deallocate the given tracks after the call.

            :param tracks:
                a list of MuslyTrack instances.
            :raises MuslyError:
                if the the given tracks cannot be used to set the style of this jukebox.
        )pbdoc")

        .def("add_tracks", py::overload_cast<const std::vector<MuslyTrack*>&>(&MuslyJukebox::add_tracks), py::arg("tracks"), R"pbdoc(
            add_tracks(tracks: list[tuple[int,MuslyTrack]]) -> list[int]
            add_tracks(tracks: list[MuslyTrack]) -> list[int]

            Register tracks with the Musly jukebox.

            When the tracks parameter contains a list of id/track tuples, the provided IDs will be used for registration.
            In case a list containing only MuslyTrack instances is provided, IDs will be generated for each track.


            To use the music similarity function, each Musly track has to be registered with a jukebox.
            Internally, Musly computes an indexing and normalization vector for each registered track based on the set of tracks passed to :func:`set_style`.

            :param track_tuples:
                a list of tuples containing a track id and a corresponding MuslyTrack.
            :return:
                a containing the ids of the tracks that were added.
            :raises MuslyError:
                if the given tracks cannot be added to the jukebox, i.e. a  :func:`set_style` has not been called yet.
        )pbdoc")

        .def("add_tracks", py::overload_cast<const std::vector<MuslyJukebox::track_tuple_t>&>(&MuslyJukebox::add_tracks), py::arg("tracks"))

        .def("remove_tracks", &MuslyJukebox::remove_tracks, py::arg("track_ids"), R"pbdoc(
            remove_tracks(track_ids: list[int]) -> None


            Remove tracks that were previously added to the jukebox via :func:`add_tracks`.

            :param track_ids:
                a list of track ids that belong to previously added tracks.
        )pbdoc")

        .def("compute_similarity", &MuslyJukebox::compute_similarity, py::arg("seed"), py::arg("tracks"), R"pbdoc(
            compute_similarity(seed: tuple[int,MuslyTrack], tracks: list[tuple[int,MuslyTrack]]) -> list[float]


            Compute the similarity between a seed track and a list of other tracks.

            To compute similarities between two music tracks, the following steps have to been taken:

            - analyze audio files, e.g. with :func:`track_from_audiofile` or :func:`track_from_audiodata`
            - set the music style of the jukebox by using a representative sample of analyzed tracks with :func:`set_style`
            - register the audio tracks with the jukebox using :func:`add_tracks`

            :param seed:
                a tuple containing a track id and a MuslyTrack instance used as reference.
            :param tracks:
                a list of MuslyTrack instances for which the similarities to the `seed_track` should be computed.
            :param track_ids:
                a list of track ids for the tracks given in `tracks`.
            :return:
                a list with similarities to the seed track for each given track.
            :raises MuslyError:
                if the style computation failed.
        )pbdoc");
}

} // namespace pymusly