#!/bin/bash
# Repository build orchestration.
# Usage:
#   ./scripts/build.sh test build [Config] [Arch]
#   ./scripts/build.sh test run [Config]
#   ./scripts/build.sh runtime [Debug|Release]
#   ./scripts/build.sh leanaot publish
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

usage() {
    cat <<'EOF'
Usage:
  scripts/build.sh test build [Config] [Arch]
  scripts/build.sh test run [Config]
  scripts/build.sh runtime [Debug|Release]
  scripts/build.sh leanaot publish

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
        exec "$REPO_ROOT/src/runtime/build.sh" "$@"
        ;;
    leanaot)
        SUB="${1:-}"
        shift || true
        case "$SUB" in
            publish) exec "$SCRIPT_DIR/release/publish-leanaot.sh" "$@" ;;
            *) usage ;;
        esac
        ;;
    *)
        usage
        ;;
esac
