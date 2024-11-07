#ifndef MUSLY_JUKEBOX_H_
#define MUSLY_JUKEBOX_H_

#include "BytesIO.h"
#include "MuslyTrack.h"
#include "common.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <memory>
#include <musly/musly_types.h>
#include <vector>

namespace pymusly {

class PYMUSLY_EXPORT MuslyJukebox {
public:
    typedef std::pair<musly_trackid, MuslyTrack*> track_tuple_t;

public:
    static MuslyJukebox* create_from_stream(pymusly::BytesIO& in_stream, bool ignore_decoder = true);

    static void register_class(pybind11::module_& module);

public:
    MuslyJukebox(const char* method = nullptr, const char* decoder = nullptr);
    ~MuslyJukebox();

    const char* method() const;

    const char* method_info() const;

    const char* decoder() const;

    int track_size() const;

    MuslyTrack* track_from_audiofile(const char* filename, int length, int start);

    MuslyTrack* track_from_audiodata(const std::vector<float>& pcm_data);

    MuslyTrack* deserialize_track(pybind11::bytes bytes);

    pybind11::bytes serialize_track(MuslyTrack* track);

    void set_style(const std::vector<MuslyTrack*>& tracks);

    int track_count() const;

    std::vector<musly_trackid> track_ids() const;

    musly_trackid highest_track_id() const;

    std::vector<musly_trackid> add_tracks(const std::vector<MuslyTrack*>& tracks);

    std::vector<musly_trackid> add_tracks(const std::vector<track_tuple_t>& tracks);

    void remove_tracks(const std::vector<musly_trackid>& track_ids);

    std::vector<float> compute_similarity(track_tuple_t seed, const std::vector<track_tuple_t>& track_tuples);

    void serialize(pymusly::BytesIO& out_stream);

private:
    musly_jukebox* m_jukebox;
};

} // namespace pymusly

#endif // !MUSLY_JUKEBOX_H_
