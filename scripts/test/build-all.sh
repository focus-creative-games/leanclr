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
ARCH="${2:-}"

echo "=== Config: $CONFIG | Arch: $ARCH ==="

echo "build basic_test_runner"
"$SCRIPT_DIR/basic_test_runner/build.sh" "$CONFIG" "$ARCH"

echo "build managed tests"
pushd "$TESTS_DIR/managed" >/dev/null
dotnet build -c "$CONFIG"
cp -f "$(leanclr_dotnet_out_dir CoreTests "$CONFIG")/CoreTests.dll" MiscDlls/
cp -f "$(leanclr_dotnet_out_dir CorlibTests "$CONFIG")/CorlibTests.dll" MiscDlls/
popd >/dev/null

if [[ -n "$ARCH" ]]; then
    CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic_test_runner "$CONFIG" "$ARCH")"
else
    CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic_test_runner "$CONFIG")"
fi
if [[ -f "$CMAKE_BUILD_DIR/bin/$CONFIG/test" ]]; then
    EXE_DIR="$CMAKE_BUILD_DIR/bin/$CONFIG"
elif [[ -f "$CMAKE_BUILD_DIR/bin/test" ]]; then
    EXE_DIR="$CMAKE_BUILD_DIR/bin"
else
    EXE_DIR="$CMAKE_BUILD_DIR/bin/$CONFIG"
fi
DLLS_DIR="$EXE_DIR/dlls"

echo "stage test DLLs to $DLLS_DIR"
rm -rf "$DLLS_DIR"
mkdir -p "$DLLS_DIR"

cp -a "$REPO_ROOT/src/libraries/dotnetframework4.x" "$DLLS_DIR/"
cp -f "$REPO_ROOT/src/tests/managed/MiscDlls/"*.dll "$DLLS_DIR/" 2>/dev/null || true
CORETESTS_DLL="$(leanclr_dotnet_out_dir CoreTests "$CONFIG")/CoreTests.dll"
CORLIBTESTS_DLL="$(leanclr_dotnet_out_dir CorlibTests "$CONFIG")/CorlibTests.dll"
if [[ ! -f "$CORLIBTESTS_DLL" ]]; then
    CORLIBTESTS_DLL="$(leanclr_dotnet_out_dir CorlibTests Debug)/CorlibTests.dll"
fi
cp -f "$CORETESTS_DLL" "$DLLS_DIR/"
cp -f "$CORLIBTESTS_DLL" "$DLLS_DIR/"

echo "All tests built successfully."
