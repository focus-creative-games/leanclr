"""Compare the same AOT interface loops with/without invocation-local caches."""
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
    parser.add_argument('--output', type=Path, default=ROOT / 'out/performance/interface-dispatch')
    parser.add_argument('--emsdk', type=Path)
    parser.add_argument('--samples', type=int, default=7)
    parser.add_argument('--iterations', type=int, default=5000000)
    parser.add_argument('--lightweight-returns', action='store_true')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--skip-build', action='store_true')
    opts = parser.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        parser.error('At least three samples and positive iterations required')
    output = opts.output.resolve()
    output.relative_to(ROOT)  # Node mounts paths relative to the repository.
    output.mkdir(parents=True, exist_ok=True)
    if not opts.skip_build:
        for cached in [False, True]:
            directory = output / ('cached' if cached else 'baseline')
            directory.mkdir(parents=True, exist_ok=True)
            print('Building:', directory, flush=True)
            build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=2048,
                                     lightweight_returns=opts.lightweight_returns, interface_call_cache=cached), directory,
                  output / 'baseline/managed/AotBaseline.dll' if cached else None)
    if opts.build_only:
        return
    engines, binaries = {}, {}
    for variant in ['baseline', 'cached']:
        directory = output / variant
        native = directory / 'native/Release/aot-baseline.exe'
        engines['native-' + variant] = [str(native), str(directory / 'managed')]
        binaries['native-' + variant] = native
        if opts.emsdk:
            script = directory / 'wasm/aot-baseline.js'
            engines['wasm-' + variant] = ['node', str(script), (directory / 'managed').relative_to(ROOT).as_posix()]
            binaries['wasm-' + variant] = script.with_suffix('.wasm')
    rows = []
    rng = random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, elapsed = invoke(engines[engine] + [str(opts.iterations), 'interfaces'], output / f'{sample}-{engine}.jsonl')
            records = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(records) == 4 and all(r['execution'] == 'aot' for r in records), engine
            rows.extend(dict(row, engine=engine, sample=sample, process_wall_seconds=elapsed) for row in records)
        print(f'Interface dispatch sample {sample + 1}/{opts.samples}', flush=True)
    summary = []
    for workload in sorted({row['workload'] for row in rows}):
        assert len({row['checksum'] for row in rows if row['workload'] == workload}) == 1, workload
        for engine in engines:
            values = [row['ns_per_op'] for row in rows if row['workload'] == workload and row['engine'] == engine]
            summary.append(dict(engine=engine, workload=workload, median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    modified = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others', '--exclude-standard', 'src', 'scripts'], cwd=ROOT, text=True).splitlines()
    source_paths = set(modified) | {'src/leanaot/LeanAOT.ToCpp/ReturnProtocol.cs', 'src/runtime/vm/method.cpp'}
    result = dict(schema=1, platform=platform.platform(), cpu=platform.processor(), samples=rows, summary=summary,
                  iterations=opts.iterations, lightweight_returns=opts.lightweight_returns,
                  parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  artifacts={name: dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(path), **sizes(path)) for name, path in binaries.items()},
                  managed_inputs={variant: sha(output / variant / 'managed/AotBaseline.dll') for variant in ['baseline', 'cached']},
                  build_manifests={variant: json.loads((output / variant / 'build-manifest.json').read_text()) for variant in ['baseline', 'cached']},
                  toolchains={path.parent.parent.parent.name + '-' + path.parent.parent.parent.parent.name:
                              dict(re.findall(r'set\((CMAKE_CXX_COMPILER(?:_ID|_VERSION)?) "([^"\n]*)"\)', path.read_text()))
                              for variant in ['baseline', 'cached'] for path in (output / variant).glob('*/CMakeFiles/*/CMakeCXXCompiler.cmake')},
                  node_version=subprocess.check_output(['node', '--version'], text=True).strip() if opts.emsdk else None,
                  source_hashes={name: sha(ROOT / name) for name in sorted(source_paths) if (ROOT / name).is_file()},
                  generated_hashes={path.relative_to(ROOT).as_posix(): sha(path) for variant in ['baseline', 'cached'] for path in sorted((output / variant / 'generated').glob('*.cpp'))},
                  notes=['Working-tree implementation based on parent_commit; only the cache flag differs between configurations.',
                         'Same managed source and compiler options; metadata is warm, each AOT invocation starts with empty stack caches.',
                         'Monomorphic, alternating-class, generic-method and straight-line control workloads; actual generic override is verified AOT.',
                         'Up to four three-pointer entries per generated method invocation; no static cache, heap map or new GC roots.',
                         'Seven interleaved process samples by default; frequency/background activity is not controlled.',
                         'No IL2CPP, browser, game-wide or cold-start performance claim; artifact sizes exclude external BCL/metadata/resources.'])
    assert len(set(result['managed_inputs'].values())) == 1, 'Managed inputs differ'
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
