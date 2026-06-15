#!/bin/bash
# Build aot-tester for WebAssembly (Emscripten).
# Usage: build-wasm.sh [Config]
# Requires emcmake/emmake on PATH.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../lib/out-dir.sh
source "$SCRIPT_DIR/../../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
AOT_TESTER_SRC="$REPO_ROOT/src/tests/aot-tester"
CONFIG="${1:-Debug}"
CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/aot-tester "$CONFIG" wasm)"

echo "=== Config: $CONFIG | Arch: wasm ==="
echo "Build dir: $CMAKE_BUILD_DIR"
mkdir -p "$CMAKE_BUILD_DIR"

echo "[1/2] CMake configure (Emscripten)..."
emcmake cmake -S "$AOT_TESTER_SRC" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$CONFIG"

echo "[2/2] Build target 'aot-tester'..."
emmake cmake --build "$CMAKE_BUILD_DIR" --parallel --target aot-tester

echo "Done. Output: $CMAKE_BUILD_DIR/bin/aot-tester.js and aot-tester.wasm"
