Installation
============

Install pymusly via `pip <https://pip-installer.org>`_::

    $ pip install pymusly

Currently only **Python 3.9+** is supported.

Pymusly is a wrapper for the musly C++ library and therefore requires the presence of libmusly on your system.
For convenience precompiled wheel packages for some platforms are available.precompiled.
The provided precompiled binaries come with platform dependant audio file decoder support:

- on macos a CoreAudio based decoder is available ('coreaudio')
- on MS Windows a Media Foundation based decoder is available ('mediafoundation')
- on Linux no precompiled decoder is enabled, beccause packaging ffmpeg libraries into the package currently doesn't work.
  But pymusly can use the `ffmpeg`/`avconv` command line utility to decode audio files, when available on your system.


Building from source
--------------------

In case no precompiled wheel package is provided, the library needs to be built on your machine.
To perform the build, you'll need the following prerequisites installed:

- A working C/C++ compiler suite (gcc/g++ on Linux, Xcode on macos, or the MSVC/MinGW toolchain on Windows)
- CMake 3.25+
- (Optional) development files for FFMpeg/LibAV

If you don't want to use one of the precompiled packages, for example to enable the libav decoder on Linux, you can enforce building pymusly by installing it using::

    $ pip install --no-binary pymusly
