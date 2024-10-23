#ifndef MUSLY_JUKEBOX_H_
#define MUSLY_JUKEBOX_H_

#include "common.h"
#include "MuslyTrack.h"
#include "pystream.h"

#include <pybind11/stl_bind.h>
#include <pybind11/pybind11.h>

#include <musly/musly_types.h>
#include <vector>
#include <memory>


class PYMUSLY_EXPORT MuslyJukebox
{
public:
    typedef std::vector<MuslyTrack*>    musly_track_vec;
    typedef std::vector<musly_trackid> musly_trackid_vec;

public:
    static MuslyJukebox*
    create_from_stream(std::istream& i, bool ignore_decoder = true);

    static void
    register_class(pybind11::module_& m);

public:
    MuslyJukebox(const char* method = nullptr, const char* decoder = nullptr);
    ~MuslyJukebox();

    const char*
    method() const;

    const char*
    method_info() const;

    const char*
    decoder() const;

    int
    track_size() const;

    MuslyTrack*
    track_from_audiofile(const char* filename, int start, int length);

    MuslyTrack*
    track_from_audiodata(const std::vector<float>& pcm_data);

    MuslyTrack*
    deserialize_track(pybind11::bytes bytes);

    pybind11::bytes
    serialize_track(MuslyTrack* track);

    void
    set_style(const musly_track_vec& tracks);

    int
    track_count() const;

    musly_trackid_vec
    track_ids() const;

    musly_trackid
    highest_track_id() const;

    musly_trackid_vec
    add_tracks(const musly_track_vec& track_list, const musly_trackid_vec& track_ids);

    void
    remove_tracks(const musly_trackid_vec& track_ids);

    musly_trackid_vec
    guess_neighbors(musly_trackid seed_track_id, int n);

    musly_trackid_vec
    guess_neighbors(musly_trackid seed_track_id,
                    int n,
                    const musly_trackid_vec& track_ids);

    std::vector<float>
    compute_similarity(MuslyTrack* seed_track,
                       musly_trackid seed_track_id,
                       const musly_track_vec& tracks,
                       const musly_trackid_vec& track_ids);

    void
    serialize(std::ostream& o);

private:
    musly_jukebox m_jukebox;
};

#endif // !MUSLY_JUKEBOX_H_
