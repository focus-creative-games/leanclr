"""Measure packed int32 AOT returns against byte-identical managed input."""
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
    parser.add_argument('--output', type=Path, default=ROOT / 'out/performance/int32-returns')
    parser.add_argument('--emsdk', type=Path, required=True)
    parser.add_argument('--samples', type=int, default=7)
    parser.add_argument('--iterations', type=int, default=5000000)
    parser.add_argument('--lightweight-returns', action='store_true')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--skip-build', action='store_true')
    parser.add_argument('--abi-only', action='store_true')
    opts = parser.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        parser.error('At least three samples and positive iterations required')
    output = opts.output.resolve()
    output.relative_to(ROOT)
    output.mkdir(parents=True, exist_ok=True)
    if not opts.skip_build and not opts.abi_only:
        for variant in ['baseline', 'packed']:
            directory = output / variant
            directory.mkdir(parents=True, exist_ok=True)
            print('Building:', variant, flush=True)
            build(argparse.Namespace(emsdk=opts.emsdk, unity=None, max_generic_instances=2048,
                                     lightweight_returns=opts.lightweight_returns, packed_int32_returns=variant == 'packed',
                                     interface_call_cache=False, interface_cache_entries=1), directory,
                  output / 'baseline/managed/AotBaseline.dll' if variant == 'packed' else None)
    if opts.build_only:
        return
    for variant in ['baseline', 'packed']:
        declarations = {}
        for source in (output / variant / 'generated').glob('*method*part*.cpp'):
            for section in source.read_text().split('// Method: ')[1:]:
                for kind in ['Int32', 'UInt32']:
                    for level in ['Leaf', 'Middle', 'Top']:
                        name = kind + 'Return' + level
                        if 'Benchmarks::' + name + '(' in section.splitlines()[0]:
                            declarations[name] = section.splitlines()[1]
        if not opts.abi_only:
            assert len(declarations) == 6 and all('LEANCLR_CODEGEN_NOINLINE' in line for line in declarations.values())
            for name, line in declarations.items():
                assert ('AotInt32Result<' in line) == (variant == 'packed'), (variant, name)
    abi = {}
    for config in ['Release', 'Debug']:
        ir = output / f'abi-{config}.ll'
        invoke([opts.emsdk.resolve() / 'upstream/emscripten/em++.bat', '-std=c++17', '-O2', '-S', '-emit-llvm',
                '-Wno-return-type-c-linkage', '-DNDEBUG' if config == 'Release' else '-DDEBUG', '-I' + str(ROOT / 'src/runtime'),
                ROOT / 'src/tests/aot-returns/main.cpp', '-o', ir], output / f'abi-{config}.log')
        lines = [line for line in ir.read_text().splitlines() if re.match(r'define .*@(aot_int32_probe|aot_uint32_probe|ordinary_int32_probe)\(', line)]
        assert len(lines) == 3
        for line in lines:
            if '@ordinary_' in line:
                assert 'sret(' in line, line
            else:
                assert re.match(r'define .*\bi64 @aot_', line) and 'sret(' not in line, line
        abi[config] = dict(signatures=lines, sha256=sha(ir))
    if opts.abi_only:
        print('Wasm Release/Debug integer ABI probes passed', flush=True)
        return
    engines, artifacts = {}, {}
    for variant in ['baseline', 'packed']:
        directory = output / variant
        native = directory / 'native/Release/aot-baseline.exe'
        wasm = directory / 'wasm/aot-baseline.js'
        engines['native-' + variant] = [str(native), str(directory / 'managed')]
        engines['wasm-' + variant] = ['node', str(wasm), (directory / 'managed').relative_to(ROOT).as_posix()]
        artifacts['native-' + variant] = native
        artifacts['wasm-' + variant] = wasm.with_suffix('.wasm')
    rows = []
    rng = random.Random(20261111)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            stdout, elapsed = invoke(engines[engine] + [str(opts.iterations), 'integer-returns'], output / f'{sample}-{engine}.jsonl')
            group = [json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
            assert len(group) == 12
            assert all(row['execution'] == ('aot-normal/interpreter-eh' if row['workload'] == 'exceptions' else 'aot') for row in group)
            rows.extend(dict(row, engine=engine, sample=sample, process_wall_seconds=elapsed) for row in group)
        print(f'Integer return sample {sample + 1}/{opts.samples}', flush=True)
    summary = []
    for workload in sorted({row['workload'] for row in rows}):
        assert len({row['checksum'] for row in rows if row['workload'] == workload}) == 1, workload
        for engine in engines:
            values = [row['ns_per_op'] for row in rows if row['engine'] == engine and row['workload'] == workload]
            summary.append(dict(engine=engine, workload=workload, median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    sources = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'ls-files', '--modified', '--others', '--exclude-standard', 'src', 'scripts'], cwd=ROOT, text=True).splitlines()
    result = dict(schema=1, parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  platform=platform.platform(), cpu=platform.processor(), node_version=subprocess.check_output(['node', '--version'], text=True).strip(),
                  iterations=opts.iterations, lightweight_returns=opts.lightweight_returns, samples=rows, summary=summary, wasm_abi=abi,
                  managed_inputs={mode: sha(output / mode / 'managed/AotBaseline.dll') for mode in ['baseline', 'packed']},
                  artifacts={name: dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(path), **sizes(path)) for name, path in artifacts.items()},
                  source_hashes={name: sha(ROOT / name) for name in sorted(set(sources)) if (ROOT / name).is_file()},
                  build_manifests={mode: json.loads((output / mode / 'build-manifest.json').read_text()) for mode in ['baseline', 'packed']},
                  generated_hashes={path.relative_to(ROOT).as_posix(): sha(path) for mode in ['baseline', 'packed'] for path in sorted((output / mode / 'generated').glob('*.cpp'))},
                  toolchains={path.parent.parent.parent.name + '-' + mode: dict(re.findall(r'set\((CMAKE_CXX_COMPILER(?:_ID|_VERSION)?) "([^"\n]*)"\)', path.read_text()))
                              for mode in ['baseline', 'packed'] for path in (output / mode).glob('*/CMakeFiles/*/CMakeCXXCompiler.cmake')},
                  notes=['Working-tree implementation based on parent_commit; only packed int32 enable state differs.',
                         'Byte-identical managed DLLs; interface caches disabled; default Result for void/references unless explicitly enabled on both sides.',
                         'New signed/unsigned chains retain observable work after each noinline leaf/middle call.',
                         'Original call/generic workloads may benefit from tail calls/inlining; do not attribute all gains to memory returns.',
                         'EH workload switches to the interpreter after its first exception; no claim of native exception acceleration.',
                         'Seven interleaved process samples by default, after all compilation; frequency/background activity not controlled.',
                         'Node/Wasm and Windows native, not IL2CPP/browser/game-wide timing; artifact sizes exclude external BCL/IL/resources.'])
    assert len(set(result['managed_inputs'].values())) == 1
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
