#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../../scripts/lib/out-dir.sh
source "$SCRIPT_DIR/../../../scripts/lib/out-dir.sh"

BUILD_TYPE=${1:-Debug}
CMAKE_BUILD_DIR="$(leanclr_cmake_build_dir tools/lean "$BUILD_TYPE")"

echo "Build dir: $CMAKE_BUILD_DIR"
mkdir -p "$CMAKE_BUILD_DIR"

cmake -S "$SCRIPT_DIR" -B "$CMAKE_BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$CMAKE_BUILD_DIR" --target lean -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

EXE="$CMAKE_BUILD_DIR/bin/lean"
if [ -f "$EXE" ]; then
  echo "Built: $EXE"
else
  echo "Warning: expected exe not found at $EXE"
fi
echo "Done."
