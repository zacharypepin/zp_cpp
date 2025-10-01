#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

VERSION="${1:?Error: VERSION required. Usage: ./scripts/release.sh VERSION}"

echo "======================================"
echo "Running release pipeline (version: $VERSION)"
echo "======================================"
echo ""

# Core validation steps (identical to GitHub Actions)
./scripts/build-release.sh
echo ""

./scripts/test-release.sh
echo ""

./scripts/package.sh "$VERSION"
echo ""

echo "======================================"
echo "Release pipeline completed successfully"
echo "======================================"

# Local pragmatic enhancements
ARTIFACT_PATH="zp_cpp-${VERSION}-linux-x86_64.tar.gz"
ARTIFACT_SIZE=$(du -h "$ARTIFACT_PATH" | cut -f1)

echo ""
echo "📦 Release artifact verified: $ARTIFACT_PATH ($ARTIFACT_SIZE)"
echo ""

rm -rf artifacts
rm -f "$ARTIFACT_PATH"
echo "🧹 Cleaned up temporary artifacts and tarball"
