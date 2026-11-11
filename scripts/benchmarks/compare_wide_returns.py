"""Compare legacy Result with real scalar-status/explicit-out AOT entry points."""
import argparse
import json
import random
import statistics
import subprocess
from pathlib import Path
from run_aot import build, invoke, sizes

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--emsdk', type=Path, required=True)
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/wide-returns')
    ap.add_argument('--build-only', action='store_true')
    ap.add_argument('--skip-build', action='store_true')
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=5000000)
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        ap.error('Require at least three samples and positive iterations')
    output = opts.output.resolve()
    output.relative_to(ROOT)
    output.mkdir(parents=True, exist_ok=True)
    if not opts.skip_build:
        for mode in ['result', 'status-out']:
            print('Building wide returns:', mode, flush=True)
            directory = output / mode
            directory.mkdir(parents=True, exist_ok=True)
            build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=2048,
                                     lightweight_returns=False, packed_int32_returns=False, status_out_returns=mode == 'status-out',
                                     interface_call_cache=False, interface_cache_entries=1,
                                     aot_rule_file=ROOT / 'src/benchmarks/aot-baseline/wide-returns.xml'), directory,
                  output / 'result/managed/AotBaseline.dll' if mode == 'status-out' else None)
    if opts.build_only:
        return
    import hashlib
    def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
    engines, artifacts = {}, {}
    for mode in ['result', 'status-out']:
        directory = output / mode
        report = json.loads((directory / 'generated/generic-aot-report.json').read_text())
        assert report['return_protocol_flags'] == (4 if mode == 'status-out' else 0)
        native, wasm = directory / 'native/Release/aot-baseline.exe', directory / 'wasm/aot-baseline.js'
        engines['native-' + mode] = [str(native), str(directory / 'managed')]
        engines['wasm-' + mode] = ['node', str(wasm), (directory / 'managed').relative_to(ROOT).as_posix()]
        artifacts['native-' + mode], artifacts['wasm-' + mode] = native, wasm.with_suffix('.wasm')
    assert sha(output / 'result/managed/AotBaseline.dll') == sha(output / 'status-out/managed/AotBaseline.dll')
    rows = []
    rng = random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, wall = invoke(engines[engine] + [str(opts.iterations), 'wide-returns'], output / f'{sample}-{engine}.jsonl')
            group = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(group) == 3
            rows.extend(dict(row, engine=engine, sample=sample, process_wall_seconds=wall) for row in group)
        print(f'Wide return sample {sample + 1}/{opts.samples}', flush=True)
    summary = []
    for workload in sorted({row['workload'] for row in rows}):
        assert len({row['checksum'] for row in rows if row['workload'] == workload}) == 1
        for engine in engines:
            values = [row['ns_per_op'] for row in rows if row['engine'] == engine and row['workload'] == workload]
            summary.append(dict(workload=workload, engine=engine, median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    result = dict(parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  node_version=subprocess.check_output(['node', '--version'], text=True).strip(),
                  samples=rows, summary=summary,
                  artifacts={name: dict(sha256=sha(path), **sizes(path)) for name, path in artifacts.items()},
                  managed_sha256=sha(output / 'result/managed/AotBaseline.dll'),
                  source_hashes={name: sha(ROOT / name) for name in subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others', '--exclude-standard', 'src', 'scripts'], cwd=ROOT, text=True).splitlines() if (ROOT / name).is_file()},
                  build_manifests={mode: json.loads((output / mode / 'build-manifest.json').read_text()) for mode in ['result', 'status-out']},
                  generated_hashes={path.relative_to(ROOT).as_posix(): sha(path) for mode in ['result', 'status-out'] for path in sorted((output / mode / 'generated').glob('*.cpp'))},
                  notes=['Same managed bytes, compiler settings and noinline leaves; independent long, double and small-struct workloads.',
                         'Explicit status/out entry points; logical adapter may inline but is not assumed free.',
                         'Timing after all builds, randomized process order, no frequency/background-load controls.',
                         'No assertion of IL2CPP parity or benefit for every struct size/native ABI.'])
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
