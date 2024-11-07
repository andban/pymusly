#ifndef MUSLY_TRACK_H_
#define MUSLY_TRACK_H_

#include "common.h"

#include <musly/musly_types.h>
#include <pybind11/pybind11.h>
#include <utility>

namespace pymusly {

class PYMUSLY_EXPORT MuslyTrack {
public:
    static void register_class(pybind11::module_& module);

public:
    MuslyTrack(musly_track* track);

    ~MuslyTrack();

    musly_track* data() const;

    operator bool() const
    {
        return static_cast<bool>(m_track);
    }

private:
    MuslyTrack(MuslyTrack&& other) = delete;

    MuslyTrack& operator=(MuslyTrack&& other) = delete;

    musly_track* m_track;
};

} // namespace pymusly

#endif // !MUSLY_TRACK_H_
