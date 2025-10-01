#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

echo "======================================"
echo "Running full CI pipeline"
echo "======================================"
echo ""

./scripts/format-check.sh
echo ""

./scripts/build-debug.sh
echo ""

./scripts/test-debug.sh
echo ""

echo "======================================"
echo "CI pipeline completed successfully"
echo "======================================"
