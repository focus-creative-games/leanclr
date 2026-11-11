"""Reproduce same-source LeanAOT/IL2CPP baselines (Windows host, optional Node/Wasm)."""
import argparse
import hashlib
import json
import os
import platform
import random
import shutil
import statistics
import subprocess
import time
from pathlib import Path
from run_protocols import ROOT, invoke, sizes

DEPS = ['mscorlib', 'System', 'System.Core', 'System.Xml', 'System.Configuration',
        'System.Security', 'Mono.Security', 'System.Numerics']
WORKLOADS = ['arithmetic', 'calls', 'properties', 'array', 'interfaces', 'structs', 'generics', 'exceptions']


def native_path(path):
    return str(path).replace('/', '\\')


def build_il2cpp(unity, output, stage, commands):
    """Use installed tools. Never copy/edit the Unity installation or generated methods."""
    tools = unity / 'Editor/Data/il2cpp/build/deploy'
    linked = output / 'il2cpp-unitylinked'
    linker = [tools / 'UnityLinker.exe', '--platform=WindowsDesktop', '--architecture=x64',
              '--dotnetprofile=unityaot-win32', '--dotnetruntime=il2cpp', '--rule-set=Conservative',
              f'--include-assembly={stage / "AotBaseline.dll"}',
              f'--search-directory={unity / "Editor/Data/MonoBleedingEdge/lib/mono/unityaot-win32"}',
              f'--out={linked}', '--include-profile-descriptors', '--include-profile-search-dirs',
              '--core-action=link', '--i18n=none']
    commands.append([str(x) for x in linker])
    invoke(linker, output / 'linker-unity.log')
    baselib = unity / 'Editor/Data/PlaybackEngines/windowsstandalonesupport/Variations/win64_player_nondevelopment_il2cpp'
    cli = [tools / 'il2cpp.exe', '--convert-to-cpp', '--compile-cpp', '--libil2cpp-static',
           '--dont-deploy-baselib', '--platform=WindowsDesktop', '--architecture=x64',
           '--configuration=Release', '--dotnetprofile=unityaot-win32', f'--directory={linked}',
           f'--generatedcppdir={output / "il2cpp-generated"}',
           f'--outputpath={output / "il2cpp/AotBaseline.exe"}',
           f'--cachedirectory={output / "il2cpp-cache"}',
           f'--additional-libraries={baselib / "baselib.dll.lib"}',
           '--emit-null-checks', '--enable-array-bounds-check', '--enable-divide-by-zero-check',
           '--enable-stacktrace', '--jobs=4']
    commands.append([str(x) for x in cli])
    helper = False
    try:
        invoke(cli, output / 'il2cpp-build.log')
    except RuntimeError:
        log = (output / 'il2cpp-build.log').read_text()
        graph = output / 'il2cpp-cache/buildstate/bee.dag.json'
        if 'Libil2cppLackey' not in log or 'C1083' not in log or not graph.exists():
            raise
        # The CLI is distributed without one standalone host helper. Rebuild
        # only that helper and replay the two link steps. No managed method,
        # runtime implementation, optimizer option or compiled object is replaced.
        helper = True
        build = graph.parent
        nodes = json.loads(graph.read_text())['Nodes']
        helper_node = next(n for n in nodes if n.get('Annotation', '').startswith('C_Win64')
                           and 'Libil2cppLackey' in n.get('Action', ''))
        missing_source = next(p for p in helper_node['Inputs'] if p.endswith('DllMain.cpp'))
        cmd = helper_node['Action'].replace(native_path(missing_source),
                                           native_path(ROOT / 'src/benchmarks/aot-baseline/il2cpp-thread-hook.cpp'))
        replay = [cmd]
        final_outputs = []
        for node in nodes:
            if not node.get('Annotation', '').startswith('Link_Win64'):
                continue
            cmd = node['Action']
            absent_baselib = native_path(unity / 'Editor/Data/il2cpp/build/baselib/release_win64/baselib.dll.lib')
            cmd = cmd.replace('"' + absent_baselib + '"', '')
            replay.append(cmd)
            final_outputs.extend(node['Outputs'])
        (output / 'il2cpp-standalone-commands.json').write_text(json.dumps(replay, indent=2))
        for index, cmd in enumerate(replay):
            p = subprocess.run(cmd, cwd=build, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               text=True, encoding='utf-8', errors='replace')
            (output / f'il2cpp-replay-{index}.log').write_text(p.stdout)
            if p.returncode:
                raise RuntimeError(f'IL2CPP standalone link failed: {p.stdout[-2000:]}')
        for suffix in ['AotBaseline.exe', 'Libil2cppLackey.dll']:
            artifact = next(build / p for p in final_outputs if p.endswith('/' + suffix))
            shutil.copy2(artifact, output / 'il2cpp' / suffix)
    shutil.copy2(baselib / 'baselib.dll', output / 'il2cpp/baselib.dll')
    return helper


def build(opts, output, managed_input=None):
    commands = []
    stage = output / 'managed'
    stage.mkdir(parents=True, exist_ok=True)
    compiler = Path(os.environ.get('WINDIR', 'C:/Windows')) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
    csc = [compiler, '/nologo', '/noconfig', '/optimize+', '/nostdlib+', '/target:exe',
           '/out:' + native_path(stage / 'AotBaseline.dll'),
           '/r:' + native_path(ROOT / 'src/libraries/mono-4.5/mscorlib.dll'),
           '/r:' + native_path(ROOT / 'src/libraries/mono-4.5/System.dll'),
           native_path(ROOT / 'src/benchmarks/aot-baseline/Benchmarks.cs')]
    if managed_input:
        shutil.copy2(managed_input, stage / 'AotBaseline.dll')
    else:
        commands.append([str(x) for x in csc])
        invoke(csc, output / 'csc.log')
    for name in DEPS:
        shutil.copy2(ROOT / 'src/libraries/mono-4.5' / f'{name}.dll', stage)
    invoke(['dotnet', 'build', ROOT / 'src/leanaot/LeanAOT/LeanAOT.csproj', '-c', 'Release'], output / 'tool-build.log')
    generator = ['dotnet', ROOT / 'out/dotnet/LeanAOT/Release/net8.0/LeanAOT.dll', '-d', stage,
                 '-d', ROOT / 'src/libraries/mono-4.5', '-a', 'AotBaseline', '-o', output / 'generated',
                 '--leanaot-aot-rule-file', getattr(opts, 'aot_rule_file', ROOT / 'src/benchmarks/aot-baseline/aot.xml'),
                 '--leanaot-max-generic-instances', str(opts.max_generic_instances)]
    if opts.lightweight_returns:
        generator += ['--leanaot-lightweight-returns']
    if getattr(opts, 'packed_int32_returns', False):
        generator += ['--leanaot-packed-int32-returns']
    if getattr(opts, 'status_out_returns', False):
        generator += ['--leanaot-status-out-returns']
    if opts.interface_call_cache:
        generator += ['--leanaot-interface-call-cache', '--leanaot-interface-cache-entries', str(opts.interface_cache_entries)]
    commands.append([str(x) for x in generator])
    invoke(generator, output / 'generate.log')
    for target in ['native'] + (['wasm'] if opts.emsdk else []):
        cmd = ['cmake', '-S', ROOT / 'src/benchmarks/aot-baseline', '-B', output / target,
               '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_TRY_COMPILE_CONFIGURATION=Release',
               f'-DLEANCLR_BENCH_GENERATED_DIR={output / "generated"}']
        if target == 'native':
            cmd += ['-A', 'x64']
        else:
            cmd += ['-G', 'Ninja', '-DLEANCLR_HOST=PORTABLE',
                    f'-DCMAKE_TOOLCHAIN_FILE={opts.emsdk.resolve() / "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"}']
        commands.append([str(x) for x in cmd])
        invoke(cmd, output / f'{target}-configure.log')
        compile_cmd = ['cmake', '--build', output / target, '--config', 'Release', '--parallel', '4']
        commands.append([str(x) for x in compile_cmd])
        invoke(compile_cmd, output / f'{target}-build.log')
    helper = build_il2cpp(opts.unity, output, stage, commands) if opts.unity else None
    (output / 'build-manifest.json').write_text(json.dumps(dict(commands=commands, standalone_thread_hook=helper), indent=2))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--unity', type=Path, help='Unity installation, e.g. D:/UnityHubs/2022.3.62f3')
    ap.add_argument('--emsdk', type=Path)
    ap.add_argument('--skip-build', action='store_true')
    ap.add_argument('--build-only', action='store_true')
    ap.add_argument('--max-generic-instances', type=int, default=2048)
    ap.add_argument('--lightweight-returns', action='store_true')
    ap.add_argument('--packed-int32-returns', action='store_true')
    ap.add_argument('--status-out-returns', action='store_true')
    ap.add_argument('--interface-call-cache', action='store_true')
    ap.add_argument('--interface-cache-entries', type=int, choices=[1, 2], default=1)
    ap.add_argument('--samples', type=int, default=7)
    ap.add_argument('--iterations', type=int, default=5000000)
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/aot')
    opts = ap.parse_args()
    if opts.samples < 3 or opts.iterations < 1:
        ap.error('Use at least 3 samples and positive iterations')
    output = opts.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if not opts.skip_build:
        build(opts, output)
    if opts.build_only:
        return
    stage_arg = os.path.relpath(output / 'managed', ROOT).replace('\\', '/')
    if stage_arg.startswith('..'):
        ap.error('Wasm Node host mount requires --output inside the repository')
    engines = {'native-leanaot': [str(output / 'native/Release/aot-baseline.exe'), str(output / 'managed')],
               'native-cpp': [str(output / 'native/Release/cpp-reference.exe')]}
    if opts.unity:
        engines['native-il2cpp'] = [str(output / 'il2cpp/AotBaseline.exe')]
    if opts.emsdk:
        engines['wasm-leanaot'] = ['node', str(output / 'wasm/aot-baseline.js'), stage_arg]
        engines['wasm-cpp'] = ['node', str(output / 'wasm/cpp-reference.js')]
    rows = []
    rng = random.Random(20261110)
    for sample in range(opts.samples):
        order = list(engines)
        rng.shuffle(order)
        for engine in order:
            text, elapsed = invoke(engines[engine] + [str(opts.iterations)], output / f'sample-{sample}-{engine}.jsonl')
            records = [json.loads(line) for line in text.splitlines() if line.startswith('{')]
            if len(records) != (7 if engine.endswith('-cpp') else 8):
                raise RuntimeError(f'Incomplete benchmark output: {engine}')
            for row in records:
                row.update(engine=engine, sample=sample, process_wall_seconds=elapsed)
                if 'execution' not in row:
                    row['execution'] = 'il2cpp'
                rows.append(row)
        print(f'AOT baseline sample {sample + 1}/{opts.samples}', flush=True)
    for workload in WORKLOADS:
        checksums = {row['checksum'] for row in rows if row['workload'] == workload}
        if len(checksums) != 1:
            raise RuntimeError(f'Cross-engine checksum mismatch: {workload}: {checksums}')
    summary = []
    for engine in engines:
        for workload in WORKLOADS:
            group = [r for r in rows if r['engine'] == engine and r['workload'] == workload]
            if not group:
                continue
            values = [r['ns_per_op'] for r in group]
            summary.append(dict(engine=engine, workload=workload, execution=group[0]['execution'],
                                median_ns=statistics.median(values), min_ns=min(values), max_ns=max(values)))
    artifacts = {}
    for engine, command in engines.items():
        binary = Path(command[1] if engine.startswith('wasm') else command[0])
        if engine.startswith('wasm'):
            binary = binary.with_suffix('.wasm')
        artifacts[engine] = sizes(binary)
    source = ROOT / 'src/benchmarks/aot-baseline/Benchmarks.cs'
    result = dict(schema=1, platform=platform.platform(), unity=str(opts.unity), samples=rows,
                  summary=summary, artifacts=artifacts,
                  cpu=platform.processor(),
                  commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                  notes=['IL2CPP uses Unity BCL after UnityLinker; LeanAOT uses repository Mono BCL.',
                         'Execution labels inspect actual entry/helper backends; AOT EH entries may hand off to interpreter on exceptions.',
                         'LeanAOT now emits native noinline for managed NoInlining; historical P0 used different inlining behavior.',
                         'C++ is a computational reference, not a managed semantic equivalent.',
                         'Process wall time includes workload, warmup, verification and output; it is NOT startup time.',
                         'Native executables and Wasm bytes exclude external BCL/metadata/resources; do not compare as complete deployment sizes.',
                         'IL2CPP/WebGL end-to-end timing is not measured by this runner.'])
    result['source_hashes'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                               for p in sorted((ROOT / 'src/benchmarks/aot-baseline').glob('*')) if p.is_file()}
    (output / 'results.json').write_text(json.dumps(result, indent=2))
    print(output / 'results.json')


if __name__ == '__main__':
    main()
