#!/bin/bash
# Generate aot-tester C++ sources (Windows BCL layout).
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../../lib/out-dir.sh
source "$SCRIPT_DIR/../../lib/out-dir.sh"

REPO_ROOT="$(leanclr_repo_root)"
AOT_TESTER_SRC="$REPO_ROOT/src/tests/aot-tester"
LEANAOT_EXE="$(leanclr_dotnet_sdk_exe LeanAOT Debug net8.0)"
AOTTEST_DLL_DIR="$(leanclr_dotnet_out_dir AotTests Debug)"
COMMON_DLL="$(leanclr_dotnet_out_dir Common Debug)/Common.dll"
CPP_OUT="$AOT_TESTER_SRC/cpp"

echo "[1/3] Building managed AotTests project (Debug)..."
dotnet build "$REPO_ROOT/src/tests/managed/AotTests/AotTests.csproj" -c Debug

if [[ -f "$COMMON_DLL" ]]; then
  cp -f "$COMMON_DLL" "$AOTTEST_DLL_DIR/Common.dll"
fi

echo "[2/3] Building LeanAOT (Debug)..."
dotnet build "$REPO_ROOT/src/leanaot/LeanAOT/LeanAOT.csproj" -c Debug

echo "[3/3] Running LeanAOT..."
"$LEANAOT_EXE" \
  -o "$CPP_OUT" \
  --emit-null-checks \
  --enable-array-bounds-check \
  --leanaot-may-throw-exception-in-icall \
  -d "$REPO_ROOT/src/libraries/mono-4.5" \
  -d "$AOTTEST_DLL_DIR" \
  --leanaot-aot-rule-file "$AOT_TESTER_SRC/aot-rules-mscorlib.xml" \
  --leanaot-aot-rule-file "$AOT_TESTER_SRC/aot-rules-test.xml" \
  --leanaot-pgo-rule-file "$AOT_TESTER_SRC/pgo-aot.xml" \
  -a mscorlib \
  -a System \
  -a System.Core \
  -a AotTests

echo "Done. Output: $CPP_OUT"
