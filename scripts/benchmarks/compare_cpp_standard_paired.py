"""Interleave retained pre/post C++17 binaries after all builds have finished."""
import argparse
import hashlib
import json
import random
import statistics
from pathlib import Path
from run_aot import invoke, sizes

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=5000000)
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1: ap.error('Require at least three samples and positive iterations')
    base = ROOT / 'out/performance/cpp-standard'
    output = base / 'paired'
    output.mkdir(exist_ok=True)
    engines, artifacts, manifests = {}, {}, {}
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    for stage in ['baseline', 'upgraded']:
        directory = base / stage
        manifests[stage] = json.loads((directory / 'results.json').read_text())
        files = dict(native_probe=directory / 'probe-native/Release/core-standard.exe', wasm_probe=directory / 'probe-wasm/core-standard.wasm',
                     native_aot=directory / 'aot/native/Release/aot-baseline.exe', wasm_aot=directory / 'aot/wasm/aot-baseline.wasm')
        artifacts[stage] = {key:dict(sha256=sha(path), **sizes(path)) for key,path in files.items()}
        assert all(artifacts[stage][key]['sha256'] == manifests[stage]['artifacts'][key]['sha256'] for key in files), 'Retained binary changed'
        engines[stage] = {
            'native-probe': [files['native_probe'], '1000000'],
            'wasm-probe': ['node', files['wasm_probe'].with_suffix('.js'), '1000000'],
            'native-aot': [files['native_aot'], directory / 'aot/managed', str(opts.iterations), 'integer-returns'],
            'wasm-aot': ['node', files['wasm_aot'].with_suffix('.js'), (directory / 'aot/managed').relative_to(ROOT).as_posix(), str(opts.iterations), 'integer-returns'],
        }
    assert manifests['baseline']['managed_sha256'] == manifests['upgraded']['managed_sha256']
    rows, rng = [], random.Random(20261111)
    for sample in range(opts.samples):
        order = [(stage,engine) for stage in engines for engine in engines[stage]]
        rng.shuffle(order)
        for stage,engine in order:
            stdout, wall = invoke(engines[stage][engine], output / f'{sample}-{stage}-{engine}.jsonl')
            group = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(group) == (1 if engine.endswith('probe') else 12)
            rows.extend(dict(row, stage=stage, engine=engine, sample=sample, process_wall_seconds=wall) for row in group)
        print(f'Paired standard sample {sample+1}/{opts.samples}', flush=True)
    comparison = []
    for engine in engines['baseline']:
        for workload in sorted({r.get('workload', 'string-view-map') for r in rows if r['engine'] == engine}):
            group = [r for r in rows if r['engine'] == engine and r.get('workload', 'string-view-map') == workload]
            assert len({r['checksum'] for r in group}) == 1
            field = 'lookup_ns' if engine.endswith('probe') else 'ns_per_op'
            def summary(stage):
                values = [r[field] for r in group if r['stage'] == stage]
                return dict(median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values))
            before, after = summary('baseline'), summary('upgraded')
            comparison.append(dict(engine=engine, workload=workload, before=before, after=after,
                                   median_delta_percent=(after['median_ns']/before['median_ns']-1)*100,
                                   ranges_overlap=max(before['min_ns'],after['min_ns']) <= min(before['max_ns'],after['max_ns'])))
    result = dict(samples=rows, comparison=comparison, artifacts=artifacts, iterations=opts.iterations,
                  managed_sha256=manifests['baseline']['managed_sha256'], source_commits={k:v['commit'] for k,v in manifests.items()},
                  notes=['No builds during sampling; deterministic randomized process order interleaves retained old/new binaries.',
                         'AOT loops use ten times the initial iteration count; probe still uses one million lookups.',
                         'No CPU pinning/frequency control; sample ranges are observations, not confidence intervals.'])
    (output / 'results.json').write_text(json.dumps(result,indent=2)+'\n')


if __name__ == '__main__':
    main()
