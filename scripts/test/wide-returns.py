"""Validate wide AOT return protocols, legacy callers and all eight target flags."""
import argparse
import json
import subprocess
import sys
import hashlib
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--emsdk', type=Path, required=True)
    opts = ap.parse_args()
    output = ROOT / 'out/wide-return-validation'
    output.mkdir(parents=True, exist_ok=True)
    rows = []
    configurations = [
        ('combined', ['--status-out-returns', '--lightweight-returns', '--packed-int32-returns', '--target', 'all', '--emsdk', str(opts.emsdk), '--output', 'out/status-out-validation']),
        ('legacy', ['--target', 'all', '--emsdk', str(opts.emsdk)]),
        ('wide-win32', ['--status-out-returns', '--arch', 'Win32']),
        ('combined-debug', ['--status-out-returns', '--lightweight-returns', '--packed-int32-returns', '--config', 'Debug', '--output', 'out/status-out-validation-debug']),
    ]
    for name, arguments in configurations:
        print('Wide validation:', name, flush=True)
        with (output / (name + '.log')).open('w') as stream:
            subprocess.run([sys.executable, 'scripts/test/global-generics.py'] + arguments, cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT, check=True)
        source = ROOT / (arguments[arguments.index('--output') + 1] if '--output' in arguments else 'out/global-generics')
        source /= 'lightweight' if '--lightweight-returns' in arguments else 'result'
        reports = {}
        for variant in ['default', 'disabled']:
            target = output / f'{name}-{variant}-report.json'
            shutil.copy2(source / variant / 'generic-aot-report.json', target)
            reports[variant] = dict(sha256=hashlib.sha256(target.read_bytes()).hexdigest(), report=json.loads(target.read_text()))
        rows.append(dict(name=name, arguments=arguments, reports=reports))
    for target in ['native', 'wasm']:
        for cfg in ['Release', 'Debug'] if target == 'native' else ['Release']:
            directory = output / (target + '-' + cfg)
            command = ['cmake', '-S', 'src/tests/aot-returns', '-B', str(directory), '-DCMAKE_BUILD_TYPE=' + cfg]
            if target == 'native':
                command += ['-A', 'x64']
            else:
                command += ['-G', 'Ninja', '-DCMAKE_CROSSCOMPILING_EMULATOR=node', '-DCMAKE_TOOLCHAIN_FILE=' + str(opts.emsdk.resolve() / 'upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake')]
            with (output / (target + '-' + cfg + '.log')).open('w') as stream:
                for step in [command, ['cmake', '--build', str(directory), '--config', cfg], ['ctest', '--test-dir', str(directory), '-C', cfg, '--output-on-failure']]:
                    subprocess.run(step, cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT, check=True)
    ir = output / 'wide-abi.ll'
    subprocess.run([str(opts.emsdk.resolve() / 'upstream/emscripten/em++.bat'), '-std=c++17', '-O2', '-I', 'src/runtime', '-S', '-emit-llvm', 'src/tests/aot-returns/main.cpp', '-o', str(ir)], cwd=ROOT, check=True)
    import re
    signature = re.search(r'^define .*@aot_wide_probe\(.*$', ir.read_text(), re.M).group()
    assert re.search(r'\bi8 @aot_wide_probe', signature) and 'sret' not in signature, signature
    (output / 'validation.json').write_text(json.dumps(dict(configurations=rows, wide_llvm_signature=signature, checks='65 managed cases plus eight mixed target flags per formal run; all nonzero error bytes and output preservation in standalone test'), indent=2) + '\n')


if __name__ == '__main__':
    main()
