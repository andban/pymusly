
build:
	python -m build

install-dev:
	python -m pip install -ve . \
		-Ccmake.define.CMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-Cbuild-dir=build \
		-Ccmake.build-type=Debug \
		-Cbuild.verbose=true


install:
	python -m pip install -v .

lint:
	@command -v ruff >/dev/null || python -m pip install '.[dev]'
	python -m ruff check .

format:
	@command -v ruff >/dev/null || python -m pip install '.[dev]'
	python -m ruff format .
	cmake-format -i native/**/CMakeLists.txt

tests:
	python -m pip install -v '.[dev]' \
		-Cbuild-dir=build \
		-Ccmake.build-type=Profile \
		-Cbuild.verbose=true
	python -m pytest --cov=pymusly --cov-report=term
	gcovr --config .gcovrrc -d build

docs:
	@command -v sphinx-build >/dev/null || python -m pip install '.[doc]'
	cd docs && make html


.PHONY: build tests install install-dev docs format lint
