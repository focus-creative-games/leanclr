#!/bin/bash
# Run all LeanCLR unit tests.
# Usage: run.sh [Config] [Arch]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/out-dir.sh
source "$SCRIPT_DIR/../lib/out-dir.sh"

CONFIG="${1:-Debug}"
ARCH="${2:-}"

if [[ -n "$ARCH" ]]; then
    CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic-tester "$CONFIG" "$ARCH")"
    RUNNER="$CMAKE_BUILD_DIR/bin/$CONFIG/test"
else
    CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic-tester "$CONFIG")"
    RUNNER="$CMAKE_BUILD_DIR/bin/test"
fi

if [ ! -f "$RUNNER" ]; then
    echo "ERROR: Test runner not found at '$RUNNER'. Run 'scripts/test/build-all.sh' first."
    exit 1
fi

"$RUNNER"
RESULT=$?
if [ $RESULT -ne 0 ]; then
    echo "Some tests failed."
    exit $RESULT
fi

echo "All tests passed."
