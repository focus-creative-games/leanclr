"""Build real managed/AotTests and verify generic and interface AOT paths."""
import argparse
import json
import os
import re
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
    ap.add_argument('--packed-int32-returns', action='store_true')
    ap.add_argument('--status-out-returns', action='store_true')
    ap.add_argument('--interface-call-cache', action='store_true')
    ap.add_argument('--interface-cache-entries', type=int, choices=[1, 2], default=1)
    ap.add_argument('--output', type=Path, default=ROOT / 'out/global-generics')
    opts = ap.parse_args()
    if opts.target != 'native' and not opts.emsdk:
        ap.error('--emsdk is required for Wasm')
    root = opts.output.resolve()
    output = root / ('lightweight' if opts.lightweight_returns else 'result')
    stage = root / 'managed'
    output.mkdir(parents=True, exist_ok=True)
    stage.mkdir(parents=True, exist_ok=True)
    run(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], output / 'tool.log')
    for invalid in ['0', '3', '-1']:
        result = subprocess.run(['dotnet', str(ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll'),
                                 '--leanaot-interface-cache-entries', invalid], cwd=ROOT, text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (output / f'cache-entries-invalid-{invalid}.log').write_text(result.stdout, encoding='utf-8')
        assert result.returncode != 0 and '--leanaot-interface-cache-entries must be 1 or 2.' in result.stdout, invalid
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
                   '-a', 'AotTests', '-o', generated, '--leanaot-aot-rule-file', ROOT / 'src/tests/generic-registry/aot.xml',
                   '--leanaot-interface-cache-entries', str(opts.interface_cache_entries)]
        if variant == 'disabled':
            command += ['--leanaot-max-generic-instances', '0']
        if opts.lightweight_returns:
            command += ['--leanaot-lightweight-returns']
        if opts.status_out_returns: command += ['--leanaot-status-out-returns']
        if opts.packed_int32_returns:
            command += ['--leanaot-packed-int32-returns']
        if opts.interface_call_cache:
            command += ['--leanaot-interface-call-cache']
        run(command, output / f'{variant}-generate.log')
        report = json.loads((generated / 'generic-aot-report.json').read_text())
        assert report['packed_int32_returns'] == opts.packed_int32_returns
        assert report['interface_call_cache'] == opts.interface_call_cache
        assert report['interface_call_cache_entries'] == (opts.interface_cache_entries if opts.interface_call_cache else 0)
        budget = 2 if opts.interface_cache_entries == 2 else 4
        methods = {}
        integer_signatures = {}
        for cpp in generated.glob('*method*part*.cpp'):
            for section in cpp.read_text().split('// Method: ')[1:]:
                name = section.splitlines()[0]
                single = section.count('leanclr::codegen::InterfaceCallCache ')
                double = section.count('leanclr::codegen::InterfaceCallCache2 ')
                count = single + double
                assert count <= budget and single * 3 + double * 6 <= 12, name
                assert not (single if opts.interface_cache_entries == 2 else double), name
                methods[name] = count
                if 'TC_Int32Returns::' in name:
                    integer_signatures[name.split('TC_Int32Returns::', 1)[1].split('(', 1)[0]] = section.splitlines()[1]
        for method, value in [('SignedLeaf', 'int32_t'), ('UnsignedLeaf', 'uint32_t')]:
            expected = f'leanclr::codegen::AotInt32Result<{value}>' if opts.packed_int32_returns else f'leanclr::RtResult<{value}>'
            assert expected in integer_signatures[method], method
        for method, value in [('EnumLeaf', 'int32_t'), ('UnsignedEnumLeaf', 'uint32_t'), ('ByRef', 'void*'), ('Wide', 'int64_t'), ('Floating', 'float64_t')]:
            expected = 'leanclr::RtErr' if opts.status_out_returns and method in ['Wide', 'Floating'] else f'leanclr::RtResult<{value}>'
            assert expected in integer_signatures[method], method
        for name, expected in [('StraightLine', 0), ('Sum', 1), ('Budget', budget)]:
            matches = [count for method, count in methods.items() if re.search(r'TC_InterfaceDispatch::' + name + r'\(', method)]
            assert matches == [expected if opts.interface_call_cache else 0], (name, matches)
        if not opts.interface_call_cache:
            assert not any(methods.values()), 'cache flag must be opt-in'
        assert bool(report['instances']) == (variant == 'default')
        assert bool(report['generic_normal_path_methods']) == (variant == 'default')
        if variant == 'default':
            for reason in ['eh-snapshot-type', 'eh-value-this', 'eh-filter-or-fault', 'eh-handler-inside-finally', 'eh-address-or-native-control']:
                assert any(row['Reason'] == reason for row in report['fallbacks']), reason
            generic_cpp = '\n'.join(cpp.read_text() for cpp in generated.glob('generic_methods_part*.cpp'))
            assert 'resume_exception(__aot_method,' in generic_cpp
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
