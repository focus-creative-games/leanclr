#!/bin/bash
# Build basic_test_runner on Linux/macOS.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../lib/out-dir.sh
source "$SCRIPT_DIR/../../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
RUNNER_SRC_DIR="$REPO_ROOT/src/tests/basic_test_runner"

BUILD_TYPE="${1:-Debug}"
ARCH="${2:-}"

if [[ -n "$ARCH" ]]; then
  CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic_test_runner "$BUILD_TYPE" "$ARCH")"
else
  CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic_test_runner "$BUILD_TYPE")"
fi

echo "Source dir: $RUNNER_SRC_DIR"
echo "Build dir: $CMAKE_BUILD_DIR"
mkdir -p "$CMAKE_BUILD_DIR"

cmake -S "$RUNNER_SRC_DIR" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$CMAKE_BUILD_DIR" --target test -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

EXE="$CMAKE_BUILD_DIR/bin/test"
if [ -f "$EXE" ]; then
  echo "Built: $EXE"
else
  echo "Warning: expected exe not found at $EXE"
fi
echo "Done."
