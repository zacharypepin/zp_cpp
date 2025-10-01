#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Running tests..."
cd zp_cpp
ctest --test-dir build/debug --output-on-failure
echo "==> Tests complete"
