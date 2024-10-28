
build:
	python -m build

lint:
	python -m pip install '.[lint]'
	python -m ruff check .

format:
	python -m pip install '.[lint]'
	python -m ruff format .

test:
	python -m pip install '.[test]'
	python -m pytest

docs:
	cd docs && make html

install-dev:
	python -m pip install -e . -Ccmake.define.CMAKE_EXPORT_COMPILE_COMMANDS=1 -Cbuild-dir=build -Ccmake.build-type=Debug -Cbuild.verbose=true

install:
	python -m pip install .

.PHONY: build test install install-dev docs
