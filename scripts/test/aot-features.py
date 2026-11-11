"""Build and verify closed generic AOT + normal-path EH handoff on Windows/Node."""
import argparse
import json
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def run(command, log):
    print('Running:', str(command[0]), log.name, flush=True)
    with log.open('w', encoding='utf-8') as stream:
        p = subprocess.run([str(x) for x in command], cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT)
    if p.returncode:
        raise RuntimeError(f'{log}: exit {p.returncode}\n{log.read_text(errors="replace")[-3000:]}')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--emsdk', type=Path)
    ap.add_argument('--target', choices=['native', 'wasm', 'all'], default='native')
    ap.add_argument('--config', choices=['Debug', 'Release'], default='Release')
    ap.add_argument('--arch', choices=['x64', 'Win32'], default='x64')
    ap.add_argument('--lightweight-returns', action='store_true')
    ap.add_argument('--packed-int32-returns', action='store_true')
    ap.add_argument('--status-out-returns', action='store_true')
    ap.add_argument('--share-reference-generics', action='store_true')
    ap.add_argument('--generic-only', action='store_true')
    opts = ap.parse_args()
    if opts.target != 'native' and not opts.emsdk:
        ap.error('--emsdk is required for wasm')
    output = ROOT / ('out/aot-features-lightweight' if opts.lightweight_returns else 'out/aot-features')
    output.mkdir(parents=True, exist_ok=True)
    csc = Path(os.environ.get('WINDIR', 'C:/Windows')) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
    ilasm = csc.with_name('ilasm.exe')
    bcl = ROOT / 'src/libraries/mono-4.5'
    run(['dotnet', 'build', 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], output / 'tool.log')
    import shutil
    for family, assembly in [('generic', 'GenericFixtures'), ('eh', 'EhFixtures')]:
        if opts.generic_only and family != 'generic':
            continue
        stage = output / family / 'managed'
        stage.mkdir(parents=True, exist_ok=True)
        if family == 'generic':
            run([csc, '/nologo', '/noconfig', '/optimize+', '/nostdlib+', '/target:library',
                 '/out:' + str(stage / 'GenericLibrary.dll'), '/r:' + str(bcl / 'mscorlib.dll'),
                 ROOT / 'src/tests/generic-aot/Library.cs'], output / 'generic-library-csc.log')
        run([csc, '/nologo', '/noconfig', '/optimize+', '/nostdlib+', '/target:library',
             '/out:' + str(stage / (assembly + '.dll')), '/r:' + str(bcl / 'mscorlib.dll'),
             *(['/r:' + str(stage / 'GenericLibrary.dll')] if family == 'generic' else []),
             ROOT / f'src/tests/{family}-aot/Fixtures.cs'], output / f'{family}-csc.log')
        shutil.copy2(bcl / 'mscorlib.dll', stage)
        if family == 'generic':
            run(['dotnet', 'run', '--project', ROOT / 'src/tests/generic-aot/SignatureFixture.csproj', '-c', 'Release', '--',
                 stage / 'GenericFixtures.dll'], output / 'generic-signatures.log')
        if family == 'eh':
            run([ilasm, '/DLL', '/OUTPUT:' + str(stage / 'FilterFixture.dll'), ROOT / 'src/tests/eh-aot/FilterFixture.il'], output / 'filter-ilasm.log')
        for variant in ['default', 'disabled'] if family == 'generic' else ['default', 'interpreted-loop']:
            generated = output / family / variant
            command = ['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll',
                       '-d', stage, '-d', bcl, '-a', assembly, '-o', generated,
                       '--leanaot-aot-rule-file', ROOT / f'src/tests/{family}-aot/{"interpreted.xml" if variant == "interpreted-loop" else "aot.xml"}']
            if family == 'eh': command += ['-a', 'FilterFixture']
            if variant == 'disabled': command += ['--leanaot-max-generic-instances', '0']
            if opts.lightweight_returns: command += ['--leanaot-lightweight-returns']
            if opts.share_reference_generics: command += ['--leanaot-share-reference-generics']
            if opts.status_out_returns: command += ['--leanaot-status-out-returns']
            if opts.packed_int32_returns: command += ['--leanaot-packed-int32-returns']
            run(command, output / f'{family}-{variant}-generate.log')
            report = json.loads((generated / 'generic-aot-report.json').read_text())
            assert report['return_protocol'] == ('lightweight-void-reference' if opts.lightweight_returns else 'result')
            assert report['packed_int32_returns'] == opts.packed_int32_returns
            if family == 'eh':
                assert any(x['Reason'] == 'eh-filter-or-fault' for x in report['method_fallbacks'])
                assert any(x['Reason'] == 'eh-handler-inside-finally' for x in report['method_fallbacks'])
            else:
                assert bool(report['instances']) == (variant == 'default')
            targets = ['native', 'wasm'] if opts.target == 'all' else [opts.target]
            for target in targets:
                build = output / family / f'{variant}-{target}-{opts.config}-{opts.arch}'
                prefix = 'GENERIC' if family == 'generic' else 'EH'
                command = ['cmake', '-S', ROOT / f'src/tests/{family}-aot', '-B', build,
                           f'-DCMAKE_BUILD_TYPE={opts.config}', '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release',
                           f'-D{prefix}_AOT_CPP={generated}', f'-D{prefix}_AOT_DLLS:STRING={stage.relative_to(ROOT).as_posix()}']
                if family == 'generic': command += [f'-DGENERIC_AOT_DISABLED={"ON" if variant == "disabled" else "OFF"}']
                if target == 'native': command += ['-A', opts.arch]
                else:
                    command += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE', '-DCMAKE_CROSSCOMPILING_EMULATOR=node',
                                f'-DCMAKE_TOOLCHAIN_FILE={opts.emsdk.resolve() / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"}']
                label = f'{family}-{variant}-{target}-{opts.config}-{opts.arch}'
                run(command, output / (label + '-configure.log'))
                run(['cmake', '--build', build, '--config', opts.config, '--parallel', '4'], output / (label + '-build.log'))
                run(['ctest', '--test-dir', build, '-C', opts.config, '--output-on-failure'], output / (label + '-tests.log'))
    run(['dotnet', 'run', '--project', 'src/tests/generic-aot/PlannerTests.csproj', '-c', 'Release', '--',
         output / 'generic/managed', bcl], output / 'planner.log')
    print('AOT feature checks passed', flush=True)


if __name__ == '__main__':
    main()
