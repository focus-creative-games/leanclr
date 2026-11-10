#!/bin/bash
# Build leanclr runtime library on Linux and macOS.
# Usage: ./build.sh [Debug|Release] [clean] [native|minimal|custom]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/out-dir.sh
source "$SCRIPT_DIR/../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
RUNTIME_SRC="$REPO_ROOT/src/runtime"

BUILD_TYPE=Release
CLEAN=0
HOST=NATIVE

for arg in "$@"; do
    case "$arg" in
        Debug|Release) BUILD_TYPE="$arg" ;;
        clean|-clean) CLEAN=1 ;;
        native) HOST=NATIVE ;;
        minimal) HOST=PORTABLE ;;
        custom) HOST=CUSTOM ;;
    esac
done

MODULE=runtime
if [[ "$HOST" != NATIVE ]]; then MODULE="runtime/$HOST"; fi
CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir "$MODULE" "$BUILD_TYPE")"

echo "Build dir: $CMAKE_BUILD_DIR"
if [[ "$CLEAN" -eq 1 ]]; then
    rm -rf "$CMAKE_BUILD_DIR"
fi
mkdir -p "$CMAKE_BUILD_DIR"

cmake -S "$RUNTIME_SRC" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_TRY_COMPILE_CONFIGURATION="$BUILD_TYPE" -DLEANCLR_BCL_ADAPTER=MONO -DLEANCLR_HOST="$HOST" -DLEANCLR_FORCE_PORTABLE=OFF
cmake --build "$CMAKE_BUILD_DIR" -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Build finished in $CMAKE_BUILD_DIR"
