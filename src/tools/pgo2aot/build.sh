#!/bin/bash
# Build pgo2aot to the default out/dotnet layout (Directory.Build.props).
# Usage: ./build.sh [Debug|Release]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../../scripts/lib/out-dir.sh
source "$SCRIPT_DIR/../../../scripts/lib/out-dir.sh"

CONFIG="${1:-Release}"

case "$CONFIG" in
    Debug|Release) ;;
    *)
        echo "ERROR: Configuration must be Debug or Release." >&2
        exit 1
        ;;
esac

PGO2AOT_PROJ="$SCRIPT_DIR/Pgo2Aot.csproj"
OUT_DIR="$(leanclr_out_root)/dotnet/Pgo2Aot/$CONFIG/net8.0"

echo "Building pgo2aot ($CONFIG)..."
echo "Output: $OUT_DIR/"

dotnet build "$PGO2AOT_PROJ" -c "$CONFIG"

DLL="$OUT_DIR/pgo2aot.dll"
if [ -f "$DLL" ]; then
    echo "Built: $DLL"
else
    echo "Warning: expected dll not found at $DLL"
fi

echo "Build succeeded."
