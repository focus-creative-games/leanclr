#!/bin/bash
# CI entry point: build and run all tests. Matches .github/workflows/ci.yml.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG="${1:-Release}"

"$SCRIPT_DIR/test/build-all.sh" "$CONFIG"
"$SCRIPT_DIR/test/run.sh" "$CONFIG"
