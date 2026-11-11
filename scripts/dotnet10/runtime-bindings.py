"""Snapshot runtime-bound CoreLib methods so AOT never compiles their IL placeholders."""
import argparse
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
TARGET = ROOT / 'src/leanaot/LeanAOT/runtime-apis/coreclr-net10/runtime-implemented.json'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
methods = set()
for source in sorted((ROOT / 'src/runtime/bcl/coreclr-net10').rglob('*.cpp')):
    # Join adjacent C++ string literals (including split signatures).
    for expression in re.findall(r'(?:"(?:\\.|[^"\\])*"\s*)+', source.read_text(encoding='utf-8-sig')):
        value = ''.join(re.findall(r'"((?:\\.|[^"\\])*)"', expression))
        if value.startswith('System.') and '::' in value and '\\' not in value and ' ' not in value:
            methods.add(value)
text = json.dumps(sorted(methods), indent=2) + '\n'
if args.check:
    if not TARGET.exists() or TARGET.read_text() != text:
        raise SystemExit('Runtime binding catalog is stale: run scripts/dotnet10/runtime-bindings.py')
else:
    TARGET.write_text(text)
print(f'{len(methods)} runtime-bound method names')
