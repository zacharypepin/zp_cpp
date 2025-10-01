#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Checking code formatting..."

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

# Run clang-format in dry-run mode to check formatting
failed=0
for file in $files; do
    if ! clang-format --style=file --dry-run --Werror "$file" 2>&1; then
        echo "Formatting issues found in: $file"
        failed=1
    fi
done

if [ $failed -eq 1 ]; then
    echo ""
    echo "==> Formatting check FAILED"
    echo "Run './scripts/format.sh' to fix formatting issues"
    exit 1
fi

echo "==> Formatting check PASSED"
