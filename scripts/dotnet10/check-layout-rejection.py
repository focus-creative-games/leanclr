"""Verify fail-closed CoreLib layout validation without changing the SDK copy."""
from pathlib import Path
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--runner", required=True)
parser.add_argument("--runtime-dir", required=True)
parser.add_argument("--assembly-dir", required=True)
parser.add_argument("--output", default="out/net10-layout-rejection")
args = parser.parse_args()
runtime = Path(args.runtime_dir).resolve()
output = Path(args.output).resolve()
output.mkdir(parents=True, exist_ok=True)
source = runtime / "System.Private.CoreLib.dll"
data = source.read_bytes()
needle = b"_message\x00"
if data.count(needle) != 1:
    raise SystemExit("Unexpected CoreLib string-heap shape; refusing an ambiguous mutation")
(output / source.name).write_bytes(data.replace(needle, b"_messagX\x00", 1))
result = subprocess.run([args.runner, "-l", str(output), "-l", args.assembly_dir, "-l", str(runtime), "-e",
    "ManagedNet10.Smoke.Program::TestPairArithmetic", "ManagedNet10.Smoke"], capture_output=True, timeout=30)
log = result.stdout + result.stderr
(output / "rejection.log").write_bytes(log)
if result.returncode == 0 or b"CoreLib layout mismatch" not in log:
    raise SystemExit("An incompatible CoreLib did not fail with the expected layout diagnostic: " + log.decode(errors="replace"))
print("Incompatible CoreLib rejected before managed entry execution")
