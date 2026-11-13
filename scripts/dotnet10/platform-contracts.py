"""Verify minimal globalization, environment and reflection on the target CoreLib."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--runtime', type=Path, required=True)
    ap.add_argument('--target', choices=['native', 'wasm'], default='native')
    ap.add_argument('--emsdk', type=Path)
    ap.add_argument('--output', type=Path, default=ROOT / 'out/platform-contracts')
    opts = ap.parse_args()
    if opts.target == 'wasm' and not opts.emsdk: ap.error('--emsdk is required for Wasm')
    output = opts.output.resolve() / opts.target
    if not output.is_relative_to(ROOT / 'out'): ap.error('--output must stay in repository out')
    output.mkdir(parents=True, exist_ok=True)
    def run(command, label, env=None):
        print('Running', label, flush=True)
        path = output / (label + '.log')
        with path.open('w', encoding='utf-8') as stream:
            p = subprocess.run(list(map(str, command)), cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT, env=env)
        if p.returncode:
            raise RuntimeError(f'{label}: {p.returncode}\n{path.read_text(encoding="utf-8", errors="replace")[-4000:]}')
    projects = [('ManagedNet10.GlobalizationContracts', ROOT / 'src/tests/managed-net10/ManagedNet10.GlobalizationContracts/ManagedNet10.GlobalizationContracts.csproj'),
                ('ReflectionMetadataTests', ROOT / 'src/tests/reflection-metadata/ReflectionMetadataTests.csproj')]
    directories = []
    run(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], 'tool')
    for assembly, project in projects:
        run(['dotnet', 'build', project, '-c', 'Release'], assembly + '-build')
        directories.append(ROOT / f'out/dotnet/{assembly}/Release/net10.0')
    runtime = opts.runtime.resolve()
    generated = output / 'generated'
    command = ['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '--leanaot-runtime-api-profile', 'coreclr-net10']
    for directory in [*directories, runtime]: command += ['-d', directory]
    for assembly, _ in projects: command += ['-a', assembly]
    run([*command, '-o', generated], 'generate')
    build = output / 'build'
    command = ['cmake', '-S', ROOT / 'src/tests/platform-contracts', '-B', build,
               '-DCMAKE_BUILD_TYPE=Release', f'-DPLATFORM_AOT_CPP={generated.as_posix()}']
    if opts.target == 'wasm':
        command += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE',
                    f'-DCMAKE_TOOLCHAIN_FILE={(opts.emsdk.resolve() / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake").as_posix()}']
    elif os.name == 'nt': command += ['-A', 'x64']
    run(command, 'configure')
    run(['cmake', '--build', build, '--config', 'Release', '--parallel', '4'], 'build')
    invariant = opts.target == 'wasm' or os.name != 'nt'
    cases = [('ManagedNet10.GlobalizationContracts', 'ManagedNet10.GlobalizationContracts.Program', 'Strings'),
             ('ManagedNet10.GlobalizationContracts', 'ManagedNet10.GlobalizationContracts.Program', 'EnvironmentRoundtrip'),
             ('ReflectionMetadataTests', 'ReflectionMetadataTests.Program', 'AttributeMetadata'),
             ('ReflectionMetadataTests', 'ReflectionMetadataTests.Program', 'PropertyAttributeInstances'),
             ('ReflectionMetadataTests', 'ReflectionMetadataTests.Program', 'ThrowingAttributeMetadata')]
    if invariant: cases.append(('ManagedNet10.GlobalizationContracts', 'ManagedNet10.GlobalizationContracts.Program', 'InvariantProfile'))
    if opts.target == 'wasm':
        import shutil
        stage = output / 'bcl'; stage.mkdir(exist_ok=True)
        for path in runtime.glob('*.dll'): shutil.copy2(path, stage)
        runtime = stage.relative_to(ROOT).as_posix()
    results = []
    for mode in ['interp', 'aot']:
        exe = build / (f'platform-{mode}.js' if opts.target == 'wasm' else f'Release/platform-{mode}.exe' if os.name == 'nt' else f'platform-{mode}')
        prefix = ['node', exe] if opts.target == 'wasm' else [exe]
        for assembly, type_name, entry in cases:
            directory = directories[0 if assembly == projects[0][0] else 1]
            if opts.target == 'wasm': directory = directory.relative_to(ROOT).as_posix()
            run([*prefix, '-l', directory, '-l', runtime, '-e', f'{type_name}::{entry}', assembly], mode + '-' + entry)
            results.append(dict(mode=mode, entry=entry, passed=True))
    env_exe = build / ('platform-environment.js' if opts.target == 'wasm' else 'Release/platform-environment.exe' if os.name == 'nt' else 'platform-environment')
    run(['node', env_exe] if opts.target == 'wasm' else [env_exe], 'environment-buffer')
    public_exe = build / ('bin/public-invoker.js' if opts.target == 'wasm' else 'bin/Release/public-invoker.exe' if os.name == 'nt' else 'bin/public-invoker')
    run(['node', public_exe, runtime] if opts.target == 'wasm' else [public_exe, runtime], 'public-invoker-abi')
    environment = dict(os.environ)
    if invariant: environment['DOTNET_SYSTEM_GLOBALIZATION_INVARIANT'] = '1'
    run(['dotnet', directories[0] / 'ManagedNet10.GlobalizationContracts.dll', *(['invariant'] if invariant else [])], 'stock-globalization', environment)
    run(['dotnet', directories[1] / 'ReflectionMetadataTests.dll', 'PropertyAttributeInstances'], 'stock-reflection')
    (output / 'results.json').write_text(json.dumps(dict(target=opts.target, invariant=invariant,
        corelib_sha256=hashlib.sha256((opts.runtime / 'System.Private.CoreLib.dll').read_bytes()).hexdigest(), tests=results), indent=2), encoding='utf-8')
    print('Platform contracts passed', flush=True)

if __name__ == '__main__': main()
