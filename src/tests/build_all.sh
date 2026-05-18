#!/bin/bash
# Deprecated: use scripts/test/build-all.sh
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
exec "$REPO_ROOT/scripts/test/build-all.sh" "$@"
