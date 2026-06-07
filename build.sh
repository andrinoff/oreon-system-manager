#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR" --parallel "$(nproc)"
