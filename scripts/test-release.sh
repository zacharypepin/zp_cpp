#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Running tests (release)..."
cd zp_cpp
ctest --test-dir build/release --output-on-failure
echo "==> Release tests complete"
