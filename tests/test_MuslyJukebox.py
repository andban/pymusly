import io
import platform
import random
from unittest.mock import patch

import pytest

import pymusly as m
from pymusly import is_ffmpeg_present

from tests.helper import (
    is_linux_platform,
    is_macos_platform,
    is_windows_platform,
    to_fixture_path,
)


def get_default_decoder():
    if is_linux_platform():
        return "none"
    elif is_macos_platform():
        return "coreaudio"
    elif is_windows_platform():
        return "mediafoundation"
    else:
        raise NotImplementedError(f"platform {platform.system()} is not supported")


def test_init():
    expected_method = "timbre"
    expected_decoder = get_default_decoder()

    jukebox = m.MuslyJukebox()

    assert jukebox.method == expected_method
    assert jukebox.decoder == expected_decoder


def test_init_parameters():
    expected_method = "mandelellis"
    expected_decoder = "none"

    jukebox = m.MuslyJukebox(method=expected_method, decoder=expected_decoder)

    assert jukebox.method == expected_method
    assert jukebox.decoder == expected_decoder


def test_init_invalid():
    expected_method = "bruteforce"
    expected_decoder = "manual"

    with pytest.raises(m.MuslyError) as e:
        m.MuslyJukebox(method=expected_method, decoder=expected_decoder)

    assert e.match("failed to initialize musly jukebox")


def test_method_info():
    jukebox = m.MuslyJukebox(method="timbre")

    assert jukebox.method_info.startswith("A timbre only ")


def test_track_from_audiofile():
    jukebox = m.MuslyJukebox()

    track = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )

    assert isinstance(track, m.MuslyTrack)


def test_track_from_audiodata():
    jukebox = m.MuslyJukebox()
    noise = [random.random() for _ in range(22050 * 10)]
    track = jukebox.track_from_audiodata(noise)

    assert isinstance(track, m.MuslyTrack)


def test_track_from_audiodata_invalid():
    jukebox = m.MuslyJukebox()

    with pytest.raises(m.MuslyError):
        jukebox.track_from_audiodata([])


def test_serialize_to_stream():
    jukebox = m.MuslyJukebox()
    stream = io.BytesIO()

    jukebox.serialize_to_stream(stream)

    assert stream.tell() > 0


def test_create_from_stream():
    jukebox = m.MuslyJukebox(method="mandelellis")
    track = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), length=15, start=0
    )
    jukebox.set_style([track])
    jukebox.add_tracks([(42, track)])

    stream = io.BytesIO()
    jukebox.serialize_to_stream(stream)
    stream.seek(0)
    jukebox2 = m.MuslyJukebox.create_from_stream(stream, ignore_decoder=False)

    assert jukebox2.method == jukebox.method
    assert jukebox2.track_ids == jukebox.track_ids
    assert jukebox2.track_count == jukebox.track_count


def test_create_from_stream_invalid_version():
    with pytest.raises(m.MuslyError) as e:
        with open(to_fixture_path("wrong_version.jukebox"), "rb") as fh:
            m.MuslyJukebox.create_from_stream(fh, ignore_decoder=False)

    assert e.match(
        "failed loading jukebox: created with different musly version '[0-9]\\.[0-9]"
    )


def test_create_from_stream_invalid_int():
    with pytest.raises(m.MuslyError) as e:
        with open(to_fixture_path("wrong_int.jukebox"), "rb") as fh:
            m.MuslyJukebox.create_from_stream(fh, ignore_decoder=False)

    assert e.match("failed loading jukebox: different architecture")


def test_create_from_stream_invalid_byteorder():
    with pytest.raises(m.MuslyError) as e:
        with open(to_fixture_path("wrong_byteorder.jukebox"), "rb") as fh:
            m.MuslyJukebox.create_from_stream(fh, ignore_decoder=False)

    assert e.match("failed loading jukebox: invalid byte order")


def test_create_from_stream_invalid_decoder():
    with pytest.raises(m.MuslyError) as e:
        with open(to_fixture_path("wrong_decoder.jukebox"), "rb") as fh:
            m.MuslyJukebox.create_from_stream(fh, ignore_decoder=False)

    assert e.match("failed loading jukebox: decoder 'corefoundation' not available")


def test_create_from_stream_ignore_invalid_decoder():
    with open(to_fixture_path("wrong_decoder.jukebox"), "rb") as fh:
        jukebox = m.MuslyJukebox.create_from_stream(fh, ignore_decoder=True)

    assert jukebox.decoder != "corefoundation"
    assert jukebox.method == "timbre"
    assert jukebox.track_count == 3
    assert jukebox.track_ids == [1, 2, 3]


def test_set_style():
    jukebox = m.MuslyJukebox()
    track = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )

    jukebox.set_style([track])


def test_add_tracks_without_setting_style():
    jukebox = m.MuslyJukebox()
    track = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )

    with pytest.raises(m.MuslyError) as e:
        jukebox.add_tracks([(1, track)])

    assert e.match(
        "failure while adding tracks to jukebox. maybe set_style has not been called?"
    )


def test_add_tracks_with_custom_ids():
    jukebox = m.MuslyJukebox()
    track_1 = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )
    track_2 = jukebox.track_from_audiofile(
        to_fixture_path("sample-12s.mp3"), start=0, length=12
    )

    jukebox.set_style([track_1])
    returned_ids = jukebox.add_tracks([(23, track_1), (42, track_2)])

    assert returned_ids == [23, 42]
    assert jukebox.track_count == 2
    assert jukebox.track_ids == returned_ids
    assert jukebox.highest_track_id == 42


def test_add_tracks_with_generated_ids():
    jukebox = m.MuslyJukebox()
    track_1 = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )
    track_2 = jukebox.track_from_audiofile(
        to_fixture_path("sample-12s.mp3"), start=0, length=12
    )

    jukebox.set_style([track_1])
    returned_ids = jukebox.add_tracks([track_1, track_2])

    assert returned_ids == [0, 1]
    assert jukebox.track_count == 2
    assert jukebox.track_ids == returned_ids
    assert jukebox.highest_track_id == 1


def test_remove_tracks():
    jukebox = m.MuslyJukebox()
    track_1 = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )
    track_2 = jukebox.track_from_audiofile(
        to_fixture_path("sample-12s.mp3"), start=0, length=12
    )
    jukebox.set_style([track_1])
    jukebox.add_tracks([(1, track_1), (2, track_2)])

    jukebox.remove_tracks([1])

    assert jukebox.track_count == 1
    assert jukebox.track_ids == [2]


def test_track_serialization():
    jukebox = m.MuslyJukebox()
    track = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )

    track_bytes = jukebox.serialize_track(track)

    assert len(track_bytes) == jukebox.track_size


def test_track_deserialization():
    jukebox = m.MuslyJukebox()
    track_1 = jukebox.track_from_audiofile(
        to_fixture_path("sample-15s.mp3"), start=0, length=15
    )
    track_2 = jukebox.track_from_audiofile(
        to_fixture_path("sample-12s.mp3"), start=0, length=12
    )
    track_3 = jukebox.track_from_audiofile(
        to_fixture_path("sample-9s.mp3"), start=0, length=9
    )
    jukebox.set_style([track_1, track_2])
    jukebox.add_tracks([(1, track_1), (2, track_2), (3, track_3)])

    similarity_before = jukebox.compute_similarity((1, track_1), [(3, track_3)])
    track_3b = jukebox.deserialize_track(jukebox.serialize_track(track_3))
    similarity_after = jukebox.compute_similarity((1, track_1), [(3, track_3b)])

    assert similarity_before == similarity_after


@pytest.mark.skipif(not is_ffmpeg_present(), reason="no ffmpeg binaries installed")
@pytest.mark.parametrize(
    "start,length",
    [
        (0, 9),
        (0, 23),
        (-5, 4),
        (6, 9),
    ],
)
def test_ffmpeg_decode_fallback(start, length):
    jukebox = m.MuslyJukebox(decoder="none")

    track = jukebox.track_from_audiofile(
        to_fixture_path("sample-9s.mp3"), start=start, length=length
    )

    assert isinstance(track, m.MuslyTrack)


@patch("pymusly.ffmpeg_decode._run_avprobe")
def test_no_ffmpeg_decode_fallback(mock_avprobe):
    mock_avprobe.side_effect = OSError("boom")
    jukebox = m.MuslyJukebox(decoder="none")

    with pytest.raises(m.MuslyError) as e:
        jukebox.track_from_audiofile(
            to_fixture_path("sample-9s.mp3"), start=0, length=9
        )

    assert e.match("jukebox has no decoder and no ffmpeg/avconv tools found")
