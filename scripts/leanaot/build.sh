#!/bin/bash
# Build LeanAOT to the default out/dotnet layout (Directory.Build.props).
# Usage: ./build.sh [Debug|Release]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/out-dir.sh
source "$SCRIPT_DIR/../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
CONFIG="${1:-Release}"

case "$CONFIG" in
    Debug|Release) ;;
    *)
        echo "ERROR: Configuration must be Debug or Release." >&2
        exit 1
        ;;
esac

LEANAOT_PROJ="$REPO_ROOT/src/leanaot/LeanAOT/LeanAOT.csproj"
OUT_DIR="$(leanclr_out_root)/dotnet/LeanAOT/$CONFIG/net8.0"

echo "Building LeanAOT ($CONFIG)..."
echo "Output: $OUT_DIR/"

dotnet build "$LEANAOT_PROJ" -c "$CONFIG"
echo "Build succeeded."
