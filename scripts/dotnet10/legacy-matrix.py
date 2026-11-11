"""Revalidate historical entry groups without hiding failed/unsupported cases."""
import argparse
import concurrent.futures
import json
from pathlib import Path
import re
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def execute(command, timeout):
    started = time.monotonic()
    try:
        process = subprocess.run(list(map(str, command)), cwd=ROOT, capture_output=True,
                                 text=True, errors='replace', timeout=timeout)
        return dict(exit_code=process.returncode, seconds=round(time.monotonic() - started, 3),
                    output=(process.stdout + process.stderr)[-6000:].replace(str(ROOT), '<repo>'))
    except subprocess.TimeoutExpired:
        return dict(exit_code='timeout', seconds=timeout, output='entry exceeded time limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runner', type=Path, required=True)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--configuration', default='Release')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--timeout', type=int, default=30)
    parser.add_argument('--workers', type=int, default=4)
    args = parser.parse_args()
    sources = ROOT / 'src/tests/managed-net10/ManagedNet10.LegacyTests'
    entries = sorted(set(re.findall(r'public static void (Run\w+)\(\)',
                                    '\n'.join(p.read_text(encoding='utf-8-sig') for p in sources.glob('*.cs')))))
    # Diagnostic selectors require environment state and are not independent groups.
    excluded = {'RunAll', 'RunAllPrefixThenRuntimeType', 'RunAllPrefixThenRuntimeTypeMethod',
                'RunGcFinalizerMethodThenRuntimeType'}
    entries = [entry for entry in entries if entry not in excluded]
    dll_dir = ROOT / f'out/dotnet/ManagedNet10.LegacyTests/{args.configuration}/net10.0'

    def run(entry):
        native = execute([args.runner.resolve(), '-l', dll_dir, '-l', args.runtime.resolve(),
                          '-e', 'ManagedNet10.LegacyTests.Program::' + entry,
                          'ManagedNet10.LegacyTests'], args.timeout)
        result = dict(entry=entry, leanclr=native)
        if native['exit_code'] != 0:
            result['reference'] = execute(['dotnet', dll_dir / 'ManagedNet10.LegacyTests.dll', entry], args.timeout)
        print(entry, native['exit_code'], flush=True)
        return result

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.workers)) as pool:
        results = list(pool.map(run, entries))
    report = dict(configuration=args.configuration, group_count=len(results),
                  passed=sum(r['leanclr']['exit_code'] == 0 for r in results), entries=results)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(f"Passed {report['passed']}/{report['group_count']}; failures include stock .NET differential output.")
    # This is an inventory, not a green test gate. Consumers must examine failures.


if __name__ == '__main__':
    main()
