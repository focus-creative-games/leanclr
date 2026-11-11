"""Compare receiver-guided interface caches with disabled and uniform controls."""
import argparse
import hashlib
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
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/pgo-caches')
    ap.add_argument('--build-only', action='store_true')
    ap.add_argument('--skip-build', action='store_true')
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=500000)
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1: ap.error('Require at least 3 samples and positive iterations')
    output = opts.output.resolve()
    output.relative_to(ROOT)
    output.mkdir(parents=True, exist_ok=True)
    rules = output / 'pgo.xml'
    modes = ['disabled', 'single', 'double', 'guided']
    if not opts.skip_build:
        for mode in ['train'] + modes:
            print('Building PGO cache benchmark:', mode, flush=True)
            directory = output / mode
            directory.mkdir(parents=True, exist_ok=True)
            build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=2048, lightweight_returns=False,
                                     interface_call_cache=mode != 'disabled', interface_cache_entries=2 if mode in ['train', 'double'] else 1,
                                     enable_profile=mode == 'train', pgo_rule_file=rules if mode == 'guided' else None,
                                     profiled_interface_caches=mode == 'guided'), directory,
                  output / 'train/managed/AotBaseline.dll' if mode != 'train' else None)
            if mode == 'train':
                profile = output / 'train.json'
                invoke([str(directory / 'native/Release/aot-baseline.exe'), str(directory / 'managed'), '200000', 'interfaces2',
                        profile.relative_to(ROOT).as_posix()], output / 'train-invoke.log')
                records = json.loads(profile.read_text(encoding='utf-8'))
                profile.write_text(json.dumps([r for r in records if r['assembly'] == 'AotBaseline'], indent=2))
                invoke(['dotnet', 'build', ROOT / 'src/tools/pgo2aot/Pgo2Aot.csproj', '-c', 'Release'], output / 'converter.log')
                invoke(['dotnet', ROOT / 'out/dotnet/Pgo2Aot/Release/net8.0/pgo2aot.dll', 'convert', '-i', profile,
                        '--strategy', 'threshold', '--sort-by', 'calls', '-o', rules], output / 'convert.log')
    if opts.build_only: return
    def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
    assert len({sha(output / mode / 'managed/AotBaseline.dll') for mode in modes}) == 1
    engines, artifacts, reports = {}, {}, {}
    for mode in modes:
        directory = output / mode
        report = json.loads((directory / 'generated/generic-aot-report.json').read_text())
        assert report['return_protocol_flags'] == 0
        assert report['profiled_interface_caches'] == (mode == 'guided')
        reports[mode] = report
        native, wasm = directory / 'native/Release/aot-baseline.exe', directory / 'wasm/aot-baseline.js'
        engines['native-' + mode] = [str(native), str(directory / 'managed')]
        engines['wasm-' + mode] = ['node', str(wasm), (directory / 'managed').relative_to(ROOT).as_posix()]
        artifacts['native-' + mode], artifacts['wasm-' + mode] = native, wasm.with_suffix('.wasm')
    rows, rng = [], random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, wall = invoke(engines[engine] + [str(opts.iterations), 'interfaces2'], output / f'{sample}-{engine}.jsonl')
            group = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(group) == 7
            rows.extend(dict(row, sample=sample, engine=engine, process_wall_seconds=wall) for row in group)
        print(f'PGO cache sample {sample+1}/{opts.samples}', flush=True)
    summary = []
    for workload in sorted({r['workload'] for r in rows}):
        assert len({r['checksum'] for r in rows if r['workload'] == workload}) == 1
        for engine in engines:
            values = [r['ns_per_op'] for r in rows if r['workload'] == workload and r['engine'] == engine]
            summary.append(dict(workload=workload, engine=engine, median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    data = dict(parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                node_version=subprocess.check_output(['node', '--version'], text=True).strip(), samples=rows, summary=summary, reports=reports,
                training_profile=json.loads((output / 'train.json').read_text()), rules=rules.read_text(encoding='utf-8-sig'),
                artifacts={name: dict(sha256=sha(path), **sizes(path)) for name, path in artifacts.items()},
                managed_sha256=sha(output / 'train/managed/AotBaseline.dll'),
                manifests={m: json.loads((output / m / 'build-manifest.json').read_text()) for m in ['train'] + modes},
                generated_hashes={p.relative_to(ROOT).as_posix(): sha(p) for m in modes for p in (output / m / 'generated').glob('*.cpp')},
                notes=['Training is a separate profiling build; timed variants have profiling disabled and identical managed bytes.',
                       'Repeated representative training and evaluation patterns; not a held-out workload/generalization claim.',
                       'Each method retains at most twelve pointer words of frame-local cache storage; no runtime deoptimization.',
                       'Seven interleaved process samples after builds; uncontrolled CPU frequency/background load.',
                       'Invocation-scoped shadow reuse supports phase locality; recursion resets history conservatively.'])
    (output / 'results.json').write_text(json.dumps(data, indent=2)+'\n')


if __name__ == '__main__':
    main()
