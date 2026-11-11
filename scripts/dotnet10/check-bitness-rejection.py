"""Verify a foreign-width CoreLib fails before executing any managed entry."""
import argparse
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--runner', type=Path, required=True)
p.add_argument('--foreign-runtime', type=Path, required=True)
p.add_argument('--node', action='store_true')
a = p.parse_args()
stage = ROOT / ('out/net10-bitness-rejection-wasm' if a.node else 'out/net10-bitness-rejection-native')
stage.mkdir(parents=True, exist_ok=True)
shutil.copy2(a.foreign_runtime / 'System.Private.CoreLib.dll', stage)
directory = stage.relative_to(ROOT).as_posix() if a.node else str(stage)
command = (['node'] if a.node else []) + [str(a.runner), '-l', directory,
    '-l', 'out/dotnet/ManagedNet10.Smoke/Release/net10.0', '-e',
    'ManagedNet10.Smoke.Program::TestPairArithmetic', 'ManagedNet10.Smoke']
result = subprocess.run(command, cwd=ROOT, capture_output=True, timeout=30)
log = result.stdout + result.stderr
(stage / 'rejection.log').write_bytes(log)
if result.returncode == 0 or b'CoreLib pointer size' not in log or b'incompatible' not in log:
    raise RuntimeError('Missing pointer-width rejection: ' + log.decode(errors='replace'))
print('Foreign-width CoreLib rejected before managed entry execution')
