"""Build the real managed/AotTests project and verify its global generic AOT cases."""
import argparse
import json
import os
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEPS = ['mscorlib', 'System', 'System.Core', 'System.Xml', 'System.Configuration',
        'System.Security', 'Mono.Security', 'System.Numerics']


def run(command, log):
    print('Running:', log.name, flush=True)
    with log.open('w', encoding='utf-8') as stream:
        p = subprocess.run([str(x) for x in command], cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT)
    if p.returncode:
        raise RuntimeError(f'{log}: {p.returncode}\n{log.read_text(errors="replace")[-3500:]}')


def msbuild():
    direct = shutil.which('msbuild')
    if direct:
        return direct
    finder = Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    if finder.exists():
        result = subprocess.check_output([str(finder), '-latest', '-products', '*', '-requires', 'Microsoft.Component.MSBuild',
                                          '-find', 'MSBuild/**/Bin/MSBuild.exe'], text=True).strip().splitlines()
        if result:
            return result[0]
    raise RuntimeError('MSBuild not found; pass --msbuild with a Visual Studio MSBuild.exe path')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--target', choices=['native', 'wasm', 'all'], default='native')
    ap.add_argument('--config', choices=['Release', 'Debug'], default='Release')
    ap.add_argument('--arch', choices=['x64', 'Win32'], default='x64')
    ap.add_argument('--emsdk', type=Path)
    ap.add_argument('--msbuild', type=Path)
    ap.add_argument('--lightweight-returns', action='store_true')
    opts = ap.parse_args()
    if opts.target != 'native' and not opts.emsdk:
        ap.error('--emsdk is required for Wasm')
    root = ROOT / 'out/global-generics'
    output = root / ('lightweight' if opts.lightweight_returns else 'result')
    stage = root / 'managed'
    output.mkdir(parents=True, exist_ok=True)
    stage.mkdir(parents=True, exist_ok=True)
    run(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], output / 'tool.log')
    run([opts.msbuild or msbuild(), ROOT / 'src/tests/managed/AotTests/AotTests.csproj', '/t:Build', '/p:Configuration=Release',
         '/p:LangVersion=latest', f'/p:LEANCLR_OUT_ROOT={root / "managed-build"}', '/v:minimal', '/nologo'], output / 'managed.log')
    for assembly in ['AotTests', 'Common', 'GenericAotArguments']:
        shutil.copy2(root / f'managed-build/dotnet/{assembly}/Release/{assembly}.dll', stage)
    bcl = ROOT / 'src/libraries/mono-4.5'
    for name in DEPS:
        shutil.copy2(bcl / (name + '.dll'), stage)
    for variant in ['default', 'disabled']:
        generated = output / variant
        command = ['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '-d', stage, '-d', bcl,
                   '-a', 'AotTests', '-o', generated, '--leanaot-aot-rule-file', ROOT / 'src/tests/generic-registry/aot.xml']
        if variant == 'disabled':
            command += ['--leanaot-max-generic-instances', '0']
        if opts.lightweight_returns:
            command += ['--leanaot-lightweight-returns']
        run(command, output / f'{variant}-generate.log')
        report = json.loads((generated / 'generic-aot-report.json').read_text())
        assert bool(report['instances']) == (variant == 'default')
        assert not list(generated.glob('Common.module_registration.*')), 'generic definition must not require module registration'
        if variant == 'default':
            assert any('Common,' in x['key'] for x in report['instances'])
            assert list(generated.glob('generic_methods_part*.cpp'))
            registration = (generated / 'generic_registration.cpp').read_text()
            assert 'g_aot_generic_methods' in registration
            assert registration.count('[] = { { static_cast<leanclr::metadata::RtElementType>(8), nullptr, 0x00000000, nullptr, 0 } };') == 1, 'singleton int argument sequences were not interned'
        for target in (['native', 'wasm'] if opts.target == 'all' else [opts.target]):
            label = f'{variant}-{target}-{opts.config}-{opts.arch}'
            build = output / label
            command = ['cmake', '-S', ROOT / 'src/tests/generic-registry', '-B', build,
                       f'-DCMAKE_BUILD_TYPE={opts.config}', '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release',
                       f'-DGENERIC_REGISTRY_CPP={generated}', f'-DGENERIC_REGISTRY_DLLS:STRING={stage.relative_to(ROOT).as_posix()}',
                       f'-DGENERIC_REGISTRY_DISABLED={"ON" if variant == "disabled" else "OFF"}']
            if target == 'native':
                command += ['-A', opts.arch]
            else:
                command += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE', '-DCMAKE_CROSSCOMPILING_EMULATOR=node',
                            f'-DCMAKE_TOOLCHAIN_FILE={opts.emsdk.resolve() / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"}']
            run(command, output / f'{label}-configure.log')
            run(['cmake', '--build', build, '--config', opts.config, '--parallel', '4'], output / f'{label}-build.log')
            run(['ctest', '--test-dir', build, '-C', opts.config, '--output-on-failure'], output / f'{label}-tests.log')
    print('Formal AotTests global generic checks passed', flush=True)


if __name__ == '__main__':
    main()
