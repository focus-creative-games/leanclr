"""Build the production Cast/Delegate/NewArray boundary checks (Windows x64)."""
import argparse
import json
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'src/tests/aot-hotspots'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--all-protocols', action='store_true')
    ap.add_argument('--gc-debug', action='store_true')
    opts = ap.parse_args()
    os.environ['MSBuildEnableWorkloadResolver'] = 'false'
    out = ROOT / ('out/aot-hotspots-debug' if opts.gc_debug else 'out/aot-hotspots')
    stage = out / 'managed'
    stage.mkdir(parents=True, exist_ok=True)

    def run(command, label):
        print(label, flush=True)
        with (out / (label + '.log')).open('w', encoding='utf-8') as log:
            p = subprocess.run([str(x) for x in command], cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        if p.returncode:
            raise RuntimeError((out / (label + '.log')).read_text(errors='replace')[-3500:])

    bcl = ROOT / 'src/libraries/mono-4.5'
    csc = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
    run([csc, '/nologo', '/noconfig', '/optimize+', '/nostdlib+', '/target:exe', '/out:' + str(stage / 'HotspotFixtures.dll'),
         '/r:' + str(bcl / 'mscorlib.dll'), SOURCE / 'Fixtures.cs'], 'csc')
    shutil.copy2(bcl / 'mscorlib.dll', stage)
    (stage / 'HotspotFixtures.runtimeconfig.json').write_text(json.dumps({'runtimeOptions': {
        'tfm': 'net10.0', 'framework': {'name': 'Microsoft.NETCore.App', 'version': '10.0.8'}, 'rollForward': 'Disable'}}))
    run(['dotnet', stage / 'HotspotFixtures.dll'], 'net10-reference')
    run(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], 'tool-build')
    variants = [('default', [])]
    if opts.all_protocols:
        variants += [('protocols', ['--leanaot-lightweight-returns', '--leanaot-packed-int32-returns', '--leanaot-status-out-returns']),
                     ('shared', ['--leanaot-share-reference-generics'])]
    for name, flags in variants:
        generated = out / name / 'generated'
        build = out / 'build'
        run(['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '-d', stage, '-d', bcl,
             '-a', 'HotspotFixtures', '-o', generated, '--leanaot-aot-rule-file', SOURCE / 'aot.xml', *flags], name + '-generate')
        run(['cmake', '-S', SOURCE, '-B', build, '-A', 'x64', '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release',
             '-DHOTSPOT_AOT_CPP=' + str(generated), '-DHOTSPOT_DLLS=' + str(stage),
             '-DLEANCLR_GC_DEBUG=' + ('ON' if opts.gc_debug else 'OFF')], name + '-configure')
        run(['cmake', '--build', build, '--config', 'Release', '--parallel', '4'], name + '-build')
        run(['ctest', '--test-dir', build, '-C', 'Release', '--output-on-failure'], name + '-test')
    print('Hotspot boundary tests passed', flush=True)


if __name__ == '__main__':
    main()
