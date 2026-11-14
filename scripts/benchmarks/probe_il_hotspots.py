"""Diagnostic generated-code ablations, reusing the measured baseline runtime library.

Does not change runtime/generator sources or overwrite baseline generated sources.
"""
import argparse
import ctypes
import hashlib
import json
import random
import shutil
import statistics
from pathlib import Path
from run_protocols import ROOT, invoke


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline', type=Path, default=ROOT / 'out/performance/il-families')
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/il-hotspots')
    ap.add_argument('--skip-build', action='store_true')
    ap.add_argument('--sweep-only', action='store_true', help='Measure NewArray scaling using existing probe binaries')
    opts = ap.parse_args()
    baseline, out = opts.baseline.resolve(), opts.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    lib = baseline / 'native/runtime/Release/leanclr.lib'
    if not opts.skip_build and not opts.sweep_only:
        generated = out / 'generated'
        shutil.copytree(baseline / 'generated', generated, dirs_exist_ok=True)
        body = generated / 'AotBaseline.method_body_part1.cpp'
        text = body.read_text()
        # These call sites are specific to the recorded input assembly.
        start = text.index('// Method: System.UInt32 Benchmarks::Delegate(')
        end = text.index('// Method: System.UInt32 Benchmarks::Struct(', start)
        section = text[start:end]
        assert section.count('leanclr_generated_direct_call_bridge_97da025c8ea411d126916fab24ea5c5a(') == 1
        section = section.replace('leanclr_generated_direct_call_bridge_97da025c8ea411d126916fab24ea5c5a(', 'probe_delegate(')
        text = text[:start] + section + text[end:]
        # Restrict cast replacements to the Cast kernel (not exception handlers).
        start = text.index('// Method: System.UInt32 Benchmarks::Cast(')
        end = text.index('// Method: System.UInt32 Benchmarks::Boxing(', start)
        section = text[start:end].replace('leanclr::codegen::cast_class(', 'probe_cast(').replace('leanclr::codegen::is_inst(', 'probe_cast(')
        text = text[:start] + section + text[end:]
        # Replace only NewArray's site; MakeData and .cctor retain baseline behavior.
        start = text.index('// Method: System.UInt32 Benchmarks::NewArray(')
        end = text.index('// Method: System.UInt32 Benchmarks::EhNormal(', start)
        assert text[start:end].count('LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ELE_KLASS(') == 1
        section = text[start:end].replace('LEANCLR_CODEGEN_NEW_SZARRAY_FROM_ELE_KLASS(', 'probe_array(')
        text = text[:start] + section + text[end:]
        body.write_text('#include "probes.h"\n' + text)
        host = (ROOT / 'src/benchmarks/il-families/main.cpp').read_text()
        host = '#include "gc/garbage_collector.h"\n' + host
        if 'before_gc =' not in host:
            host = host.replace('auto start = std::chrono::steady_clock::now();',
                                'int before_gc = gc::GarbageCollector::get_collection_count();\n'
                                'int64_t before_bytes = gc::GarbageCollector::get_used_size();\n'
                                'auto start = std::chrono::steady_clock::now();')
        host = host.replace('    return 0;\n}',
                            '    std::printf("{\\"gc_count_delta\\":%d,\\"used_bytes_delta\\":%lld}\\n", '
                            'gc::GarbageCollector::get_collection_count() - before_gc, '
                            'static_cast<long long>(gc::GarbageCollector::get_used_size() - before_bytes));\n    return 0;\n}')
        (out / 'main.cpp').write_text(host)
        invoke([ROOT / 'tools/clang-format/clang-format.exe', '--style=file:' + str(ROOT / '.clang-format'), '-i', body, out / 'main.cpp'], out / 'format.log')
        cmake = '''cmake_minimum_required(VERSION 3.15)
project(il_hotspot_probes CXX)
set(CMAKE_CXX_STANDARD 17)
'''
        for name, directory, level in [('base', baseline / 'generated', 0), ('array-class', generated, 1), ('array-layout', generated, 2)]:
            cmake += f'''file(GLOB sources_{level} "{directory.as_posix()}/*.cpp")
add_executable({name} main.cpp ${{sources_{level}}})
target_include_directories({name} PRIVATE "{directory.as_posix()}" "{ROOT.as_posix()}/src/runtime" "{ROOT.as_posix()}/src/benchmarks/il-families")
target_compile_definitions({name} PRIVATE BENCH_AOT=1 LEANCLR_STATIC=1 LEANCLR_GC_MARK_SWEEP=1 PROBE_ARRAY_LEVEL={level})
target_link_libraries({name} PRIVATE "{lib.as_posix()}" bcrypt)
'''
        (out / 'CMakeLists.txt').write_text(cmake)
        invoke(['cmake', '-S', out, '-B', out / 'build', '-A', 'x64'], out / 'configure.log')
        invoke(['cmake', '--build', out / 'build', '--config', 'Release', '--parallel', '4'], out / 'build.log')
    k32 = ctypes.windll.kernel32
    k32.GetCurrentProcess.restype = ctypes.c_void_p
    k32.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    if not k32.SetProcessAffinityMask(k32.GetCurrentProcess(), 4):
        raise ctypes.WinError()
    if opts.sweep_only:
        rows = []
        jobs = [(sample, engine, n) for sample in range(3)
                for engine in ['base', 'array-layout', 'il2cpp']
                for n in [65536, 262144, 1048576, 4194304]]
        random.Random(20261114).shuffle(jobs)
        for sample, engine, n in jobs:
            command = ([baseline / 'il2cpp/AotBaseline.exe', 23, n] if engine == 'il2cpp' else
                       [out / ('build/Release/' + engine + '.exe'), baseline / 'managed', 'NewArray', n, 19])
            output, _ = invoke(command, out / ('sweep-%d-%s-%d.jsonl' % (sample, engine, n)))
            records = [json.loads(line) for line in output.splitlines() if line.startswith('{')]
            rows.append(dict(records[0], **(records[1] if len(records) > 1 else {}), engine=engine, sample=sample))
        for n in [65536, 262144, 1048576, 4194304]:
            assert len({r['checksum'] for r in rows if r['iterations'] == n}) == 1
        (out / 'scaling.json').write_text(json.dumps(rows, indent=2))
        for n in [65536, 262144, 1048576, 4194304]:
            print(n, {e: statistics.median(r['ns_per_op'] for r in rows if r['engine'] == e and r['iterations'] == n)
                      for e in ['base', 'array-layout', 'il2cpp']})
        return
    ns = {'Delegate': 20000000, 'Cast': 20000000, 'NewArray': 2000000}
    rows = []
    rng = random.Random(20261114)
    for sample in range(7):
        jobs = [(engine, name) for engine in ['base', 'array-class', 'array-layout'] for name in ns]
        rng.shuffle(jobs)
        for engine, name in jobs:
            output, _ = invoke([out / ('build/Release/' + engine + '.exe'), baseline / 'managed', name, ns[name], 19],
                               out / ('sample-%d-%s-%s.jsonl' % (sample, engine, name)))
            records = [json.loads(line) for line in output.splitlines() if line.startswith('{')]
            assert len(records) == 2
            rows.append(dict(records[0], **records[1], sample=sample, engine=engine))
        print('Probe round %d/7 complete' % (sample+1), flush=True)
    oracles = {}
    for name, kind in [('Delegate', 17), ('Cast', 20), ('NewArray', 23)]:
        output, _ = invoke(['dotnet', baseline / 'managed/AotBaseline.dll', kind, ns[name]], out / ('oracle-' + name + '.jsonl'))
        reference = next(json.loads(line) for line in output.splitlines() if line.startswith('{'))
        oracles[name] = reference['checksum']
        assert all(r['checksum'] == oracles[name] for r in rows if r['workload'] == name)
    summary = []
    for name in ns:
        for engine in ['base', 'array-class', 'array-layout']:
            group = [r for r in rows if r['workload'] == name and r['engine'] == engine]
            vals = [r['ns_per_op'] for r in group]
            summary.append(dict(workload=name, engine=engine, median_ns=statistics.median(vals), min_ns=min(vals), max_ns=max(vals),
                                gc_count_delta=sorted({r['gc_count_delta'] for r in group}),
                                used_bytes_delta=sorted({r['used_bytes_delta'] for r in group})))
    result = dict(samples=rows, summary=summary, affinity_mask=4, reference_checksums=oracles,
                  baseline_runtime_lib=str(lib), runtime_lib_sha256=hashlib.sha256(lib.read_bytes()).hexdigest(),
                  baseline_managed_sha256=hashlib.sha256((baseline / 'managed/AotBaseline.dll').read_bytes()).hexdigest())
    (out / 'results.json').write_text(json.dumps(result, indent=2))
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
