.PHONY: check docs configure build test format coverage tidy
CMAKE_ARGS ?=
CXX_FILES := include/loom/circuit.h tests/circuit_test.cpp tests/wrong_width.cpp examples/transfer.cpp

docs:
	python3 scripts/check_docs.py
configure:
	cmake -S . -B build $(CMAKE_ARGS)
build: configure
	cmake --build build --parallel 4
test: build
	ctest --test-dir build --output-on-failure
format:
	clang-format -i $(CXX_FILES)
tidy: build
	python3 scripts/tidy.py
coverage:
	cmake -S . -B build-coverage -DENABLE_COVERAGE=ON $(CMAKE_ARGS)
	cmake --build build-coverage --parallel 4
	python3 scripts/coverage.py
check: docs test tidy coverage
	clang-format --dry-run --Werror $(CXX_FILES)
