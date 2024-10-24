# pymusly

Python bindings for libmusly to perform music similarity computation.

## Installation

The library can be installed via pip:

```shell
$ pip install pymusly
```

Since the library requires libmusly, wheel packages contain a precompiled libmusly.
When no wheel for the current platform is provided, the package needs to compile libmusly during installation.
In this case you might need to install additional tools like cmake and ffmpeg development files.
