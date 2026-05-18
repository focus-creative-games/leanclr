#!/bin/bash
# Build the C++ test runner and managed test assemblies.
# Usage: build-all.sh [Config] [Arch]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
TESTS_DIR="$REPO_ROOT/src/tests"
# shellcheck source=../lib/out-dir.sh
source "$SCRIPT_DIR/../lib/out-dir.sh"

CONFIG="${1:-Debug}"
ARCH="${2:-x64}"

echo "=== Config: $CONFIG | Arch: $ARCH ==="

echo "build basic_test_runner"
pushd "$TESTS_DIR/basic_test_runner" >/dev/null
./build.sh "$CONFIG"
popd >/dev/null

echo "build managed tests"
pushd "$TESTS_DIR/managed" >/dev/null
dotnet build -c "$CONFIG"
cp -f "$(leanclr_dotnet_out_dir CoreTests "$CONFIG")/CoreTests.dll" MiscDlls/
cp -f "$(leanclr_dotnet_out_dir CorlibTests "$CONFIG")/CorlibTests.dll" MiscDlls/
popd >/dev/null

echo "All tests built successfully."
