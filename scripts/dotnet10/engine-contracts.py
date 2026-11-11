"""Build the independent C ABI and Godot/Unreal lifecycle contract suite."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
from integration import ROOT, installed_runtime


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runtime', type=Path)
    p.add_argument('--target', choices=['native', 'wasm'], default='native')
    p.add_argument('--emsdk', type=Path)
    p.add_argument('--configuration', default='Release')
    p.add_argument('--sanitizers', default='')
    p.add_argument('--asan', action='store_true')
    p.add_argument('--shared', action='store_true')
    a = p.parse_args()
    if a.target == 'wasm' and (not a.emsdk or a.sanitizers or a.asan or a.shared): p.error('Wasm requires --emsdk, without native sanitizers')
    output = ROOT / ('out/net10-engine-' + a.target + '-' + a.configuration + ('-asan' if a.asan else '') + ('-shared' if a.shared else ''))
    output.mkdir(parents=True, exist_ok=True)
    runtime = (a.runtime or installed_runtime()).resolve()
    managed = ROOT / 'out/dotnet/ManagedNet10.EngineContracts/Release/net10.0'
    def run(command, name):
        print('Running', name, flush=True)
        with (output / (name + '.log')).open('w', encoding='utf-8') as log:
            subprocess.run(list(map(str, command)), cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
    run(['dotnet', 'build', ROOT / 'src/tests/managed-net10/ManagedNet10.EngineContracts', '-c', 'Release'], 'managed')
    command = ['cmake', '-S', ROOT / 'src/tests/engine-host', '-B', output, f'-DCMAKE_BUILD_TYPE={a.configuration}']
    if a.target == 'wasm':
        stage = output / 'bcl'
        stage.mkdir(exist_ok=True)
        for library in runtime.glob('*.dll'): shutil.copy2(library, stage)
        runtime = stage.relative_to(ROOT)
        managed = managed.relative_to(ROOT)
        command += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE', '-DCMAKE_CROSSCOMPILING_EMULATOR=node',
                    f'-DCMAKE_TOOLCHAIN_FILE={(a.emsdk.resolve() / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake").as_posix()}']
    elif os.name == 'nt': command += ['-A', 'x64']
    command += [f'-DLEANCLR_ENGINE_RUNTIME_DIR={runtime.as_posix()}', f'-DLEANCLR_ENGINE_MANAGED_DIR={managed.as_posix()}']
    if a.shared: command += ['-DBUILD_SHARED_LEANCLR=ON']
    if a.sanitizers: command += [f'-DLEANCLR_SANITIZERS={a.sanitizers}']
    if a.asan: command += ['-DCMAKE_CXX_FLAGS=/fsanitize=address', '-DCMAKE_C_FLAGS=/fsanitize=address', '-DCMAKE_EXE_LINKER_FLAGS=/INCREMENTAL:NO']
    run(command, 'configure')
    run(['cmake', '--build', output, '--config', a.configuration, '--parallel', '4'], 'build')
    run(['ctest', '--test-dir', output, '-C', a.configuration, '--output-on-failure'], 'contracts')
    print('Independent engine host contracts passed', flush=True)


if __name__ == '__main__': main()
