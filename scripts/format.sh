#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Formatting code..."

# Find all C++ source and header files
files=$(find zp_cpp/src zp_cpp/include zp_cpp/tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.tpp" -o -name "*.t.cpp" \) 2>/dev/null || true)

if [ -z "$files" ]; then
    echo "No C++ files found"
    exit 0
fi

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found. Please install clang-format."
    exit 1
fi

# Format all files
for file in $files; do
    echo "Formatting: $file"
    clang-format --style=file -i "$file"
done

echo "==> Formatting complete"
