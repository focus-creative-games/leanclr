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

echo "build basic-tester"
"$SCRIPT_DIR/basic-tester/build.sh" "$CONFIG" "$ARCH"

echo "build managed tests"
pushd "$TESTS_DIR/managed" >/dev/null
dotnet build -c "$CONFIG"
popd >/dev/null

if [[ -n "$ARCH" ]]; then
    CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic-tester "$CONFIG" "$ARCH")"
else
    CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/basic-tester "$CONFIG")"
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
CORETESTS_DLL="$(leanclr_dotnet_out_dir CoreTests "$CONFIG")/CoreTests.dll"
CORLIBTESTS_DLL="$(leanclr_dotnet_out_dir CorlibTests "$CONFIG")/CorlibTests.dll"
if [[ ! -f "$CORLIBTESTS_DLL" ]]; then
    CORLIBTESTS_DLL="$(leanclr_dotnet_out_dir CorlibTests Debug)/CorlibTests.dll"
fi
cp -f "$CORETESTS_DLL" "$DLLS_DIR/"
cp -f "$CORLIBTESTS_DLL" "$DLLS_DIR/"

COMMON_DLL="$(leanclr_dotnet_out_dir Common "$CONFIG")/Common.dll"
if [[ ! -f "$COMMON_DLL" ]]; then
    COMMON_DLL="$(leanclr_dotnet_out_dir Common Debug)/Common.dll"
fi
cp -f "$COMMON_DLL" "$DLLS_DIR/"

REFNETSTANDARD_DLL="$(leanclr_dotnet_out_dir RefNetstandard "$CONFIG")/RefNetstandard.dll"
if [[ ! -f "$REFNETSTANDARD_DLL" ]]; then
    REFNETSTANDARD_DLL="$(leanclr_dotnet_out_dir RefNetstandard Debug)/RefNetstandard.dll"
fi
cp -f "$REFNETSTANDARD_DLL" "$DLLS_DIR/"

GCTESTS_DLL="$(leanclr_dotnet_out_dir GcTests "$CONFIG")/GcTests.dll"
if [[ ! -f "$GCTESTS_DLL" ]]; then
    GCTESTS_DLL="$(leanclr_dotnet_out_dir GcTests Debug)/GcTests.dll"
fi
cp -f "$GCTESTS_DLL" "$DLLS_DIR/"

ILTESTS_DLL="$(leanclr_dotnet_out_dir ILTests "$CONFIG")/ILTests.dll"
ILTESTS_NATIVE_DLL="$(leanclr_dotnet_out_dir ILTests "$CONFIG")/ILTests.Native.dll"
if [[ ! -f "$ILTESTS_DLL" ]]; then
    ILTESTS_DLL="$(leanclr_dotnet_out_dir ILTests Debug)/ILTests.dll"
fi
if [[ ! -f "$ILTESTS_NATIVE_DLL" ]]; then
    ILTESTS_NATIVE_DLL="$(leanclr_dotnet_out_dir ILTests Debug)/ILTests.Native.dll"
fi
cp -f "$ILTESTS_DLL" "$DLLS_DIR/"
cp -f "$ILTESTS_NATIVE_DLL" "$DLLS_DIR/"

echo "All tests built successfully."
