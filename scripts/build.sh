#!/bin/bash
# Repository build orchestration.
# Usage:
#   ./scripts/build.sh test build [Config] [Arch]
#   ./scripts/build.sh test run [Config]
#   ./scripts/build.sh runtime [Debug|Release]
#   ./scripts/build.sh leanaot [Debug|Release]
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

usage() {
    cat <<'EOF'
Usage:
  scripts/build.sh test build [Config] [Arch]
  scripts/build.sh test run [Config]
  scripts/build.sh runtime [Debug|Release]
  scripts/build.sh leanaot [Debug|Release]

aot-runner commands (Windows): scripts/leanaot/aot-runner/*.bat
EOF
    exit 1
}

CMD="${1:-}"
shift || true

case "$CMD" in
    test)
        SUB="${1:-}"
        shift || true
        case "$SUB" in
            build) exec "$SCRIPT_DIR/test/build-all.sh" "$@" ;;
            run)   exec "$SCRIPT_DIR/test/run.sh" "$@" ;;
            *) usage ;;
        esac
        ;;
    runtime)
        exec "$SCRIPT_DIR/runtime/build.sh" "$@"
        ;;
    leanaot)
        CONFIG="${1:-Release}"
        exec "$SCRIPT_DIR/leanaot/build.sh" "$CONFIG"
        ;;
    *)
        usage
        ;;
esac
