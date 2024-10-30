
build:
	python -m build

lint:
	python -m pip install -q -r requirements-dev.txt
	python -m ruff check .

format:
	python -m pip install -q -r requirements-dev.txt
	python -m ruff format .

test:
	python -m pip install -q -r requirements-dev.txt
	python -m pytest

docs:
	python -m pip install -q -r requirements-dev.txt
	cd docs && make html

install-dev:
	python -m pip install -ve . -Ccmake.define.CMAKE_EXPORT_COMPILE_COMMANDS=1 -Cbuild-dir=build -Ccmake.build-type=Debug -Cbuild.verbose=true

install:
	python -m pip install -v .

.PHONY: build test install install-dev docs
