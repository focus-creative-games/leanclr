"""Create an isolated resource benchmark snapshot with the historical linear slot lookup.

Only gchandle.cpp differs from current sources. No branch or worktree is changed.
"""
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
output = ROOT / 'out/net10-linear-baseline'
for directory in ['src/runtime', 'src/tests/net10-contracts']:
    shutil.copytree(ROOT / directory, output / directory, dirs_exist_ok=True)
for name in ['src/tests/aot-returns/test_assert.h', 'src/benchmarks/aot-baseline/nodefs.js']:
    target = output / name
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(ROOT / name, target)
baseline = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'show', 'ad0c8624bea62d7a01462c4aa21884aa51c84d7f:src/runtime/vm/gchandle.cpp'], cwd=ROOT)
(output / 'src/runtime/vm/gchandle.cpp').write_bytes(baseline)
print(output)
