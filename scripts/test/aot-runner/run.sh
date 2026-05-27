#!/bin/bash
# Build and run aot-tester with AotTests assembly.
# Usage: run.sh [Config] [Arch]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../lib/out-dir.sh
source "$SCRIPT_DIR/../../lib/out-dir.sh"

"$SCRIPT_DIR/build.sh" "$@"

REPO_ROOT="$(leanclr_repo_root)"
CONFIG="${1:-Debug}"
ARCH="${2:-}"

if [[ -n "$ARCH" ]]; then
  CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/aot-tester "$CONFIG" "$ARCH")"
  RUNNER="$CMAKE_BUILD_DIR/bin/$CONFIG/aot-tester"
  if [[ ! -f "$RUNNER" ]]; then
    RUNNER="$CMAKE_BUILD_DIR/bin/aot-tester"
  fi
else
  CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tests/aot-tester "$CONFIG")"
  RUNNER="$CMAKE_BUILD_DIR/bin/aot-tester"
fi

if [[ ! -f "$RUNNER" ]]; then
  echo "ERROR: aot-tester not found at '$RUNNER'. Run 'scripts/test/aot-runner/build.sh' first." >&2
  exit 1
fi

AOTTEST_DLL_DIR="$(leanclr_dotnet_out_dir AotTests "$CONFIG")"
COMMON_DLL="$(leanclr_dotnet_out_dir Common "$CONFIG")/Common.dll"
if [[ ! -f "$COMMON_DLL" ]]; then
  COMMON_DLL="$(leanclr_dotnet_out_dir Common Debug)/Common.dll"
fi
if [[ -f "$COMMON_DLL" ]]; then
  cp -f "$COMMON_DLL" "$AOTTEST_DLL_DIR/Common.dll"
fi

"$RUNNER" \
  -l "$REPO_ROOT/src/libraries/dotnetframework4.x" \
  -l "$AOTTEST_DLL_DIR" \
  -e AotTests.App::Main \
  AotTests
