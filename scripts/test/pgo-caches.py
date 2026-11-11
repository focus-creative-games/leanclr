"""Exercise real receiver feedback, adaptive code generation, stale profiles and budgets."""
import argparse
import hashlib
import json
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--emsdk', type=Path, required=True)
    ap.add_argument('--skip-collect', action='store_true')
    opts = ap.parse_args()
    output = ROOT / 'out/pgo-cache-validation'
    output.mkdir(parents=True, exist_ok=True)
    commands = []
    def run(command, name, success=True):
        commands.append([str(x) for x in command])
        with (output / (name + '.log')).open('w', encoding='utf-8') as stream:
            result = subprocess.run([str(x) for x in command], cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT)
        assert (result.returncode == 0) == success, (name, result.returncode, (output / (name + '.log')).read_text(errors='replace')[-3000:])
    if not opts.skip_collect:
        print('PGO: collect cache hits and misses', flush=True)
        run([sys.executable, 'scripts/test/global-generics.py', '--profile', '--interface-call-cache', '--interface-cache-entries', '2',
             '--target', 'all', '--emsdk', opts.emsdk, '--output', 'out/pgo-cache-profile'], 'collect')
    profiles = [ROOT / f'out/pgo-cache-profile/result/default-{target}-Release-x64-profile.json' for target in ['native', 'wasm']]
    raw_profiles = [json.loads(path.read_text(encoding='utf-8')) for path in profiles]
    scenario_profiles = []
    for profile in raw_profiles:
        scenarios = {row['signature'].split('TC_PgoCaches::', 1)[1].split('(', 1)[0]: row for row in profile if 'TC_PgoCaches::' in row['signature']}
        assert scenarios['Mono']['interfaceSites'][0]['calls'] == 200, 'cache hits must be counted'
        assert scenarios['Mega']['interfaceSites'][0]['otherCalls'] == 40
        assert scenarios['Cold']['interfaceSites'][0]['cacheObservations'] == 100 and scenarios['Cold']['interfaceSites'][0]['twoEntryHits'] == 0
        assert scenarios['Phase']['interfaceSites'][0]['twoEntryHits'] == 196
        assert all(scenarios[name]['ilHash'] and scenarios[name]['ilBytes'] > 0 for name in ['Mono', 'Bi', 'Mega', 'Mixed', 'HotLater'])
        scenario_profiles.append(scenarios)
    run(['dotnet', 'build', 'src/tools/pgo2aot/Pgo2Aot.csproj', '-c', 'Release'], 'tool')
    converter = ['dotnet', ROOT / 'out/dotnet/Pgo2Aot/Release/net8.0/pgo2aot.dll', 'convert']
    rules = output / 'pgo.xml'
    run(converter + ['-i', profiles[0], '-i', profiles[1], '--strategy', 'threshold', '--sort-by', 'calls', '-o', rules], 'convert')
    root = ET.parse(rules).getroot()
    def nodes(name): return [m for a in root for m in a if f'TC_PgoCaches::{name}(' in m.attrib['signature']][0]
    assert [int(s.attrib['entries']) for s in nodes('Mono')] == [1]
    assert [int(s.attrib['entries']) for s in nodes('Bi')] == [2]
    assert list(nodes('Mega')) == []
    assert list(nodes('Cold')) == [] and [int(s.attrib['entries']) for s in nodes('Phase')] == [2]
    assert sorted(int(s.attrib['entries']) for s in nodes('Mixed')) == [1, 1, 2]
    assert sum(int(s.attrib['entries']) * 3 for s in nodes('Mixed')) == 12
    hot = list(nodes('HotLater'))
    assert len(hot) == 2 and int(hot[0].attrib['calls']) == 1000 and int(hot[0].attrib['entries']) == 2

    common = ['--interface-call-cache', '--profiled-interface-caches', '--lightweight-returns', '--packed-int32-returns', '--status-out-returns', '--share-reference-generics']
    print('PGO: regenerate adaptive caches', flush=True)
    run([sys.executable, 'scripts/test/global-generics.py', *common, '--pgo-rule-file', rules, '--target', 'all', '--emsdk', opts.emsdk,
         '--output', 'out/pgo-cache-guided'], 'guided')
    reports = {}
    for variant in ['default', 'disabled']:
        report = json.loads((ROOT / f'out/pgo-cache-guided/lightweight/{variant}/generic-aot-report.json').read_text())
        decisions = report['interface_cache_decisions']
        def entries(name): return [d['entries'] for d in decisions if f'TC_PgoCaches::{name}(' in d['method']]
        assert entries('Mono') == [1] and entries('Bi') == [2] and entries('Mega') == [0]
        assert entries('Cold') == [0] and entries('Phase') == [2]
        assert sorted(entries('Mixed')) == [0, 1, 1, 2]
        assert entries('HotLater') == [1, 0, 2]
        reports[variant] = report
    stale = output / 'stale.xml'
    for assembly in root:
        for method in assembly:
            if 'il-hash' in method.attrib: method.set('il-hash', '0000000000000000')
    ET.ElementTree(root).write(stale, encoding='utf-8', xml_declaration=True)
    print('PGO: reject stale fingerprints', flush=True)
    run([sys.executable, 'scripts/test/global-generics.py', *common, '--pgo-rule-file', stale, '--target', 'all', '--emsdk', opts.emsdk,
         '--output', 'out/pgo-cache-stale'], 'stale')
    for variant in ['default', 'disabled']:
        report = json.loads((ROOT / f'out/pgo-cache-stale/lightweight/{variant}/generic-aot-report.json').read_text())
        assert not any(d['entries'] for d in report['interface_cache_decisions'])
    run([sys.executable, 'scripts/test/global-generics.py', *common, '--pgo-rule-file', rules, '--config', 'Debug', '--output', 'out/pgo-cache-guided-debug'], 'debug')
    run([sys.executable, 'scripts/test/global-generics.py', *common, '--pgo-rule-file', rules, '--arch', 'Win32', '--output', 'out/pgo-cache-guided-win32'], 'win32')

    # Regression tests use CLI output, not copies of the selection algorithm.
    fixture = output / 'budget.json'
    fixture.write_text(json.dumps([dict(assembly='Budget', signature=f'System.Int32 Budget::{name}()', calls=calls, cost=calls, ilBytes=size)
                                   for name, calls, size in [('Large', 1000, 200), ('Small', 900, 10), ('Small2', 800, 10), ('Unknown', 700, 0)]]))
    budget = output / 'budget.xml'
    run(converter + ['-i', fixture, '--strategy', 'threshold', '--sort-by', 'calls', '--max-methods', '2', '--max-il-bytes', '20', '-o', budget], 'budget')
    tree = ET.parse(budget).getroot()
    assert tree.attrib['selected-method-count'] == '2' and tree.attrib['selected-il-bytes'] == '20'
    assert all('Small' in m.attrib['signature'] for a in tree for m in a)
    fixture.write_text(json.dumps([dict(assembly='Budget', signature=f'System.Int32 Budget::M{i}()', calls=2**64-1, cost=2**64-1) for i in range(2)]))
    run(converter + ['-i', fixture, '--strategy', 'pareto', '--pareto-ratio', '0.8', '-o', budget], 'pareto-overflow')
    assert ET.parse(budget).getroot().attrib['selected-method-count'] == '2'
    invalid = raw_profiles[0]
    bad = next(row for row in invalid if row['interfaceSites'])
    bad['interfaceSites'][0]['calls'] += 1
    fixture.write_text(json.dumps(invalid))
    run(converter + ['-i', fixture, '--strategy', 'threshold', '-o', budget], 'invalid-counters', success=False)
    result = dict(parent_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(), commands=commands,
                  receiver_scenarios=scenario_profiles, reports=reports, profile_hashes={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in profiles},
                  rules=rules.read_text(encoding='utf-8-sig'), notes=['12 formal configurations after 4 collection configurations; 71 managed cases plus PGO scenarios per run.',
                      'IL bytes budget applies to selected PGO roots, not explicit roots, transitive generic closure or machine-code bytes.',
                      'Receiver histograms are bounded, single-threaded AOT instrumentation; interpreter-only callsites are not observed.',
                      'Coverage heuristics are conservative; no optimality claim for phase locality or unobserved workloads.'])
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
    print('PGO receiver feedback checks passed', flush=True)


if __name__ == '__main__':
    main()
