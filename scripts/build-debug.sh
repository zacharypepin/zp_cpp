#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "==> Building project..."
cd zp_cpp
cmake --preset debug
cmake --build --preset debug
echo "==> Build complete"