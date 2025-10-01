#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

VERSION="${1:-dev}"

echo "==> Packaging release artifacts (version: $VERSION)..."

# Create artifact directories in repo root
mkdir -p artifacts/lib
mkdir -p artifacts/include/zp_cpp

# Copy library and headers
cp zp_cpp/build/release/libzp_cpp.a artifacts/lib/
cp zp_cpp/include/zp_cpp/*.hpp artifacts/include/zp_cpp/

# Create tarball in repo root
tar -czf zp_cpp-${VERSION}-linux-x86_64.tar.gz -C artifacts .

echo "==> Package created: zp_cpp-${VERSION}-linux-x86_64.tar.gz"
