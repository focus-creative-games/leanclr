"""Measure already-built generic/EH AOT fixtures against same-runtime controls.

Build the feature fixtures with scripts/test/aot-features.py first. The baseline
build needs generated and zero-generic-instance versions in native/wasm and
disabled-native/disabled-wasm respectively. This runner never rebuilds binaries
or changes historical P0/P1 results. Run without competing build processes.
"""
import argparse
import hashlib
import json
import platform
import random
import statistics
import subprocess
from pathlib import Path

from run_protocols import ROOT, invoke, sizes


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline-build', type=Path, default=ROOT / 'out/performance/generic-step')
    ap.add_argument('--features-build', type=Path, default=ROOT / 'out/aot-features')
    ap.add_argument('--control-build', type=Path, help='Separate run_aot.py output built with --max-generic-instances 0')
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/aot-features-review')
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=5000000)
    ap.add_argument('--wasm', action='store_true')
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        ap.error('Use at least three samples and positive iterations')
    output = opts.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    baseline = opts.baseline_build.resolve()
    features = opts.features_build.resolve()
    engines = {}
    artifacts = {}
    for target in ['native'] + (['wasm'] if opts.wasm else []):
        for enabled in [True, False]:
            name = target + ('-generic-aot' if enabled else '-generic-control')
            build = baseline / target if enabled else (
                opts.control_build.resolve() / target if opts.control_build else baseline / ('disabled-' + target))
            binary = build / ('Release/aot-baseline.exe' if target == 'native' else 'aot-baseline.js')
            stage = baseline / 'managed'
            engines[name] = ([str(binary), str(stage)] if target == 'native' else
                             ['node', str(binary), stage.relative_to(ROOT).as_posix()])
            artifacts[name] = binary if target == 'native' else binary.with_suffix('.wasm')
        for variant in ['default', 'interpreted-loop']:
            name = target + '-eh-' + variant
            build = features / 'eh' / f'{variant}-{target}-Release-x64'
            binary = build / ('Release/eh-aot.exe' if target == 'native' else 'eh-aot.js')
            stage = features / 'eh/managed'
            engines[name] = ([str(binary), str(stage)] if target == 'native' else
                             ['node', str(binary), stage.relative_to(ROOT).as_posix()])
            artifacts[name] = binary if target == 'native' else binary.with_suffix('.wasm')
    rows = []
    rng = random.Random(20261110)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            text, elapsed = invoke(engines[engine] + [str(opts.iterations)], output / f'{sample}-{engine}.jsonl')
            records = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
            if len(records) != (1 if '-eh-' in engine else 8):
                raise RuntimeError(f'Incomplete output: {engine}')
            for row in records:
                row.update(engine=engine, sample=sample, process_wall_seconds=elapsed)
                rows.append(row)
        print(f'Feature comparison sample {sample + 1}/{opts.samples}', flush=True)
    for workload in {r['workload'] for r in rows}:
        if len({r['checksum'] for r in rows if r['workload'] == workload}) != 1:
            raise RuntimeError(f'Checksum mismatch: {workload}')
    summary = []
    for engine in engines:
        for workload in sorted({r['workload'] for r in rows if r['engine'] == engine}):
            group = [r for r in rows if r['engine'] == engine and r['workload'] == workload]
            values = [r['ns_per_op'] for r in group]
            summary.append(dict(engine=engine, workload=workload, execution=group[0]['execution'],
                                median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    result = dict(schema=1, platform=platform.platform(), cpu=platform.processor(),
                  commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  iterations=opts.iterations, samples=rows, summary=summary,
                  artifacts={k: dict(path=str(p.relative_to(ROOT)), sha256=hashlib.sha256(p.read_bytes()).hexdigest(),
                                     **sizes(p)) for k, p in artifacts.items()},
                  source_hashes={str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                 for p in [ROOT / 'src/benchmarks/aot-baseline/Benchmarks.cs',
                                           ROOT / 'src/tests/eh-aot/Fixtures.cs',
                                           ROOT / 'src/benchmarks/aot-baseline/main.cpp',
                                           ROOT / 'src/tests/eh-aot/main.cpp']},
                  notes=['Working-tree implementation; the commit above is its parent revision.',
                         'Generic control uses the same runtime with the generic instance budget set to zero.',
                         'EH control excludes only NormalLoop using interpreted.xml, on the same runtime.',
                         'Original exceptions workload throws at iteration zero; the current invocation then stays interpreted.',
                         'Sizes include the entire test executable/runtime, excluding external BCL/IL resources.',
                         'Process wall time includes warmup, verification and output; it is not startup time.',
                         'These microbenchmarks are not end-to-end application or IL2CPP comparisons.'])
    modified = subprocess.check_output(
        ['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others', '--exclude-standard',
         'src/leanaot', 'src/runtime'], cwd=ROOT, text=True).splitlines()
    result['implementation_source_hashes'] = {
        name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
        for name in sorted(set(modified)) if Path(name).suffix in ['.cs', '.cpp', '.h']}
    (output / 'results.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
