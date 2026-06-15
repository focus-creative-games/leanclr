#!/bin/bash
# Repository build orchestration.
# Usage:
#   ./scripts/build.sh test build [Config] [Arch]
#   ./scripts/build.sh test run [Config]
#   ./scripts/build.sh runtime [Debug|Release]
#   ./scripts/build.sh leanaot [Debug|Release]
#   ./scripts/build.sh aot-tester [build] [Config] [Arch]
#   ./scripts/build.sh aot-tester gen-cpp
#   ./scripts/build.sh aot-tester gen-cpp-posix
#   ./scripts/build.sh aot-tester run [Config] [Arch]
#   ./scripts/build.sh aot-tester build-wasm [Config]
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
  scripts/build.sh aot-tester [build] [Config] [Arch]
  scripts/build.sh aot-tester gen-cpp
  scripts/build.sh aot-tester gen-cpp-posix
  scripts/build.sh aot-tester run [Config] [Arch]
  scripts/build.sh aot-tester build-wasm [Config]
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
    aot-tester)
        SUB="${1:-}"
        shift || true
        case "$SUB" in
            gen-cpp)       exec "$SCRIPT_DIR/test/aot-tester/gen_cpp.sh" ;;
            gen-cpp-posix) exec "$SCRIPT_DIR/test/aot-tester/gen_cpp_posix.sh" ;;
            run)           exec "$SCRIPT_DIR/test/aot-tester/run.sh" "$@" ;;
            build-wasm)    exec "$SCRIPT_DIR/test/aot-tester/build-wasm.sh" "$@" ;;
            build)         exec "$SCRIPT_DIR/test/aot-tester/build.sh" "$@" ;;
            "")            exec "$SCRIPT_DIR/test/aot-tester/build.sh" "$@" ;;
            *)             exec "$SCRIPT_DIR/test/aot-tester/build.sh" "$SUB" "$@" ;;
        esac
        ;;
    *)
        usage
        ;;
esac
