import datetime
import io
import queue
import re
import struct
import subprocess
import sys
import threading
import time
from typing import List

from pymusly._pymusly import MuslyError

_sample_rate = "22050"
_channels = "1"
_ffmpeg_commands = ("ffmpeg", "avconv")

_ffprobe_commands = ("ffprobe", "avprobe")
_duration_pattern = re.compile('^format\\.duration="(?P<duration>[0-9]+\\.[0-9]+)"$')


class _ReaderThread(threading.Thread):
    def __init__(self, fh, blocksize=1024, discard=False):
        super().__init__()
        self.fh = fh
        self.blocksize = blocksize
        self.daemon = True
        self.discard = discard
        self.queue = None if discard else queue.Queue()

    def run(self):
        while True:
            data = self.fh.read(self.blocksize)
            if not self.discard:
                self.queue.put(data)
            if not data:
                break


def _popen_avconv(cli_args: List[str], *args, **kwargs):
    for cmd in _ffmpeg_commands:
        cmd_with_args = [cmd] + cli_args
        try:
            return subprocess.Popen(cmd_with_args, *args, **kwargs)
        except OSError:
            continue
    raise MuslyError(f"no ffmpeg command found, tried: {_ffmpeg_commands}")


def _run_avprobe(cli_args, *args, **kwargs):
    for cmd in _ffprobe_commands:
        cmd_with_args = [cmd] + cli_args
        try:
            return subprocess.run(cmd_with_args, *args, **kwargs)
        except OSError:
            continue
    raise MuslyError(f"no ffprobe command found, tried: {_ffprobe_commands}")


class _AudioReader:
    def __init__(self, filename: str, start: float, length: float):
        start_str = f"0{datetime.timedelta(seconds=start)}"[0:8]
        end_str = f"0{datetime.timedelta(seconds=start + length)}"[0:8]

        args = [
            "-y",
            "-i", filename,
            "-ss", start_str,
            "-to", end_str,
            "-f", "f32le" if sys.byteorder == "little" else "f32be",
            "-ac", _channels,
            "-ar", _sample_rate,
            "-",
        ]  # fmt: skip

        try:
            self.proc = _popen_avconv(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.DEVNULL,
            )
        except Exception as e:
            raise MuslyError(f"failure while calling ffmpeg: {e}")

        self.reader = _ReaderThread(self.proc.stdout, io.DEFAULT_BUFFER_SIZE)
        self.reader.start()

        self.err_reader = _ReaderThread(self.proc.stderr, io.DEFAULT_BUFFER_SIZE)
        self.err_reader.start()

    def read_bytes(self, timeout=10.0):
        start_time = time.time()
        while True:
            data = None
            try:
                data = self.reader.queue.get(timeout=timeout)
                if data:
                    yield data
                else:
                    break

            except queue.Empty:
                cur_time = time.time()
                if not data:
                    if cur_time - start_time >= timeout:
                        err = b"".join(self.err_reader.queue.queue)
                        raise RuntimeError(f"ffmpeg hangs: {err}")
                    else:
                        start_time = cur_time
                        continue

    def close(self):
        self.proc.poll()
        if self.proc.returncode is None:
            self.proc.kill()
            self.proc.wait()

        if hasattr(self, "err_reader"):
            self.err_reader.join()
        if hasattr(self, "reader"):
            self.reader.join()

        self.proc.stdout.close()
        self.proc.stderr.close()

    def __del__(self):
        self.close()

    def __iter__(self):
        return self.read_bytes()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False


def _decode_with_ffmpeg(filename: str, start: float, length: float):
    raw_samples = bytearray()
    with _AudioReader(filename=filename, start=start, length=length) as r:
        for buf in r:
            raw_samples.extend(buf)

    n_samples = int(len(raw_samples) / 4)
    return struct.unpack(f"={n_samples}f", raw_samples)


def _duration_with_ffprobe(filename: str):
    args = [
        "-v", "quiet",
        "-print_format", "flat",
        "-show_entries", "format=duration",
        filename,
    ]  # fmt: skip

    try:
        result = _run_avprobe(args, capture_output=True, text=True)
    except Exception as e:
        raise MuslyError(f"failure while calling ffprobe: {e}")

    if result.returncode != 0:
        raise MuslyError(f"failed ffprobe call: {result.stderr}")

    match = _duration_pattern.match(result.stdout)
    if match is None:
        raise MuslyError(f"failed parse ffprobe output: {result.stdout}")

    return float(match.group("duration"))


def is_ffmpeg_present():
    try:
        result = _run_avprobe(["-version"], capture_output=True, text=True)
    except Exception:
        return False

    if result.returncode != 0:
        return False

    return True
