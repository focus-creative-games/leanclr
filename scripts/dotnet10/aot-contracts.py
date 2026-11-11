"""Generate, compile and execute .NET 10 AOT smoke, real-heap GC and API contracts."""
import argparse
import hashlib
import platform
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def checked(command, log):
    print('Running', log.name, flush=True)
    with log.open('w', encoding='utf-8') as stream:
        process = subprocess.run(list(map(str, command)), cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT)
    if process.returncode:
        raise RuntimeError(f'{log}: {process.returncode}\n{log.read_text(encoding="utf-8", errors="replace")[-4000:]}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=['native', 'wasm'], default='native')
    parser.add_argument('--emsdk', type=Path)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--variant', choices=['default', 'optimized'], default='default')
    parser.add_argument('--configuration', choices=['Debug', 'Release', 'RelWithDebInfo'], default='Release')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/net10-aot-current')
    parser.add_argument('--sanitizers', default='', help='Native Clang/GCC sanitizer list')
    parser.add_argument('--asan', action='store_true', help='MSVC AddressSanitizer; put its runtime DLL directory on PATH')
    args = parser.parse_args()
    if args.target == 'wasm' and (not args.emsdk or args.asan or args.sanitizers):
        parser.error('Wasm requires --emsdk and does not use native sanitizer options')
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / 'out'):
        parser.error('--output must stay inside the repository out directory')
    if args.variant != 'default': output /= args.variant
    if args.target == 'wasm': output /= 'wasm'
    output.mkdir(parents=True, exist_ok=True)
    managed_configuration = 'Debug' if args.configuration == 'Debug' else 'Release'
    assemblies = ['ManagedNet10.Smoke', 'ManagedNet10.GcContracts', 'ManagedNet10.ApiContracts']
    checked(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], output / 'tool.log')
    directories = []
    for assembly in assemblies:
        checked(['dotnet', 'build', ROOT / f'src/tests/managed-net10/{assembly}/{assembly}.csproj', '-c', managed_configuration], output / (assembly + '-build.log'))
        directories.append(ROOT / f'out/dotnet/{assembly}/{managed_configuration}/net10.0')
    command = ['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '--leanaot-runtime-api-profile', 'coreclr-net10']
    for directory in [*directories, args.runtime.resolve()]: command += ['-d', directory]
    for assembly in assemblies: command += ['-a', assembly]
    generated = output / 'generated'
    command += ['-o', generated]
    if args.variant == 'optimized':
        command += ['--leanaot-lightweight-returns', '--leanaot-packed-int32-returns',
                    '--leanaot-status-out-returns', '--leanaot-share-reference-generics']
    checked(command, output / 'generate.log')
    manifest = json.loads((generated / 'generic-aot-report.json').read_text(encoding='utf-8'))
    assert any(item['Reason'] == 'runtime-native-binding' for item in manifest['fallbacks'])
    assert any(item['Reason'] == 'vararg-calling-convention' and 'VarargsTarget' in item['Target'] for item in manifest['method_fallbacks'])
    assert manifest['packed_int32_returns'] == (args.variant == 'optimized')
    native = output / ('asan' if args.asan else 'native')
    configure = ['cmake', '-S', ROOT / 'src/tests/aot-tester', '-B', native, f'-DCMAKE_BUILD_TYPE={args.configuration}',
                 '-DLEANCLR_BCL_ADAPTER=CORECLR_NET10', f'-DAOT_GENERATED_CPP_DIR={generated.as_posix()}']
    if args.target == 'wasm':
        configure += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE', '-DCMAKE_CROSSCOMPILING_EMULATOR=node',
            f'-DCMAKE_TOOLCHAIN_FILE={(args.emsdk.resolve() / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake").as_posix()}']
    elif os.name == 'nt': configure += ['-A', 'x64']
    if args.sanitizers: configure += [f'-DLEANCLR_SANITIZERS={args.sanitizers}']
    if args.asan:
        configure += ['-DCMAKE_CXX_FLAGS=/fsanitize=address', '-DCMAKE_C_FLAGS=/fsanitize=address',
                      '-DCMAKE_EXE_LINKER_FLAGS=/INCREMENTAL:NO']
    checked(configure, output / 'configure.log')
    checked(['cmake', '--build', native, '--config', args.configuration, '--parallel', '4'], output / 'build.log')
    runner = native / (f'bin/{args.configuration}/aot-tester.exe' if os.name == 'nt' else 'bin/aot-tester')
    prefix = []
    runtime = args.runtime.resolve()
    if args.target == 'wasm':
        import shutil
        stage = output / 'bcl'
        stage.mkdir(exist_ok=True)
        for library in runtime.glob('*.dll'):
            shutil.copy2(library, stage)
        runtime = stage.relative_to(ROOT).as_posix()
        directories = [directory.relative_to(ROOT).as_posix() for directory in directories]
        runner = native / 'bin/aot-tester.js'
        prefix = ['node']
    results = []
    for assembly, directory in zip(assemblies, directories):
        source = ROOT / f'src/tests/managed-net10/{assembly}/Program.cs'
        visibility = 'private' if assembly == 'ManagedNet10.Smoke' else 'public'
        entries = re.findall(rf'{visibility} static void (Test\w+)\(\)', source.read_text(encoding='utf-8-sig'))
        for entry in entries:
            command = [*prefix, runner, '-l', directory, '-l', runtime, '-e', f'{assembly}.Program::{entry}', assembly]
            checked(command, output / f'{assembly}-{entry}.log')
            results.append(dict(assembly=assembly, entry=entry, result='passed'))
    (output / 'results.json').write_text(json.dumps(dict(variant=args.variant, configuration=args.configuration,
        target=args.target, host=platform.platform(), corelib_sha256=hashlib.sha256((args.runtime / 'System.Private.CoreLib.dll').read_bytes()).hexdigest(),
        tests=results, generic_body_count=manifest['generic_body_count'], root_policy='call-safepoints-and-stable-byrefs'), indent=2) + '\n', encoding='utf-8')
    print(f'{len(results)} .NET 10 AOT/GC/API entries passed ({args.variant})', flush=True)


if __name__ == '__main__':
    main()
