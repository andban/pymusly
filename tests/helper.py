import os.path
import platform

_test_dir = os.path.dirname(__file__)

FIXTURE_DIR = os.path.abspath(os.path.join(_test_dir, "fixtures"))


def to_fixture_path(fixture: str):
    return os.path.join(FIXTURE_DIR, fixture)


def is_linux_platform():
    return platform.system() == "Linux"


def is_macos_platform():
    return platform.system() == "Darwin"


def is_windows_platform():
    return platform.system() == "Windows"
