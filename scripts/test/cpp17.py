"""Validate the C++17 minimum, host compositions and higher-standard consumers."""
import argparse
import json
import subprocess
import sys
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TARGETS = 'public-invoker public-c public-cpp host-capabilities default-host-services host-services core-boundary foundation portable-memory integer-math bit-scan il-bounds string-allocation command-line aligned-allocation lifecycle container-allocation il-fuzz-smoke'.split()
TESTS = '^(public-invoker|public-c|public-cpp|host-capabilities|default-host-services|host-services|core-boundary|layer-boundaries|foundation|portable-memory|integer-math|bit-scan|il-bounds|string-allocation|command-line|aligned-allocation|lifecycle-failure|lifecycle-before-start|container-allocation|il-fuzz-smoke)$'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--emsdk', type=Path, required=True)
    opts = ap.parse_args()
    output = ROOT / 'out/cpp17-matrix'
    output.mkdir(parents=True, exist_ok=True)
    commands, results = [], []
    def run(command, name, success=True):
        commands.append([str(x) for x in command])
        with (output / (name + '.log')).open('w', encoding='utf-8') as stream:
            result = subprocess.run([str(x) for x in command], cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT)
        assert (result.returncode == 0) == success, (name, (output / (name + '.log')).read_text(errors='replace')[-4000:])
        return (output / (name + '.log')).read_text(errors='replace')
    for name, config, arch, options in [
        ('native', 'Release', 'x64', []), ('debug', 'Debug', 'x64', []),
        ('win32', 'Release', 'Win32', []), ('portable', 'Release', 'x64', ['-DLEANCLR_HOST=PORTABLE']),
        ('custom', 'Release', 'x64', ['-DLEANCLR_HOST=CUSTOM', '-DLEANCLR_REGRESSION_PUBLIC_API_ONLY=ON']),
        ('shared', 'Release', 'x64', ['-DBUILD_SHARED_LEANCLR=ON', '-DLEANCLR_REGRESSION_PUBLIC_API_ONLY=ON']),
        ('wasm', 'Release', None, ['-DLEANCLR_HOST=PORTABLE']),
    ]:
        print('C++17 composition:', name, flush=True)
        directory = output / name
        command = ['cmake', '-S', ROOT / 'src/tests/runtime-regressions', '-B', directory, '-DCMAKE_BUILD_TYPE=' + config, *options]
        if arch: command += ['-A', arch, '-DLEANCLR_TEST_CPP20_CONSUMER=ON']
        else: command += ['-G', 'Ninja', '-DCMAKE_CROSSCOMPILING_EMULATOR=node', '-DCMAKE_TOOLCHAIN_FILE=' + str(opts.emsdk.resolve() / 'upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake')]
        run(command, name + '-configure')
        run(['cmake', '--build', directory, '--config', config, '--parallel', '4', *(['--target', *TARGETS] if name == 'wasm' else [])], name + '-build')
        result = run(['ctest', '--test-dir', directory, '-C', config, '--output-on-failure', *(['-R', TESTS] if name == 'wasm' else [])], name + '-tests')
        results.append(dict(name=name, ctest=result))
    # CMAKE_CXX_STANDARD=20 must propagate to every layer, not get reset to 17.
    directory = output / 'runtime20'
    run(['cmake', '-S', ROOT / 'src/benchmarks/core-standard', '-B', directory, '-A', 'x64', '-DCMAKE_CXX_STANDARD=20'], 'runtime20-configure')
    for target in ['leanclr-core-objects', 'leanclr-host-objects', 'leanclr-bcl-mono-objects']:
        assert '<LanguageStandard>stdcpp20</LanguageStandard>' in (directory / 'runtime' / (target + '.vcxproj')).read_text(encoding='utf-8-sig')
    run(['cmake', '--build', directory, '--config', 'Release', '--parallel', '4'], 'runtime20-build')
    probe = json.loads(run([directory / 'Release/core-standard.exe', '10000'], 'runtime20-probe'))
    assert probe['standard'] >= 202002, probe
    # Independent old-standard callers must get a clear diagnostic.
    negative = output / 'negative'
    negative.mkdir(exist_ok=True)
    (negative / 'main.cpp').write_text('#include "build_config.h"\nint main() { return 0; }\n')
    (negative / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.15)\nproject(old_cpp CXX)\nadd_executable(old_cpp main.cpp)\nset_target_properties(old_cpp PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)\ntarget_include_directories(old_cpp PRIVATE "' + (ROOT / 'src/runtime').as_posix() + '")\n')
    run(['cmake', '-S', negative, '-B', negative / 'build', '-A', 'x64'], 'old-native-configure')
    assert 'LeanCLR requires C++17' in run(['cmake', '--build', negative / 'build', '--config', 'Release'], 'old-native-build', False)
    assert 'LeanCLR requires C++17' in run([opts.emsdk.resolve() / 'upstream/emscripten/em++.bat', '-std=c++14', '-I', ROOT / 'src/runtime', '-c', negative / 'main.cpp', '-o', negative / 'old.o'], 'old-wasm-build', False)
    for sample in ['tools/leanrun', 'samples/startup']:
        name = Path(sample).name
        print('C++17 consumer build:', name, flush=True)
        directory = output / name
        run(['cmake', '-S', ROOT / 'src' / sample, '-B', directory, '-A', 'x64'], name + '-configure')
        run(['cmake', '--build', directory, '--config', 'Release', '--parallel', '4'], name + '-build')
    for name, config in [('native', 'Release'), ('native', 'Debug'), ('wasm', 'Release')]:
        label = 'returns-' + name + '-' + config
        directory = output / label
        command = ['cmake', '-S', ROOT / 'src/tests/aot-returns', '-B', directory, '-DCMAKE_BUILD_TYPE=' + config]
        if name == 'native': command += ['-A', 'x64']
        else: command += ['-G', 'Ninja', '-DCMAKE_CROSSCOMPILING_EMULATOR=node', '-DCMAKE_TOOLCHAIN_FILE=' + str(opts.emsdk.resolve() / 'upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake')]
        run(command, label + '-configure')
        run(['cmake', '--build', directory, '--config', config, '--parallel', '4'], label + '-build')
        run(['ctest', '--test-dir', directory, '-C', config, '--output-on-failure'], label + '-tests')
    run([sys.executable, ROOT / 'scripts/benchmarks/compare_int32_returns.py', '--abi-only', '--emsdk', opts.emsdk], 'scalar-abi')
    fixture = output / 'managed'
    fixture.mkdir(exist_ok=True)
    (fixture / 'ToolFixture.cs').write_text('public static class Entry { public static void Main() {} public static void Fail() { throw new System.InvalidOperationException("cpp17-smoke"); } }\n')
    csc = Path(os.environ.get('WINDIR', 'C:/Windows')) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
    run([csc, '/nologo', '/noconfig', '/nostdlib+', '/target:library', '/out:' + str(fixture / 'ToolFixture.dll'), '/r:' + str(ROOT / 'src/libraries/mono-4.5/mscorlib.dll'), fixture / 'ToolFixture.cs'], 'tool-fixture')
    command = [output / 'leanrun/bin/Release/leanrun.exe', '-l', fixture, '-l', ROOT / 'src/libraries/mono-4.5', 'ToolFixture', '-e']
    assert 'ok!' in run([*command, 'Entry::Main'], 'leanrun-normal')
    failure = run([*command, 'Entry::Fail'], 'leanrun-exception', False)
    assert 'InvalidOperationException' in failure and 'cpp17-smoke' in failure, failure
    # PInvoke registration and managed exception handoff are covered by the
    # public-invoker test in both the native and Wasm compositions above.
    name = 'startup-wasm'
    print('C++17 Wasm consumer build:', name, flush=True)
    directory = output / name
    run(['cmake', '-S', ROOT / 'src/samples/startup', '-B', directory, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release', '-DLEANCLR_HOST=PORTABLE',
         '-DCMAKE_TOOLCHAIN_FILE=' + str(opts.emsdk.resolve() / 'upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake')], name + '-configure')
    run(['cmake', '--build', directory, '--parallel', '4'], name + '-build')
    (output / 'results.json').write_text(json.dumps(dict(results=results, cpp20_probe=probe, commands=commands), indent=2) + '\n')
    print('C++17 composition, C++20 and minimum diagnostics passed', flush=True)


if __name__ == '__main__':
    main()
