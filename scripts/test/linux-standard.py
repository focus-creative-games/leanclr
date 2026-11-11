"""GNU fallback validation when a Linux environment has GCC but no CMake.

The source composition mirrors src/runtime/CMakeLists.txt (native host/Mono
adapter). This is a compiler/runtime check, not a Linux CMake CI replacement.
"""
import argparse
import concurrent.futures
import hashlib
import json
import os
import subprocess
from pathlib import Path


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--source', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--probe', type=Path, required=True)
    ap.add_argument('--standard', choices=['11', '17'], default='17')
    ap.add_argument('--sanitizers', action='store_true')
    opts = ap.parse_args()
    root, output = opts.source.resolve(), opts.output.resolve()
    output.relative_to(root.parent)  # Outputs must stay in the named workspace.
    output.mkdir(parents=True, exist_ok=True)
    runtime = root / 'src/runtime'
    cpp = set()
    for directory in ['alloc', 'codegen', 'core', 'interp', 'log', 'metadata', 'misc', 'profile', 'public_impl', 'utils', 'vm']:
        cpp.update((runtime / directory).glob('*.cpp'))
    cpp.update((runtime / 'gc').rglob('*.cpp'))
    cpp.update(runtime / p for p in ['host/services.cpp', 'host/encoding.cpp', 'host/default_services.cpp', 'host/default_allocation.cpp', 'host/extended_services.cpp'])
    cpp.update((runtime / 'platform').glob('*.cpp'))
    cpp.update((runtime / 'bcl/mono').rglob('*.cpp'))
    for directory in ['icalls', 'intrinsics', 'pinvokes', 'vmutils']:
        cpp.update((runtime / directory).glob('*.cpp'))
    c = set((runtime / '3rd/zlib').glob('*.c'))
    flags = ['-O2', '-DNDEBUG', '-DLEANCLR_STATIC=1', '-DLEANCLR_USE_SHIPPING_ZLIB=1', '-DLEANCLR_GC_MARK_SWEEP=1', '-DLEANCLR_GC_DEBUG=1', '-I', str(runtime)]
    sanitizer = ['-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'] if opts.sanitizers else []
    cxxflags = [f'-std=gnu++{opts.standard}', '-fno-exceptions'] + flags + sanitizer
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:allocator_may_return_null=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    commands = []
    def run(command, label):
        commands.append([str(x) for x in command])
        result = subprocess.run([str(x) for x in command], cwd=root, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (output / (label + '.log')).write_bytes(result.stdout)
        if result.returncode: raise RuntimeError(f'{label}: {result.returncode}\n{result.stdout.decode(errors="replace")[-3500:]}')
        return result.stdout.decode(errors='replace')
    def compile_source(source):
        key = hashlib.sha256(str(source).encode()).hexdigest()[:12]
        obj = output / (key + '.o')
        run(['g++' if source.suffix == '.cpp' else 'gcc', *(cxxflags if source.suffix == '.cpp' else flags + sanitizer), '-c', source, '-o', obj], key)
        return obj
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        objects = list(pool.map(compile_source, sorted(cpp | c)))
    library = output / 'libleanclr.a'
    run(['ar', 'rcs', library, *objects], 'archive')
    targets = {'public-cpp': ['public-cpp.cpp'], 'host-capabilities': ['host-capabilities.cpp'], 'default-host-services': ['default-host-services.cpp'],
               'host-services': ['host-services.cpp'], 'core-boundary': ['core-boundary.cpp', 'adapter-abi-probe.cpp'], 'il-bounds': ['il-bounds.cpp'],
               'foundation': ['foundation.cpp'], 'portable-memory': ['portable-memory.cpp'], 'integer-math': ['integer-math.cpp'], 'bit-scan': ['bit-scan.cpp'],
               'string-allocation': ['string-allocation.cpp'], 'command-line': ['command-line.cpp'], 'aligned-allocation': ['aligned-allocation.cpp'],
               'lifecycle': ['lifecycle.cpp'], 'container-allocation': ['container-allocation.cpp'], 'il-fuzz-smoke': ['il-fuzz-smoke.cpp', 'il-fuzz.cpp']}
    tests = []
    for name, files in targets.items():
        executable = output / name
        run(['g++', *cxxflags, *(root / 'src/tests/runtime-regressions' / f for f in files), library, '-ldl', '-o', executable], name + '-build')
        run([executable], name + '-test')
        tests.append(name)
    run([output / 'lifecycle', 'stop'], 'lifecycle-before-start-test'); tests.append('lifecycle-before-start')
    c_object = output / 'public-c.o'
    run(['gcc', *flags, *sanitizer, '-c', root / 'src/tests/runtime-regressions/public-c.c', '-o', c_object], 'public-c-compile')
    run(['g++', *sanitizer, c_object, library, '-ldl', '-o', output / 'public-c'], 'public-c-link')
    run([output / 'public-c'], 'public-c-test'); tests.append('public-c')
    probe = output / 'core-standard'
    run(['g++', *cxxflags, opts.probe.resolve(), library, '-ldl', '-o', probe], 'probe-build')
    probe_samples = [json.loads(run([probe, '1000000'], f'probe-{i}').strip()) for i in range(7)]
    (output / 'results.json').write_text(json.dumps(dict(standard=opts.standard, sanitizers=opts.sanitizers, tests=tests, probe=probe_samples,
        compiler=run(['g++', '--version'], 'compiler'), commands=commands,
        notes=['Manual native composition, GCC without CMake; Windows/Wasm CMake paths validated separately.',
               'Sanitizer leak checking disabled for process-lifetime runtime state; no leak-freedom claim. CI retains its separate leak checks.']), indent=2)+'\n')
    print(f'GNU C++{opts.standard}: {len(tests)} regression checks and probe passed', flush=True)


if __name__ == '__main__':
    main()
