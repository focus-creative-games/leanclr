"""Validate all four return-protocol flags and archive each generated-mode result."""
import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emsdk', type=Path, required=True)
    args = parser.parse_args()
    archive = ROOT / 'out/int32-returns-validation'
    archive.mkdir(parents=True, exist_ok=True)
    rows = []
    for flags in [0, 1, 2, 3]:
        protocol = (['--lightweight-returns'] if flags & 1 else []) + (['--packed-int32-returns'] if flags & 2 else [])
        for configuration in (['all', 'Debug', 'Win32'] if flags & 2 else ['all']):
            config = ['--target', 'all', '--emsdk', str(args.emsdk)] if configuration == 'all' else ['--config', 'Debug'] if configuration == 'Debug' else ['--arch', 'Win32']
            destination = archive / f'flags-{flags}' / configuration
            destination.mkdir(parents=True, exist_ok=True)
            for suite in ['aot-features', 'global-generics']:
                command = [sys.executable, str(ROOT / f'scripts/test/{suite}.py')] + protocol + config
                if suite == 'global-generics' and flags & 2:
                    command += ['--interface-call-cache', '--interface-cache-entries', '2']
                print(f'Validating flags={flags} {configuration} {suite}', flush=True)
                process = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')
                (destination / f'{suite}.log').write_text(process.stdout, encoding='utf-8')
                if process.returncode:
                    raise RuntimeError(process.stdout[-4500:])
                cfg = 'Debug' if configuration == 'Debug' else 'Release'
                arch = 'Win32' if configuration == 'Win32' else 'x64'
                targets = ['native', 'wasm'] if configuration == 'all' else ['native']
                root = ROOT / ('out/aot-features-lightweight' if flags & 1 else 'out/aot-features') if suite == 'aot-features' else ROOT / 'out/global-generics' / ('lightweight' if flags & 1 else 'result')
                families = ['generic', 'eh'] if suite == 'aot-features' else ['formal']
                for family in families:
                    source = root / family if suite == 'aot-features' else root
                    for variant in (['default', 'interpreted-loop'] if family == 'eh' else ['default', 'disabled']):
                        generated = source / variant
                        report = json.loads((generated / 'generic-aot-report.json').read_text())
                        assert report['return_protocol_flags'] == flags
                        shutil.copy2(generated / 'generic-aot-report.json', destination / f'{family}-{variant}-report.json')
                        for target in targets:
                            build = source / f'{variant}-{target}-{cfg}-{arch}'
                            log = build / 'Testing/Temporary/LastTest.log'
                            text = log.read_text(errors='replace')
                            assert 'Test Failed.' not in text and 'Test Passed.' in text
                            if family == 'formal':
                                assert text.count('Passed: AotTests.TC_Int32Returns.') == 7
                                assert text.count('Passed: AotTests.TC_InterfaceDispatch.') == 14
                                assert text.count('Passed: AotTests.TC_GenericAot.') == 24
                            name = f'{family}-{variant}-{target}.log'
                            shutil.copy2(log, destination / name)
                            rows.append(dict(flags=flags, configuration=configuration, family=family, variant=variant, target=target,
                                             log=(destination / name).relative_to(ROOT).as_posix()))
    (archive / 'matrix.json').write_text(json.dumps(rows, indent=2) + '\n', encoding='utf-8')
    print(f'Integer return validation passed: {len(rows)} configurations', flush=True)


if __name__ == '__main__':
    main()
