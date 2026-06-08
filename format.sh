#!/usr/bin/env bash
# Apply clang-format to all source files in-place.
# Run this once before pushing after adding .clang-format to the repo.
set -euo pipefail

find src tests -type f \( -name '*.cpp' -o -name '*.h' \) \
    | xargs clang-format -i

echo "Formatting complete."
