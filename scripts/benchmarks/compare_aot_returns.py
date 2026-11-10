"""Interleave same-source AOT Result/lightweight samples and capture Wasm ABI evidence."""
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


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build-root', type=Path, default=ROOT / 'out/performance/aot-returns')
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=5000000)
    ap.add_argument('--emsdk', type=Path)
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        ap.error('At least three samples and positive iterations required')
    root = opts.build_root.resolve()
    output = root / 'comparison'
    output.mkdir(parents=True, exist_ok=True)
    engines, artifacts = {}, {}
    for target in ['native'] + (['wasm'] if opts.emsdk else []):
        for mode in ['result', 'lightweight']:
            build = root / mode
            binary = build / ('native/Release/aot-baseline.exe' if target == 'native' else 'wasm/aot-baseline.js')
            stage = build / 'managed'
            name = target + '-' + mode
            engines[name] = ([str(binary), str(stage)] if target == 'native' else
                             ['node', str(binary), stage.relative_to(ROOT).as_posix()])
            artifacts[name] = binary if target == 'native' else binary.with_suffix('.wasm')
    abi = {}
    if opts.emsdk:
        compiler = opts.emsdk.resolve() / 'upstream/emscripten/em++.bat'
        for mode in ['Release', 'Debug']:
            ir = output / f'abi-{mode}.ll'
            invoke([compiler, '-std=c++11', '-O2', '-S', '-emit-llvm', '-Wno-return-type-c-linkage',
                    '-DNDEBUG' if mode == 'Release' else '-DDEBUG', '-I' + str(ROOT / 'src/runtime'),
                    ROOT / 'src/tests/aot-returns/main.cpp', '-o', ir], output / f'abi-{mode}.log')
            lines = [line for line in ir.read_text().splitlines() if re.match(
                r'define .*@(aot_pointer_probe|aot_void_probe|ordinary_pointer_probe|ordinary_void_probe)\(', line)]
            if len(lines) != 4:
                raise RuntimeError('Incomplete ABI probes')
            for line in lines:
                assert ('sret(' in line) == ('@ordinary_' in line), line
            abi[mode] = dict(signatures=lines, ir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest())
    rows = []
    rng = random.Random(20261110)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            text, elapsed = invoke(engines[engine] + [str(opts.iterations), 'returns'], output / f'{sample}-{engine}.jsonl')
            group = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
            if len(group) != 10:
                raise RuntimeError(f'Incomplete benchmark output: {engine}')
            rows.extend(dict(row, engine=engine, sample=sample, process_wall_seconds=elapsed) for row in group)
        print(f'Return ABI comparison {sample + 1}/{opts.samples}', flush=True)
    for workload in {r['workload'] for r in rows}:
        assert len({r['checksum'] for r in rows if r['workload'] == workload}) == 1, workload
    summary = []
    for engine in engines:
        for workload in sorted({r['workload'] for r in rows}):
            values = [r['ns_per_op'] for r in rows if r['engine'] == engine and r['workload'] == workload]
            summary.append(dict(engine=engine, workload=workload, median_ns=statistics.median(values),
                                min_ns=min(values), max_ns=max(values)))
    modified = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others',
                                       '--exclude-standard', 'src', 'scripts'], cwd=ROOT, text=True).splitlines()
    result = dict(schema=1, platform=platform.platform(), cpu=platform.processor(), iterations=opts.iterations,
                  parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  samples=rows, summary=summary, wasm_abi=abi,
                  artifacts={name: dict(path=str(p.relative_to(ROOT)), sha256=hashlib.sha256(p.read_bytes()).hexdigest(),
                                        **sizes(p)) for name, p in artifacts.items()},
                  managed_inputs={mode: {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                         for p in sorted((root / mode / 'managed').glob('*.dll'))}
                                  for mode in ['result', 'lightweight']},
                  implementation_source_hashes={name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
                                                for name in sorted(set(modified)) if (ROOT / name).is_file()},
                  notes=['Working-tree implementation based on parent_commit.',
                         'Same Benchmarks.cs and runtime, differing only in generated return protocol.',
                         'Both configurations now honor managed NoInlining; do not compare their call/generic timings directly with historical P0.',
                         'Microbenchmark call chains retain work after the leaf call; not end-to-end application performance.',
                         'Wasm runs in Node; no IL2CPP or browser timing in this experiment.',
                         'Sizes exclude external BCL, IL and resources; process wall time is not startup latency.'])
    (output / 'results.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
