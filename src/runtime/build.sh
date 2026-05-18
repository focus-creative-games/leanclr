#!/bin/bash
# build.sh - Build script for leanclr runtime on Linux and macOS
# Usage: ./build.sh [Debug|Release]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../scripts/lib/out-dir.sh
source "$SCRIPT_DIR/../../scripts/lib/out-dir.sh"

BUILD_TYPE=${1:-Release}
CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir runtime "$BUILD_TYPE")"

echo "Build dir: $CMAKE_BUILD_DIR"
mkdir -p "$CMAKE_BUILD_DIR"

cmake -S "$SCRIPT_DIR" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$CMAKE_BUILD_DIR" -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Build finished in $CMAKE_BUILD_DIR"
