"""Measure global generic lookup scaling after all builds have finished.

The linear counterfactual scans the SAME synthetic entry layout as the index;
it is an algorithm comparison, not an old-binary or cold-start comparison.
"""
import argparse
import hashlib
import json
import platform
import random
import re
import statistics
import subprocess
from pathlib import Path
from run_protocols import ROOT, invoke, sizes


def identity_pool(source):
    arrays = {}
    roots = []
    for line in source.splitlines():
        array = re.search(r'static const .* (s_generic_identity_\d+)\[\] = \{ (.*) \};', line)
        if array:
            arrays[array[1]] = re.findall(r', (s_generic_identity_\d+|nullptr), \d+ \}', array[2])
        entry = re.search(r'\}, (s_generic_identity_\d+|nullptr), (s_generic_identity_\d+|nullptr), \d+, \d+, [01] \},?$', line)
        if entry:
            roots += [entry[1], entry[2]]
    weights = {'nullptr': 0}

    def expanded(name):
        if name not in weights:
            weights[name] = len(arrays[name]) + sum(expanded(child) for child in arrays[name])
        return weights[name]

    return dict(interned_arrays=len(arrays), interned_descriptors=sum(len(x) for x in arrays.values()),
                without_interning_descriptors=sum(expanded(root) for root in roots), instances=len(roots) // 2)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build-root', type=Path, default=ROOT / 'out/global-generics')
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/global-generics')
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=20000)
    ap.add_argument('--wasm', action='store_true')
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        ap.error('At least three samples and positive iterations required')
    root, output = opts.build_root.resolve(), opts.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    stage = root / 'managed'
    native = root / 'result/default-native-Release-x64/Release/generic-registry.exe'
    engines = {'native': [str(native), str(stage)]}
    binaries = {'native': native}
    if opts.wasm:
        script = root / 'result/default-wasm-Release-x64/generic-registry.js'
        engines['wasm'] = ['node', str(script), stage.relative_to(ROOT).as_posix()]
        binaries['wasm'] = script.with_suffix('.wasm')
    rows, layouts = [], {}
    rng = random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, elapsed = invoke(engines[engine] + ['benchmark', str(opts.iterations)], output / f'{sample}-{engine}.jsonl')
            records = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(records) == 12, engine
            layout = re.search(r'Global generic registry: (\d+) entries; entry size (\d+); method size (\d+); identity size (\d+); module size (\d+); global size (\d+); pointer size (\d+)', stdout)
            assert layout, engine
            layouts[engine] = dict(zip(['instances', 'entry_bytes', 'method_info_bytes', 'identity_bytes', 'module_bytes', 'global_bytes', 'pointer_bytes'], map(int, layout.groups())))
            for record in records:
                assert record['checksum'] == (opts.iterations if record['workload'] == 'lookup-hit' else 0)
                rows.append(dict(record, engine=engine, sample=sample, process_wall_seconds=elapsed))
        print(f'Global generic lookup sample {sample + 1}/{opts.samples}', flush=True)
    summary = []
    for engine in engines:
        for count in [64, 1024, 16384]:
            for workload in ['lookup-hit', 'lookup-miss']:
                for index in ['linear', 'sorted-hash']:
                    values = [r['ns_per_lookup'] for r in rows if r['engine'] == engine and r['entries'] == count and r['workload'] == workload and r['index'] == index]
                    summary.append(dict(engine=engine, entries=count, workload=workload, index=index,
                                        median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    registration = root / 'result/default/generic_registration.cpp'
    pool = identity_pool(registration.read_text())
    for layout in layouts.values():
        assert layout['instances'] == pool['instances']
        # Counterfactual storage for the same instance set, excluding strings,
        # relocations, code, and module/global container differences.
        legacy_entry = 64 if layout['pointer_bytes'] == 8 else 32
        layout['entry_and_identity_bytes'] = layout['entry_bytes'] * pool['instances'] + layout['identity_bytes'] * pool['interned_descriptors']
        layout['without_global_interning_bytes'] = legacy_entry * pool['instances'] + layout['identity_bytes'] * pool['without_interning_descriptors']
    modified = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others', '--exclude-standard', 'src', 'scripts'], cwd=ROOT, text=True).splitlines()
    result = dict(schema=1, platform=platform.platform(), cpu=platform.processor(), iterations=opts.iterations,
                  parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  samples=rows, summary=summary, layouts=layouts, identity_pool=pool,
                  artifacts={name: dict(path=str(p.relative_to(ROOT)), sha256=hashlib.sha256(p.read_bytes()).hexdigest(), **sizes(p)) for name, p in binaries.items()},
                  registration_sha256=hashlib.sha256(registration.read_bytes()).hexdigest(),
                  source_hashes={name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in sorted(set(modified)) if (ROOT / name).is_file()},
                  notes=['Working-tree implementation based on parent_commit.',
                         'Synthetic tables have a hit in the middle, and use the same new entry layout for both algorithms.',
                         'Lookup metadata and argument modules are already loaded; this is not cold generic creation or startup.',
                         'Binding invokes the index once; ordinary pooled method caching retains both AOT and interpreter results.',
                         'The new index itself uses static storage and allocates no generic-index heap map; normal module/method caches still allocate.',
                         'Nominal metadata counts exclude strings, relocations, code, module containers, and linker folding.',
                         'No application-wide or IL2CPP performance claim; CPU frequency/background activity is not controlled.'])
    (output / 'results.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
