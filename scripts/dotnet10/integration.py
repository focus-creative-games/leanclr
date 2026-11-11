"""Run the minimal .NET 10 interpreter/ABI/GC gate on the current native host."""
import argparse
import json
import os
from pathlib import Path
import platform
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def installed_runtime():
    candidates = []
    for line in subprocess.check_output(['dotnet', '--list-runtimes'], text=True).splitlines():
        match = re.fullmatch(r'Microsoft\.NETCore\.App (10\.\d+\.\d+) \[(.+)\]', line)
        if match:
            candidates.append((tuple(map(int, match[1].split('.'))), Path(match[2]) / match[1]))
    if not candidates: raise RuntimeError('A .NET 10 runtime is required; pass --runtime')
    return max(candidates)[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path)
    parser.add_argument('--configuration', choices=['Debug', 'Release', 'RelWithDebInfo'], default='Release')
    parser.add_argument('--output', type=Path, default=ROOT / 'out/net10-portable-gate')
    parser.add_argument('--sanitizers', default='')
    args = parser.parse_args()
    runtime = (args.runtime or installed_runtime()).resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ)
    if args.sanitizers:
        if os.name == 'nt': parser.error('Use the MSVC ASan gate on Windows')
        environment.update(ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:allocator_may_return_null=1',
                           UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    def run(command, label, timeout=None):
        print('Running', label, flush=True)
        path = output / (label + '.log')
        with path.open('w', encoding='utf-8') as stream:
            process = subprocess.run(list(map(str, command)), cwd=ROOT, stdout=stream,
                                     stderr=subprocess.STDOUT, timeout=timeout, env=environment)
        if process.returncode:
            raise RuntimeError(f'{path}: {process.returncode}\n{path.read_text(encoding="utf-8", errors="replace")[-4000:]}')
    native = output / 'native'
    contracts = output / 'contracts'
    for source, build, label in [('src/tools/leanrun', native, 'runner'), ('src/tests/net10-contracts', contracts, 'contracts')]:
        command = ['cmake', '-S', ROOT / source, '-B', build, '-DLEANCLR_BCL_ADAPTER=CORECLR_NET10',
                   f'-DCMAKE_BUILD_TYPE={args.configuration}', f'-DLEANCLR_NET10_RUNTIME_DIR={runtime.as_posix()}']
        if os.name == 'nt': command += ['-A', 'x64']
        if args.sanitizers: command += [f'-DLEANCLR_SANITIZERS={args.sanitizers}']
        run(command, label + '-configure')
        run(['cmake', '--build', build, '--config', args.configuration, '--parallel', '4'], label + '-build')
    run(['ctest', '--test-dir', contracts, '-C', args.configuration, '--output-on-failure'], 'native-contracts', 120)
    suffix = '.exe' if os.name == 'nt' else ''
    runner = native / ('bin/' + (args.configuration + '/' if os.name == 'nt' else '') + 'leanrun' + suffix)
    managed_configuration = 'Debug' if args.configuration == 'Debug' else 'Release'
    results = []
    for assembly in ['ManagedNet10.Smoke', 'ManagedNet10.GcContracts', 'ManagedNet10.ApiContracts']:
        project = ROOT / f'src/tests/managed-net10/{assembly}/{assembly}.csproj'
        run(['dotnet', 'build', project, '-c', managed_configuration], assembly + '-build')
        directory = ROOT / f'out/dotnet/{assembly}/{managed_configuration}/net10.0'
        visibility = 'private' if assembly == 'ManagedNet10.Smoke' else 'public'
        entries = re.findall(rf'{visibility} static void (Test\w+)\(\)', project.with_name('Program.cs').read_text(encoding='utf-8-sig'))
        for entry in entries:
            run([runner, '-l', directory, '-l', runtime, '-e', f'{assembly}.Program::{entry}', assembly], assembly + '-' + entry, 30)
            results.append(dict(assembly=assembly, entry=entry, passed=True))
        if assembly == 'ManagedNet10.GcContracts':
            run(['dotnet', directory / (assembly + '.dll')], 'stock-runtime-gc', 120)
    run(['python', ROOT / 'scripts/dotnet10/check-layout-rejection.py', '--runner', runner,
         '--runtime-dir', runtime, '--assembly-dir', ROOT / f'out/dotnet/ManagedNet10.Smoke/{managed_configuration}/net10.0',
         '--output', output / 'incompatible-corelib'], 'layout-rejection', 60)
    (output / 'results.json').write_text(json.dumps(dict(host=platform.platform(), architecture=platform.machine(),
        sanitizers=args.sanitizers, tests=results), indent=2) + '\n', encoding='utf-8')
    print(f'{len(results)} managed entries, native contracts and CoreLib rejection passed')


if __name__ == '__main__':
    main()
