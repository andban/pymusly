#ifndef MUSLY_TRACK_H_
#define MUSLY_TRACK_H_

#include "common.h"

#include <musly/musly_types.h>
#include <pybind11/pybind11.h>
#include <utility>

class PYMUSLY_EXPORT MuslyTrack {
public:
    static void register_class(pybind11::module_& module);

public:
    MuslyTrack(musly_track* track);

    MuslyTrack(MuslyTrack&& other)
        : m_track(std::move(other.m_track))
    {
        other.m_track = nullptr;
    }

    MuslyTrack& operator=(MuslyTrack&& other)
    {
        m_track = std::move(other.m_track);
        other.m_track = nullptr;
        return *this;
    }

    ~MuslyTrack();

    musly_track* data() const;

    operator bool() const
    {
        return static_cast<bool>(m_track);
    }

private:
    musly_track* m_track;
};

#endif // !MUSLY_TRACK_H_
