#!/bin/bash
# Build aot-tester on Linux/macOS.
# Usage: build.sh [Config] [Arch]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../lib/out-dir.sh
source "$SCRIPT_DIR/../../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
AOT_TESTER_SRC="$REPO_ROOT/src/tests/aot-tester"

BUILD_TYPE="${1:-Debug}"
ARCH="${2:-}"

if [[ -n "$ARCH" ]]; then
  CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/aot-tester "$BUILD_TYPE" "$ARCH")"
else
  CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/aot-tester "$BUILD_TYPE")"
fi

echo "=== Config: $BUILD_TYPE | Arch: ${ARCH:-default} ==="
echo "Source dir: $AOT_TESTER_SRC"
echo "Build dir: $CMAKE_BUILD_DIR"
mkdir -p "$CMAKE_BUILD_DIR"

cmake -S "$AOT_TESTER_SRC" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$CMAKE_BUILD_DIR" --target aot-tester -- -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)"

EXE="$CMAKE_BUILD_DIR/bin/aot-tester"
if [[ -f "$EXE" ]]; then
  echo "Built: $EXE"
else
  echo "Warning: expected exe not found at $EXE"
fi
echo "Done."
