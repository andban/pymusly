
build:
	python -m build

test:
	python -m pip install '.[test]'
	python -m pytest

install-dev:
	python -m pip install -e . -Ccmake.define.CMAKE_EXPORT_COMPILE_COMMANDS=1 -Cbuild-dir=build -Ccmake.build-type=Debug -Cbuild.verbose=true

install:
	python -m pip install .

.PHONY: build test install install-dev
