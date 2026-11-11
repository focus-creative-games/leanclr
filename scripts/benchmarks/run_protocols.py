"""Build, verify and measure the isolated P0/P1 return-protocol experiment."""
import argparse
import gzip
import hashlib
import json
import os
import platform
import random
import re
import statistics
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROTOCOLS = ['current', 'trivial', 'scalar', 'status', 'context', 'eh', 'direct']


def invoke(args, log, cwd=ROOT):
    start = time.perf_counter()
    p = subprocess.run([str(x) for x in args], cwd=cwd, text=True,
                       encoding='utf-8', errors='replace', stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
    log.write_text(p.stdout, encoding='utf-8')
    if p.returncode:
        raise RuntimeError(f'{args[0]} failed ({p.returncode}); see {log}\n{p.stdout[-3000:]}')
    return p.stdout, time.perf_counter() - start


def leb(data, offset):
    value = shift = 0
    while True:
        byte = data[offset]
        offset += 1
        value |= (byte & 127) << shift
        if byte < 128:
            return value, offset
        shift += 7


def sizes(path):
    data = path.read_bytes()
    result = {'bytes': len(data), 'gzip_bytes': len(gzip.compress(data, mtime=0))}
    if data[:8] == b'\0asm\1\0\0\0':
        offset = 8
        while offset < len(data):
            section = data[offset]
            length, start = leb(data, offset + 1)
            if section == 10:
                result['code_section_bytes'] = length
            offset = start + length
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--target', choices=['native', 'wasm'], required=True)
    ap.add_argument('--emsdk', type=Path)
    ap.add_argument('--opt', choices=['2', '3', 's'], default='2')
    ap.add_argument('--lto', action='store_true')
    ap.add_argument('--iterations', type=int, default=2000000)
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--node', default='node')
    ap.add_argument('--output', type=Path)
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        ap.error('Use at least three samples and a positive iteration count')
    label = f'{opts.target}-O{opts.opt}' + ('-lto' if opts.lto else '')
    output = (opts.output or ROOT / 'out/performance' / label).resolve()
    output.mkdir(parents=True, exist_ok=True)
    build = output / 'build'
    commands = []
    configure = ['cmake', '-S', ROOT / 'src/benchmarks/result-abi', '-B', build,
                 '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release',
                 f'-DBENCH_OPT={opts.opt}', f'-DBENCH_LTO={"ON" if opts.lto else "OFF"}']
    if opts.target == 'wasm':
        if not opts.emsdk:
            ap.error('--emsdk is required for wasm')
        emscripten = opts.emsdk.resolve() / 'upstream/emscripten'
        configure += ['-G', 'Ninja',
                      f'-DCMAKE_TOOLCHAIN_FILE={emscripten / "cmake/Modules/Platform/Emscripten.cmake"}',
                      f'-DCMAKE_CROSSCOMPILING_EMULATOR={opts.node}']
    elif os.name == 'nt':
        configure += ['-A', 'x64']
    commands.append([str(x) for x in configure])
    invoke(configure, output / 'configure.log')
    compile_cmd = ['cmake', '--build', build, '--config', 'Release', '--parallel', '4']
    commands.append([str(x) for x in compile_cmd])
    _, build_seconds = invoke(compile_cmd, output / 'build.log')
    invoke(['ctest', '--test-dir', build, '-C', 'Release', '--output-on-failure'], output / 'verify.log')
    abi = []
    if opts.target == 'wasm':
        probe = [sys.executable, emscripten / 'em++.py', '-std=c++17', '-DNDEBUG',
                 f'-O{opts.opt}', '-Wno-return-type-c-linkage', '-S', '-emit-llvm',
                 '-I', ROOT / 'src/runtime', ROOT / 'src/benchmarks/result-abi/abi-probes.cpp',
                 '-o', output / 'abi.ll']
        commands.append([str(x) for x in probe])
        invoke(probe, output / 'abi.log')
        abi = [line for line in (output / 'abi.ll').read_text().splitlines() if line.startswith('define ')]
    binaries = {}
    artifacts = {}
    for name in PROTOCOLS:
        if opts.target == 'wasm':
            binary = build / f'result-{name}.js'
            binaries[name] = [opts.node, str(binary)]
            artifacts[name] = {'wasm': sizes(binary.with_suffix('.wasm')), 'js': sizes(binary)}
        else:
            binary = build / ('Release' if os.name == 'nt' else '') / f'result-{name}{".exe" if os.name == "nt" else ""}'
            binaries[name] = [str(binary)]
            artifacts[name] = {'executable': sizes(binary)}
    samples = []
    rng = random.Random(20261110)
    for sample in range(opts.samples):
        order = PROTOCOLS[:]
        rng.shuffle(order)
        for name in order:
            text, elapsed = invoke(binaries[name] + [str(opts.iterations)], output / f'sample-{sample}-{name}.jsonl')
            rows = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
            expected_rows = 5 if name == 'direct' else 15
            if len(rows) != expected_rows:
                raise RuntimeError(f'Unexpected record count for {name}: {len(rows)}')
            for row in rows:
                row.update(sample=sample, process_wall_seconds=elapsed)
                samples.append(row)
        print(f'{label}: sample {sample + 1}/{opts.samples}', flush=True)
    keys = sorted({(r['protocol'], r['workload'], r['error_period']) for r in samples})
    summary = []
    for name, workload, period in keys:
        values = [r['ns_per_op'] for r in samples
                  if (r['protocol'], r['workload'], r['error_period']) == (name, workload, period)]
        checksums = {(r['checksum'], r['errors']) for r in samples
                     if (r['protocol'], r['workload'], r['error_period']) == (name, workload, period)}
        if len(checksums) != 1:
            raise RuntimeError('Non-deterministic result')
        summary.append(dict(protocol=name, workload=workload, error_period=period,
                            median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    # All protocols must compute the same observable results, not only run successfully.
    for workload in ['u32-chain', 'u64-chain', 'struct16-chain', 'pointer-chain', 'void-chain']:
        for period in [0, 1024, 1]:
            observed = {(r['checksum'], r['errors']) for r in samples
                        if r['workload'] == workload and r['error_period'] == period}
            if len(observed) != 1:
                raise RuntimeError(f'Cross-protocol mismatch: {workload}/{period}')
    manifest = dict(schema=1, target=opts.target, opt=opts.opt, lto=opts.lto,
                    platform=platform.platform(), machine=platform.machine(), cpu=platform.processor(), python=sys.version,
                    node=subprocess.check_output([opts.node, '--version'], text=True).strip(),
                    commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                    dirty=bool(subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'status', '--porcelain'], cwd=ROOT)),
                    commands=commands, build_seconds=build_seconds, samples=samples,
                    summary=summary, artifacts=artifacts, abi_definitions=abi)
    manifest['source_hashes'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                 for p in sorted((ROOT / 'src/benchmarks/result-abi').glob('*')) if p.is_file()}
    manifest['result_header_sha256'] = hashlib.sha256((ROOT / 'src/runtime/core/rt_result.h').read_bytes()).hexdigest()
    (output / 'results.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
    print(output / 'results.json')


if __name__ == '__main__':
    main()
