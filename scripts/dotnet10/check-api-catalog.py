"""Check that metadata-only assembly/type matches never authorize API members."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--input', type=Path, required=True)
args = parser.parse_args()
scanner = ROOT / 'out/dotnet/Net10ApiScan/Release/net8.0/Net10ApiScan.dll'
base = json.loads((ROOT / 'src/tools/net10apiscan/minimal-net10-whitelist.json').read_text())
collect = 'System.Void System.GC::Collect()'
with tempfile.TemporaryDirectory(prefix='net10-api-catalog-', dir=ROOT / 'out') as temp:
    directory = Path(temp)
    def scan(catalog):
        config, report = directory / 'catalog.json', directory / 'report.json'
        config.write_text(json.dumps(catalog))
        process = subprocess.run(['dotnet', str(scanner), '--input', str(args.input.resolve()),
                                  '--whitelist', str(config), '--report', str(report)],
                                 cwd=ROOT, capture_output=True, text=True)
        assert process.returncode == 2, process.stdout + process.stderr
        return json.loads(report.read_text(encoding='utf-8-sig'))
    base['validated']['assemblyPrefixes'] = ['System.']
    base['validated']['typePrefixes'] = ['System.']
    result = scan(base)
    assert result['PendingReferenceCount'] > 0
    assert not any(item['Name'] == collect for item in result['PendingReferences'])
    base['unsupported']['memberNames'] = [collect]
    result = scan(base)
    assert any(item['Name'] == collect for item in result['UnsupportedReferences'])
    base['validated']['memberNames'] = []
    base['unsupported']['memberNames'] = []
    result = scan(base)
    assert any(item['Name'] == collect for item in result['PendingReferences'])
print('API catalog classification contracts passed')
