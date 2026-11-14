"""Same-IL Win64 interpreter / LeanAOT / Unity IL2CPP / .NET 10 comparison."""
import argparse
import hashlib
import json
import os
import platform
import random
import statistics
import subprocess
import ctypes
from datetime import datetime, timezone
from pathlib import Path
from run_protocols import ROOT, invoke
from run_aot import DEPS, build_il2cpp

NAMES = 'Int32 Int64 DivRem Float32 Float64 Convert Checked Branch Switch ArrayLoad ArrayStore Field StaticField ByRef Calls Virtual Interface Delegate Struct Generic Cast Boxing Allocation NewArray EhNormal EhThrow'.split()
SOURCE = ROOT / 'src/benchmarks/il-families'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--unity', type=Path, default=Path('D:/UnityHubs/2022.3.62f3'))
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/il-families')
    ap.add_argument('--skip-build', action='store_true')
    ap.add_argument('--build-only', action='store_true')
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--target-ms', type=int, default=100)
    ap.add_argument('--baseline-aot', type=Path, help='Optional preserved pre-optimization AOT executable for a same-session control')
    opts = ap.parse_args()
    if opts.samples < 3 or opts.target_ms < 20:
        ap.error('Use >=3 samples and >=20ms target')
    out = opts.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    stage = out / 'managed'
    stage.mkdir(exist_ok=True)
    os.environ['MSBuildEnableWorkloadResolver'] = 'false'
    # A stable, optimized JIT configuration: no ReadyToRun, tiering or dynamic PGO.
    os.environ['DOTNET_TieredCompilation'] = '0'
    os.environ['DOTNET_ReadyToRun'] = '0'
    commands = []

    def run(cmd, log):
        commands.append([str(a) for a in cmd])
        return invoke(cmd, out / log)[0]

    if not opts.skip_build:
        csc = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
        run([csc, '/nologo', '/noconfig', '/optimize+', '/nostdlib+', '/target:exe',
             '/out:' + str(stage / 'AotBaseline.dll'),
             '/r:' + str(ROOT / 'src/libraries/mono-4.5/mscorlib.dll'),
             '/r:' + str(ROOT / 'src/libraries/mono-4.5/System.dll'), SOURCE / 'Benchmarks.cs'], 'csc.log')
        import shutil
        for dep in DEPS:
            shutil.copy2(ROOT / 'src/libraries/mono-4.5' / (dep + '.dll'), stage)
        (stage / 'AotBaseline.runtimeconfig.json').write_text(json.dumps({'runtimeOptions': {
            'tfm': 'net10.0', 'framework': {'name': 'Microsoft.NETCore.App', 'version': '10.0.8'},
            'rollForward': 'Disable'}}))
        run([csc, '/nologo', '/optimize+', '/out:' + str(out / 'ILInventory.dll'),
             SOURCE / 'ILInventory.cs'], 'inventory-build.log')
        shutil.copy2(stage / 'AotBaseline.runtimeconfig.json', out / 'ILInventory.runtimeconfig.json')
        run(['dotnet', out / 'ILInventory.dll', stage / 'AotBaseline.dll'], 'il-inventory.tsv')
        run(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], 'tool-build.log')
        run(['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '-d', stage,
             '-d', ROOT / 'src/libraries/mono-4.5', '-a', 'AotBaseline', '-o', out / 'generated',
             '--leanaot-aot-rule-file', SOURCE / 'aot.xml'], 'generate.log')
        run(['cmake', '-S', SOURCE, '-B', out / 'native', '-A', 'x64',
             '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release',
             '-DLEANCLR_BENCH_GENERATED_DIR=' + str(out / 'generated'),
             '-DLEANCLR_ENABLE_PROFILE=OFF', '-DLEANCLR_GC_DEBUG=OFF'], 'configure.log')
        run(['cmake', '--build', out / 'native', '--config', 'Release', '--parallel', '4'], 'native-build.log')
        helper = build_il2cpp(opts.unity, out, stage, commands)
        (out / 'build-manifest.json').write_text(json.dumps({'commands': commands, 'standalone_thread_hook': helper}, indent=2))
    if opts.build_only:
        return
    kernel32 = ctypes.windll.kernel32
    kernel32.GetCurrentProcess.restype = ctypes.c_void_p
    kernel32.GetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t)]
    kernel32.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    handle = kernel32.GetCurrentProcess()
    available, system = ctypes.c_size_t(), ctypes.c_size_t()
    if not kernel32.GetProcessAffinityMask(handle, ctypes.byref(available), ctypes.byref(system)):
        raise ctypes.WinError()
    mask = 4 if available.value & 4 else available.value & -available.value
    if not kernel32.SetProcessAffinityMask(handle, mask):
        raise ctypes.WinError()
    engines = ['interp', 'aot', 'il2cpp', 'net10']
    if opts.baseline_aot:
        engines.append('before-aot')

    def measure(engine, k, n, log):
        if engine == 'before-aot':
            cmd = [opts.baseline_aot.resolve(), stage, NAMES[k], n, 19]
        elif engine in ('interp', 'aot'):
            cmd = [out / ('native/Release/il-' + engine + '.exe'), stage, NAMES[k], n, 19]
        elif engine == 'il2cpp':
            cmd = [out / 'il2cpp/AotBaseline.exe', k, n]
        else:
            cmd = ['dotnet', stage / 'AotBaseline.dll', k, n]
        records = [json.loads(s) for s in run(cmd, log).splitlines() if s.startswith('{')]
        if len(records) != 1 or records[0]['workload'] != NAMES[k]:
            raise RuntimeError('Unexpected output: ' + str(cmd))
        return dict(records[0], engine=engine)

    # A shared small count is a correctness gate before any calibration/timing.
    validation = []
    for k, name in enumerate(NAMES):
        group = [measure(e, k, 4096, 'verify-' + e + '-' + name + '.jsonl') for e in engines]
        if len({r['checksum'] for r in group}) != 1:
            raise RuntimeError('Verification mismatch: ' + str(group))
        validation.extend(group)
    print('All 26 kernels agree across %d engines at n=4096' % len(engines), flush=True)
    counts = {}
    oracles = {}
    for k, name in enumerate(NAMES):
        for engine in engines:
            probe = measure(engine, k, 65536, 'calibrate-' + engine + '-' + name + '.jsonl')
            n = max(16384, min(100000000, int(opts.target_ms * 1e6 / max(probe['ns_per_op'], .1))))
            # All engines use the same IL; validate every measured count against .NET.
            counts[engine, k] = n
            if (k, n) not in oracles:
                oracles[k, n] = measure('net10', k, n, 'oracle-' + engine + '-' + name + '.jsonl')['checksum']
    rows = []
    rng = random.Random(20261114)
    for sample in range(opts.samples):
        jobs = [(e, k) for e in engines for k in range(len(NAMES))]
        rng.shuffle(jobs)
        for engine, k in jobs:
            n = counts[engine, k]
            row = measure(engine, k, n, 'sample-%d-%s-%s.jsonl' % (sample, engine, NAMES[k]))
            if row['checksum'] != oracles[k, n]:
                raise RuntimeError('Measured checksum mismatch: ' + str(row))
            rows.append(dict(row, sample=sample))
        print('Sample %d/%d complete' % (sample + 1, opts.samples), flush=True)
        (out / 'samples.partial.json').write_text(json.dumps(rows, indent=2))
    summary = []
    for k, name in enumerate(NAMES):
        for engine in engines:
            group = [r for r in rows if r['engine'] == engine and r['workload'] == name]
            values = [r['ns_per_op'] for r in group]
            summary.append(dict(workload=name, engine=engine, iterations=counts[engine, k],
                                median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values),
                                entry=group[0].get('entry'), interpreted_ir=group[0].get('interpreted_ir')))
    result = dict(schema=1, measured_at_utc=datetime.now(timezone.utc).isoformat(),
                  platform=platform.platform(), processor=platform.processor(),
                  commit=subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  samples=rows, summary=summary, validation=validation, target_ms=opts.target_ms,
                  unity=str(opts.unity), affinity_mask=mask,
                  jit_environment={k: os.environ[k] for k in ('DOTNET_TieredCompilation', 'DOTNET_ReadyToRun')},
                  source_hashes={str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in SOURCE.iterdir() if p.is_file()},
                  managed_sha256=hashlib.sha256((stage / 'AotBaseline.dll').read_bytes()).hexdigest())
    (out / 'results.json').write_text(json.dumps(result, indent=2))
    print(out / 'results.json', flush=True)


if __name__ == '__main__':
    main()
