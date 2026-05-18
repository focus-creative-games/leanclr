#!/bin/bash
set -e
# shellcheck source=../lib/out-dir.sh
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../lib/out-dir.sh"
OUT="$(leanclr_out_root)"
if [[ -d "$OUT" ]]; then
    echo "Removing $OUT ..."
    rm -rf "$OUT"
    echo "Done."
else
    echo "Nothing to clean: $OUT"
fi
