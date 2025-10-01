#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Building project (release)..."
cd zp_cpp
cmake --preset release
cmake --build --preset release
echo "==> Release build complete"
