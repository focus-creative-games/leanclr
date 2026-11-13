"""Fetch a pinned Windows-flavor CoreLib of the target pointer width for minimal gates."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--version', default='10.0.8')
parser.add_argument('--rid', choices=['win-x86', 'win-x64'], default='win-x86')
parser.add_argument('--output', type=Path)
args = parser.parse_args()
if not re.fullmatch(r'10\.\d+\.\d+', args.version): parser.error('A stable .NET 10 patch version is required')
output = (args.output or ROOT / f'out/net10-runtime-{args.rid}').resolve()
if not output.is_relative_to(ROOT / 'out'): parser.error('Output must stay under repository out/')
output.mkdir(parents=True, exist_ok=True)
package = f'microsoft.netcore.app.runtime.{args.rid}.{args.version}.nupkg'
archive = output / package
url = f'https://api.nuget.org/v3-flatcontainer/microsoft.netcore.app.runtime.{args.rid}/{args.version}/{package}'
if not archive.exists():
    partial = archive.with_suffix('.partial')
    with urllib.request.urlopen(url, timeout=60) as source, partial.open('wb') as target:
        import shutil
        shutil.copyfileobj(source, target)
    partial.replace(archive)
with zipfile.ZipFile(archive) as package_zip:
    for entry in package_zip.infolist():
        if not entry.filename.startswith(f'runtimes/{args.rid}/lib/net10.0/') or entry.is_dir(): continue
        target = (output / entry.filename).resolve()
        if not target.is_relative_to(output): raise RuntimeError('Unsafe package entry')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(package_zip.read(entry))
runtime = output / f'runtimes/{args.rid}/lib/net10.0'
corelib = runtime / 'System.Private.CoreLib.dll'
(output / 'provenance.json').write_text(json.dumps(dict(url=url, package_sha256=hashlib.sha256(archive.read_bytes()).hexdigest(),
    corelib_sha256=hashlib.sha256(corelib.read_bytes()).hexdigest()), indent=2) + '\n', encoding='utf-8')
print(runtime)
