#!/usr/bin/env bash
set -euo pipefail
clang-format-19 --dry-run --Werror "$@"
