from __future__ import annotations

from ._pymusly import (
    __doc__,
    __version__,
    get_musly_version,
    set_musly_log_level,
    musly_jukebox_listmethods as _musly_list_methods,
    musly_jukebox_listdecoders as _musly_list_decoders,
    MuslyJukebox as OriginalMuslyJukebox,
    MuslyTrack,
    MuslyError,
)

from .ffmpeg_decode import duration_with_ffprobe, decode_with_ffmpeg


def get_musly_methods():
    methods: str = _musly_list_methods()

    return methods.split(sep=",") if len(methods) else []


def get_musly_decoders():
    decoders: str = _musly_list_decoders()

    return decoders.split(sep=",") if len(decoders) else []


class MuslyJukebox(OriginalMuslyJukebox):
    def __init__(self, method: str = None, decoder: str = None):  #
        OriginalMuslyJukebox.__init__(self, method=method, decoder=decoder)

    def track_from_audiofile(self, filename: str, length: float, start: float):
        if self.decoder != "none":
            return OriginalMuslyJukebox.track_from_audiofile(
                self, filename, length, start
            )

        duration = duration_with_ffprobe(filename)
        if length <= 0 or length >= duration:
            start = 0.0
            length = duration
        elif start < 0:
            start = min(-start, (duration - length) / 2.0)
        elif length + start > duration:
            start = max(0.0, duration - length)
            length = min(duration, duration - start)

        samples = decode_with_ffmpeg(filename, start, length)
        return self.track_from_audiodata(samples)


__all__ = [
    "__doc__",
    "__version__",
    "get_musly_version",
    "set_musly_log_level",
    "get_musly_methods",
    "get_musly_decoders",
    "MuslyJukebox",
    "MuslyTrack",
    "MuslyError",
]
