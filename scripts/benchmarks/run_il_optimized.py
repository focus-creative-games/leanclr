"""Build production optimizations and measure against preserved pre-change AOT."""
import argparse
import ctypes
import hashlib
import json
import os
import random
import shutil
import sys
import statistics
from pathlib import Path
from run_protocols import ROOT, invoke


def paired_controls(old, out):
    """Use identical n on all backends: allocation growth can depend on total n."""
    os.environ['DOTNET_TieredCompilation'] = '0'
    os.environ['DOTNET_ReadyToRun'] = '0'
    k32 = ctypes.windll.kernel32
    k32.GetCurrentProcess.restype = ctypes.c_void_p
    k32.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    if not k32.SetProcessAffinityMask(k32.GetCurrentProcess(), 4):
        raise ctypes.WinError()
    cases = [('Cast',20,20000000), ('Delegate',17,20000000), ('NewArray',23,2000000)]
    rows = []
    rng = random.Random(20261114)
    for sample in range(7):
        jobs = [(e,n,k,count) for n,k,count in cases for e in ['before-aot','aot','il2cpp','net10']]
        rng.shuffle(jobs)
        for engine, name, kind, count in jobs:
            if engine in ('before-aot','aot'):
                base = old if engine == 'before-aot' else out
                command = [base / 'native/Release/il-aot.exe', out / 'managed', name, count, 19]
            elif engine == 'il2cpp':
                command = [out / 'il2cpp/AotBaseline.exe', kind, count]
            else:
                command = ['dotnet', out / 'managed/AotBaseline.dll', kind, count]
            output, _ = invoke(command, out / ('paired-%d-%s-%s.jsonl' % (sample,engine,name)))
            record = next(json.loads(s) for s in output.splitlines() if s.startswith('{'))
            rows.append(dict(record, engine=engine, sample=sample))
    summary = []
    for name,kind,count in cases:
        assert len({r['checksum'] for r in rows if r['workload'] == name}) == 1
        for engine in ['before-aot','aot','il2cpp','net10']:
            values = [r['ns_per_op'] for r in rows if r['workload'] == name and r['engine'] == engine]
            summary.append(dict(workload=name, engine=engine, iterations=count, median_ns=statistics.median(values),
                                min_ns=min(values), max_ns=max(values)))
    (out / 'paired-results.json').write_text(json.dumps(dict(samples=rows, summary=summary, affinity_mask=4), indent=2))
    print('Identical-count controls passed: 84 samples', flush=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline', type=Path, default=ROOT / 'out/performance/il-families')
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/il-families-optimized')
    ap.add_argument('--build-only', action='store_true')
    ap.add_argument('--paired-only', action='store_true')
    opts = ap.parse_args()
    old, out = opts.baseline.resolve(), opts.output.resolve()
    if out == old:
        ap.error('Preserve the original baseline in a different directory')
    out.mkdir(parents=True, exist_ok=True)
    if opts.paired_only:
        paired_controls(old, out)
        return
    os.environ['MSBuildEnableWorkloadResolver'] = 'false'
    shutil.copytree(old / 'managed', out / 'managed', dirs_exist_ok=True)
    shutil.copytree(old / 'il2cpp', out / 'il2cpp', dirs_exist_ok=True)
    shutil.copy2(old / 'il-inventory.tsv', out / 'il-inventory.tsv')
    commands = [
        ['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'],
        ['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '-d', out / 'managed',
         '-d', ROOT / 'src/libraries/mono-4.5', '-a', 'AotBaseline', '-o', out / 'generated',
         '--leanaot-aot-rule-file', ROOT / 'src/benchmarks/il-families/aot.xml'],
        ['cmake', '-S', ROOT / 'src/benchmarks/il-families', '-B', out / 'native', '-A', 'x64',
         '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release', '-DLEANCLR_BENCH_GENERATED_DIR=' + str(out / 'generated'),
         '-DLEANCLR_GC_DEBUG=OFF', '-DLEANCLR_ENABLE_PROFILE=OFF'],
        ['cmake', '--build', out / 'native', '--config', 'Release', '--parallel', '4'],
    ]
    for index, command in enumerate(commands):
        print('Production build step %d/%d' % (index+1, len(commands)), flush=True)
        invoke(command, out / ('production-build-%d.log' % index))
    paths = [old / 'native/Release/il-aot.exe', old / 'native/runtime/Release/leanclr.lib',
             out / 'native/Release/il-aot.exe', out / 'native/runtime/Release/leanclr.lib', out / 'managed/AotBaseline.dll']
    manifest = dict(commands=[[str(x) for x in c] for c in commands],
                    baseline_manifest=json.loads((old / 'build-manifest.json').read_text()),
                    reused_managed_input=True, reused_il2cpp=True,
                    sha256={str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
                    artifact_bytes={str(p.relative_to(ROOT)): p.stat().st_size for p in paths})
    (out / 'build-manifest.json').write_text(json.dumps(manifest, indent=2))
    if not opts.build_only:
        invoke([sys.executable, ROOT / 'scripts/benchmarks/run_il_families.py', '--skip-build', '--output', out,
                '--baseline-aot', old / 'native/Release/il-aot.exe'], out / 'measurement.log')
        paired_controls(old, out)
        print(out / 'results.json')


if __name__ == '__main__':
    main()
