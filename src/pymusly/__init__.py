from __future__ import annotations

from ._pymusly import (__doc__,
                        __version__,
                        get_musly_version,
                        set_musly_log_level,
                        musly_jukebox_listmethods as _musly_list_methods,
                        musly_jukebox_listdecoders  as _musly_list_decoders,
                        MuslyJukebox,
                        MuslyTrack,
                        MuslyError,
                       )

def get_musly_methods():
    methods: str = _musly_list_methods()
    if not methods:
        return []
    return methods.split(sep = ",")

def get_musly_decoders():
    decoders: str = _musly_list_decoders()
    if not decoders:
        return []
    return decoders.split(sep = ",")

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