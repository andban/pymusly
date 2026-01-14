from __future__ import annotations

from typing import List

from ._pymusly import (
    __version__,
    get_musly_version,
    set_musly_loglevel,
    musly_jukebox_listmethods as _musly_list_methods,
    musly_jukebox_listdecoders as _musly_list_decoders,
    MuslyJukebox,
    MuslyTrack,
    MuslyError,
)

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
]
