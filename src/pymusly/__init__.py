from __future__ import annotations

from typing import List

from ._pymusly import (
    __version__,
    get_musly_version,
    set_musly_loglevel,
    musly_jukebox_listmethods as _musly_list_methods,
    musly_jukebox_listdecoders as _musly_list_decoders,
    MuslyJukebox as _OriginalMuslyJukebox,
    MuslyTrack,
    MuslyError,
)

from .ffmpeg_decode import _duration_with_ffprobe, _decode_with_ffmpeg, is_ffmpeg_present

__doc__ = """
    Python binding for the libmusly music similarity computation library.
"""


def get_musly_methods() -> List[str]:
    """Return a list of all available similarity methods."""
    methods: str = _musly_list_methods()

    return methods.split(sep=",") if len(methods) else []


def get_musly_decoders() -> List[str]:
    """Return a list of all available audio file decoders."""
    decoders: str = _musly_list_decoders()

    return decoders.split(sep=",") if len(decoders) else []


class MuslyJukebox(_OriginalMuslyJukebox):
    def track_from_audiofile(
        self: _OriginalMuslyJukebox, filename: str, length: float, start: float = 0.0
    ) -> MuslyTrack:
        """Create a MuslyTrack by analysing an excerpt of the given audio file.

        The audio file is decoded by using the decoder selected during MuslyJukebox creation.
        The decoded audio signal is then down- and resampled into a 20,050Hz mono signal which is used as
        input for :func:`track_from_audiodata`.

        In case the `none` decoder was selected, the audio file will be decoded by using the 'ffmpeg/avconv' command line tool, if found in `$PATH`.

        :param filename:
            the path to the audio file that should be analyzed.
        :param length:
            the length of the excerpt in seconds.
            If `length <= 0`, the whole audio file will be analyzed.
        :param start:
            the start of the excerpt in seconds.
            If `start < 0`, musly tries to extract an excerpt of `length` seconds centered around half the duration
            of the audio file, but which starts at least at `-start` seconds.
        """
        if self.decoder != "none":
            return _OriginalMuslyJukebox.track_from_audiofile(
                self, filename, length, start
            )

        if not is_ffmpeg_present():
            raise MuslyError("jukebox has no decoder and no ffmpeg/avconv tools found")

        duration = _duration_with_ffprobe(filename)
        if length <= 0 or length >= duration:
            start = 0.0
            length = duration
        elif start < 0:
            start = min(-start, (duration - length) / 2.0)
        elif length + start > duration:
            start = max(0.0, duration - length)
            length = min(duration, duration - start)

        samples = _decode_with_ffmpeg(filename, start, length)
        return self.track_from_audiodata(samples)


__all__ = [
    "__doc__",
    "__version__",
    "get_musly_version",
    "set_musly_loglevel",
    "get_musly_methods",
    "get_musly_decoders",
    "MuslyJukebox",
    "MuslyTrack",
    "MuslyError",
    "is_ffmpeg_present"
]
