#!/bin/bash
# Build leanclr runtime library on Linux and macOS.
# Usage: ./build.sh [Debug|Release] [clean]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/out-dir.sh
source "$SCRIPT_DIR/../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
RUNTIME_SRC="$REPO_ROOT/src/runtime"

BUILD_TYPE=Release
CLEAN=0

for arg in "$@"; do
    case "$arg" in
        Debug|Release) BUILD_TYPE="$arg" ;;
        clean|-clean) CLEAN=1 ;;
    esac
done

CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir runtime "$BUILD_TYPE")"

echo "Build dir: $CMAKE_BUILD_DIR"
if [[ "$CLEAN" -eq 1 ]]; then
    rm -rf "$CMAKE_BUILD_DIR"
fi
mkdir -p "$CMAKE_BUILD_DIR"

cmake -S "$RUNTIME_SRC" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$CMAKE_BUILD_DIR" -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Build finished in $CMAKE_BUILD_DIR"
