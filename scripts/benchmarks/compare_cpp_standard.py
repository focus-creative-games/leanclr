"""Capture matched pre/post language-upgrade layouts, callback memory and timings."""
import argparse
import hashlib
import json
import random
import platform
import statistics
import subprocess
from pathlib import Path
from run_aot import build, invoke, sizes

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--stage', choices=['baseline', 'upgraded'], required=True)
    ap.add_argument('--emsdk', type=Path, required=True)
    ap.add_argument('--build-only', action='store_true')
    ap.add_argument('--skip-build', action='store_true')
    ap.add_argument('--samples', type=int, default=7)
    opts = ap.parse_args()
    if opts.samples < 3: ap.error('Require at least three samples')
    output = ROOT / 'out/performance/cpp-standard' / opts.stage
    output.mkdir(parents=True, exist_ok=True)
    if not opts.skip_build:
        for target in ['native', 'wasm']:
            print('Building language probe:', opts.stage, target, flush=True)
            directory = output / ('probe-' + target)
            command = ['cmake', '-S', ROOT / 'src/benchmarks/core-standard', '-B', directory, '-DCMAKE_BUILD_TYPE=Release']
            if target == 'native': command += ['-A', 'x64']
            else: command += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE', '-DCMAKE_CROSSCOMPILING_EMULATOR=node',
                               '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', '-DCMAKE_TOOLCHAIN_FILE=' + str(opts.emsdk.resolve() / 'upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake')]
            invoke(command, output / (target + '-configure.log'))
            invoke(['cmake', '--build', directory, '--config', 'Release', '--parallel', '4'], output / (target + '-build.log'))
            invoke(['ctest', '--test-dir', directory, '-C', 'Release', '--output-on-failure'], output / (target + '-test.log'))
        print('Building AOT language baseline:', opts.stage, flush=True)
        directory = output / 'aot'
        directory.mkdir(parents=True, exist_ok=True)
        build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=2048, lightweight_returns=False,
                                 interface_call_cache=False, interface_cache_entries=1), directory,
              ROOT / 'out/performance/cpp-standard/baseline/aot/managed/AotBaseline.dll' if opts.stage == 'upgraded' else None)
    if opts.build_only: return
    engines = {
        'native-probe': [str(output / 'probe-native/Release/core-standard.exe'), '1000000'],
        'wasm-probe': ['node', str(output / 'probe-wasm/core-standard.js'), '1000000'],
        'native-aot': [str(output / 'aot/native/Release/aot-baseline.exe'), str(output / 'aot/managed'), '500000', 'integer-returns'],
        'wasm-aot': ['node', str(output / 'aot/wasm/aot-baseline.js'), (output / 'aot/managed').relative_to(ROOT).as_posix(), '500000', 'integer-returns'],
    }
    rows, rng = [], random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, wall = invoke(engines[engine], output / f'{sample}-{engine}.jsonl')
            group = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(group) == (1 if engine.endswith('probe') else 12)
            rows.extend(dict(row, engine=engine, sample=sample, process_wall_seconds=wall) for row in group)
        print(f'Language {opts.stage} sample {sample+1}/{opts.samples}', flush=True)
    summaries = []
    for engine in engines:
        for workload in sorted({r.get('workload', 'string-view-map') for r in rows if r['engine'] == engine}):
            group = [r for r in rows if r['engine'] == engine and r.get('workload', 'string-view-map') == workload]
            assert len({r['checksum'] for r in group}) == 1
            field = 'lookup_ns' if engine.endswith('probe') else 'ns_per_op'
            summaries.append(dict(engine=engine, workload=workload, median_ns=statistics.median(r[field] for r in group), min_ns=min(r[field] for r in group), max_ns=max(r[field] for r in group)))
    def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
    artifacts = dict(native_probe=output / 'probe-native/Release/core-standard.exe', wasm_probe=output / 'probe-wasm/core-standard.wasm',
                     native_aot=output / 'aot/native/Release/aot-baseline.exe', wasm_aot=output / 'aot/wasm/aot-baseline.wasm')
    result = dict(stage=opts.stage, commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(), samples=rows, summary=summaries,
                  platform=platform.platform(), cpu=platform.processor(), node_version=subprocess.check_output(['node', '--version'], text=True).strip(),
                  source_hashes={str(p.relative_to(ROOT)):sha(p) for p in [ROOT / 'src/benchmarks/core-standard/main.cpp', ROOT / 'src/runtime/core/stl_compat.h', ROOT / 'src/runtime/build_config.h', ROOT / 'src/runtime/interp/transformer.h']},
                  managed_sha256=sha(output / 'aot/managed/AotBaseline.dll'), artifacts={k:dict(sha256=sha(v),**sizes(v)) for k,v in artifacts.items()},
                  manifest=json.loads((output / 'aot/build-manifest.json').read_text()),
                  probe_toolchains={p.relative_to(ROOT).as_posix():p.read_text() for p in output.glob('probe-*/CMakeFiles/*/CMakeCXXCompiler.cmake')},
                  notes=['Same compiler/toolchain, flags and managed bytes across stages; language/library compatibility is the intended change.',
                         'Probe memory counts requested HashMap payload bytes via host callbacks; excludes headers/CRT, input strings, RSS and other runtime heaps.',
                         'String-view inputs outlive the map. Optional moves are checked for ownership without assuming moved-from disengagement.',
                         'Seven process samples after builds; uncontrolled CPU frequency/background activity, not a universal performance claim.',
                         'AOT arithmetic/calls/arrays/interfaces/structs/generics/EH/returns use legacy protocol and disabled callsite caches.'])
    if opts.stage == 'upgraded':
        before = json.loads((ROOT / 'out/performance/cpp-standard/baseline/results.json').read_text())
        assert before['managed_sha256'] == result['managed_sha256'], 'Managed benchmark input changed'
        identity = lambda row: (row['engine'], row.get('workload', 'string-view-map'))
        checksums = {identity(row):row['checksum'] for row in before['samples']}
        assert all(checksums[identity(row)] == row['checksum'] for row in rows), 'Benchmark results changed'
        comparison = []
        for row in summaries:
            old = next(r for r in before['summary'] if identity(r) == identity(row))
            comparison.append(dict(engine=row['engine'], workload=row['workload'], before=old, after=row,
                                   median_delta_percent=(row['median_ns']/old['median_ns']-1)*100,
                                   ranges_overlap=max(old['min_ns'],row['min_ns']) <= min(old['max_ns'],row['max_ns'])))
        result['comparison'] = comparison
        for row in rows:
            if row['engine'].endswith('probe'): assert row['standard'] == 201703
    (output / 'results.json').write_text(json.dumps(result,indent=2)+'\n')


if __name__ == '__main__':
    main()
