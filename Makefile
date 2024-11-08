
build:
	python -m build

install-dev:
	python -m pip install -ve . \
		-Ccmake.define.CMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-Cbuild-dir=build \
		-Ccmake.build-type=Profile \
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
	@python -m pip install -v '.[dev]'
	python -m pytest --cov=pymusly

coverage:
	@(command -v lcov >/dev/null && command -v genhtml >/dev/null) || (echo "please install 'lcov' and 'genhtml'" && exit 1)
	python -m pip install -v '.[dev]' \
		-Cbuild-dir=build \
		-Ccmake.build-type=Profile \
		-Cbuild.verbose=true
	lcov --zerocounter --directory build/
	python -m pytest --cov=pymusly --cov-report=term --cov-report=lcov
	lcov --capture \
		--directory build/ \
		--include "${PWD}/native/*" \
		--fail-under-lines 80 \
		-o build/coverage-cpp.info
	genhtml build/coverage-*.info --output-directory coverage


docs:
	@command -v sphinx-build >/dev/null || python -m pip install '.[doc]'
	cd docs && make html


.PHONY: build coverage tests install install-dev docs format lint
