"""Same generic EH IL: normal-path AOT versus whole-method interpretation."""
import argparse
import hashlib
import json
import platform
import random
import re
import statistics
import subprocess
from pathlib import Path
from run_aot import build
from run_protocols import ROOT, invoke, sizes


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/performance/generic-eh')
    parser.add_argument('--emsdk', type=Path, required=True)
    parser.add_argument('--samples', type=int, default=7)
    parser.add_argument('--iterations', type=int, default=5000000)
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--skip-build', action='store_true')
    opts = parser.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        parser.error('At least three samples and positive iterations required')
    output = opts.output.resolve()
    output.relative_to(ROOT)
    output.mkdir(parents=True, exist_ok=True)
    if not opts.skip_build:
        for mode in ['interpreted', 'aot']:
            directory = output / mode
            directory.mkdir(parents=True, exist_ok=True)
            print('Building generic EH:', mode, flush=True)
            build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=0 if mode == 'interpreted' else 2048,
                                     lightweight_returns=False, packed_int32_returns=False, interface_call_cache=False, interface_cache_entries=1,
                                     aot_rule_file=ROOT / 'src/benchmarks/aot-baseline/generic-eh.xml'), directory,
                  output / 'interpreted/managed/AotBaseline.dll' if mode == 'aot' else None)
    if opts.build_only:
        return
    engines, artifacts, reports = {}, {}, {}
    for mode in ['interpreted', 'aot']:
        directory = output / mode
        report = json.loads((directory / 'generated/generic-aot-report.json').read_text())
        assert len(report['instances']) == (0 if mode == 'interpreted' else 2), report['instances']
        assert len(report['generic_normal_path_methods']) == (0 if mode == 'interpreted' else 2)
        reports[mode] = report
        native = directory / 'native/Release/aot-baseline.exe'
        script = directory / 'wasm/aot-baseline.js'
        engines['native-' + mode] = [str(native), str(directory / 'managed')]
        engines['wasm-' + mode] = ['node', str(script), (directory / 'managed').relative_to(ROOT).as_posix()]
        artifacts['native-' + mode] = native
        artifacts['wasm-' + mode] = script.with_suffix('.wasm')
    rows = []
    rng = random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, elapsed = invoke(engines[engine] + [str(opts.iterations), 'generic-eh'], output / f'{sample}-{engine}.jsonl')
            group = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(group) == 2 and all(row['execution'] == ('aot+interpreter' if engine.endswith('interpreted') else 'aot-normal') for row in group)
            rows.extend(dict(row, engine=engine, sample=sample, process_wall_seconds=elapsed) for row in group)
        print(f'Generic EH sample {sample + 1}/{opts.samples}', flush=True)
    summary = []
    for workload in sorted({row['workload'] for row in rows}):
        assert len({row['checksum'] for row in rows if row['workload'] == workload}) == 1, workload
        for engine in engines:
            values = [row['ns_per_op'] for row in rows if row['engine'] == engine and row['workload'] == workload]
            summary.append(dict(engine=engine, workload=workload, median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    modified = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others', '--exclude-standard', 'src', 'scripts'], cwd=ROOT, text=True).splitlines()
    result = dict(schema=1, parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  platform=platform.platform(), cpu=platform.processor(), node_version=subprocess.check_output(['node', '--version'], text=True).strip(),
                  iterations=opts.iterations, samples=rows, summary=summary, reports=reports,
                  artifacts={name: dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(path), **sizes(path)) for name, path in artifacts.items()},
                  managed_inputs={mode: sha(output / mode / 'managed/AotBaseline.dll') for mode in ['interpreted', 'aot']},
                  source_hashes={name: sha(ROOT / name) for name in sorted(set(modified)) if (ROOT / name).is_file()},
                  build_manifests={mode: json.loads((output / mode / 'build-manifest.json').read_text()) for mode in ['interpreted', 'aot']},
                  generated_hashes={path.relative_to(ROOT).as_posix(): sha(path) for mode in ['interpreted', 'aot'] for path in sorted((output / mode / 'generated').glob('*.cpp'))},
                  toolchains={path.parent.parent.parent.name + '-' + mode: dict(re.findall(r'set\((CMAKE_CXX_COMPILER(?:_ID|_VERSION)?) "([^"\n]*)"\)', path.read_text()))
                              for mode in ['interpreted', 'aot'] for path in (output / mode).glob('*/CMakeFiles/*/CMakeCXXCompiler.cmake')},
                  notes=['Working-tree implementation based on parent_commit; byte-identical managed DLLs and matched compiler settings.',
                         'Selected wrappers remain AOT in both modes; only the two closed GenericEhLoop instances differ in execution backend.',
                         'No return-protocol or interface-cache trial enabled; noinline generic loop, success-only path, finally count checked each invocation.',
                         'The host checks that AOT normal paths never initialize interpreter IR.',
                         'Results compare native compilation/optimization with interpretation, not individual generic-call overhead or IL2CPP parity.',
                         'Seven interleaved process samples by default, after builds; no controlled frequency/background load.',
                         'Node/Wasm and Windows native; no browser/game or first-exception latency measurement. Sizes exclude external BCL/IL/resources.'])
    assert len(set(result['managed_inputs'].values())) == 1
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
