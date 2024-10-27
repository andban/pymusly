import pymusly as m


def test_version():
    assert m.__version__ == "0.1.0"


def test_get_musly_version():
    assert m.get_musly_version() == "0.2"


def test_get_musly_decoders():
    assert "none" in m.get_musly_decoders()


def test_get_musly_methods():
    assert m.get_musly_methods() == ["mandelellis", "timbre"]
