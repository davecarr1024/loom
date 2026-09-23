#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bazel test //...
bazel coverage --combined_report=lcov //:circuit_tests
python3 scripts/coverage.py
