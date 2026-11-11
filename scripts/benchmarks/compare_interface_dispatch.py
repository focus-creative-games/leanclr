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
    parser.add_argument('--two-entry', action='store_true', help='Add two-entry caches and three-type/phase/many-site workloads')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--skip-build', action='store_true')
    opts = parser.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        parser.error('At least three samples and positive iterations required')
    output = opts.output.resolve()
    output.relative_to(ROOT)  # Node mounts paths relative to the repository.
    output.mkdir(parents=True, exist_ok=True)
    variants = ['baseline', 'cached'] + (['cached2'] if opts.two_entry else [])
    if not opts.skip_build:
        for variant in variants:
            cached = variant != 'baseline'
            directory = output / variant
            directory.mkdir(parents=True, exist_ok=True)
            print('Building:', directory, flush=True)
            build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=2048,
                                     lightweight_returns=opts.lightweight_returns, interface_call_cache=cached,
                                     interface_cache_entries=2 if variant == 'cached2' else 1), directory,
                  output / 'baseline/managed/AotBaseline.dll' if cached else None)
    if opts.build_only:
        return
    generated_policy = {}
    for variant in variants:
        methods = {}
        for source in (output / variant / 'generated').glob('*method*part*.cpp'):
            for section in source.read_text().split('// Method: ')[1:]:
                single = section.count('leanclr::codegen::InterfaceCallCache ')
                double = section.count('leanclr::codegen::InterfaceCallCache2 ')
                assert single * 3 + double * 6 <= 12
                methods[section.splitlines()[0]] = (single, double)
        many = [value for name, value in methods.items() if 'Benchmarks::InterfacesManySites(' in name]
        once = [value for name, value in methods.items() if 'Benchmarks::InterfaceOnce(' in name]
        expected = (0, 2) if variant == 'cached2' else (4, 0) if variant == 'cached' else (0, 0)
        if opts.two_entry:
            assert many == [expected] and once == [(0, 0)], (variant, many, once)
        generated_policy[variant] = dict(single_sites=sum(v[0] for v in methods.values()), double_sites=sum(v[1] for v in methods.values()),
                                        max_words_per_method=max((v[0] * 3 + v[1] * 6 for v in methods.values()), default=0))
    engines, binaries = {}, {}
    for variant in variants:
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
            stdout, elapsed = invoke(engines[engine] + [str(opts.iterations), 'interfaces2' if opts.two_entry else 'interfaces'], output / f'{sample}-{engine}.jsonl')
            records = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(records) == (7 if opts.two_entry else 4) and all(r['execution'] == 'aot' for r in records), engine
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
                  iterations=opts.iterations, lightweight_returns=opts.lightweight_returns, two_entry=opts.two_entry,
                  generated_policy=generated_policy,
                  parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  artifacts={name: dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(path), **sizes(path)) for name, path in binaries.items()},
                  managed_inputs={variant: sha(output / variant / 'managed/AotBaseline.dll') for variant in variants},
                  build_manifests={variant: json.loads((output / variant / 'build-manifest.json').read_text()) for variant in variants},
                  toolchains={path.parent.parent.parent.name + '-' + path.parent.parent.parent.parent.name:
                              dict(re.findall(r'set\((CMAKE_CXX_COMPILER(?:_ID|_VERSION)?) "([^"\n]*)"\)', path.read_text()))
                              for variant in variants for path in (output / variant).glob('*/CMakeFiles/*/CMakeCXXCompiler.cmake')},
                  node_version=subprocess.check_output(['node', '--version'], text=True).strip() if opts.emsdk else None,
                  source_hashes={name: sha(ROOT / name) for name in sorted(source_paths) if (ROOT / name).is_file()},
                  generated_hashes={path.relative_to(ROOT).as_posix(): sha(path) for variant in variants for path in sorted((output / variant / 'generated').glob('*.cpp'))},
                  notes=['Working-tree implementation based on parent_commit; only cache enable/capacity settings differ between configurations.',
                         'Same managed source and compiler options; metadata is warm, each AOT invocation starts with empty stack caches.',
                         'Monomorphic, alternating-class, generic-method and straight-line control; --two-entry adds three-class, phase-change and five-site workloads.',
                         'All four ordinary targets, the generic override and workload entries are verified AOT before timing.',
                         'Up to four three-word single-entry sites or two six-word double-entry sites per invocation; no static cache, heap map or new GC roots.',
                         'Seven interleaved process samples by default; frequency/background activity is not controlled.',
                         'No IL2CPP, browser, game-wide or cold-start performance claim; artifact sizes exclude external BCL/metadata/resources.'])
    assert len(set(result['managed_inputs'].values())) == 1, 'Managed inputs differ'
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
